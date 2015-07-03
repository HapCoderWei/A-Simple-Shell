#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAX_STR 512
#define MAX_PARA 50
int main()
{
    pid_t p1,p2;
    char buf[MAX_STR],bufile[MAX_STR], *ps_argv[MAX_PARA],*argv1[MAX_PARA],*argv2[MAX_PARA];
    int i,n,fd[2],save_fd, wt;

    while(1)                    /* Main Circle */
    {
	    write(STDOUT_FILENO,"myshell> ",9); /* Display the shell prompt "myshell>" */
	    memset(buf,0,MAX_STR);              /* Empty buffer */
	    n = read(STDIN_FILENO, buf, MAX_STR-1); /* read input from stdin to buffer */
	    buf[strlen(buf)]='\0';                  /* Add null terminate */
	    if (n < 0) {                            /* Read error handle */
            perror("read error");
            _exit(1);
	    }
	    else if (n == 1) continue; /* n = 1 is only input '\n'*/

	    /******************************************************/
	    if ( !(strncmp(buf, "quit", 4))) { /* quit to exit */
            write(STDOUT_FILENO,"bye-bye!\n",9);
            break;
	    }

	    /******************************************************/
	    p1 = fork();            /* Fork a child process to run user's command */
	    if (p1 < 0) {           /* If fork error, exit */
            perror(" fork error");
            _exit(1);
	    }

	    /* Child Process to run command */
	    if (p1 == 0) {
            /* Check if have a pipe operator */
            ps_argv[0] = strtok(buf, "|");

            /* Note: if buff has no pipe operator, */
            /* this for expression won't be execute */
            for(i=1; (ps_argv[i]=strtok(NULL,"|")); i++);

            if (pipe(fd)<0)     /* Create a pipe */
		    {
                perror("pipe");
                _exit(1);
		    }

            /* Create a child process and communicate */
            /* by pipe*/
            if ( (p2=fork()) <0){
                perror("fork 2");
                _exit(1);
		    }

            if (p2 > 0)         /* in the parent process */
		    {
                /* save STDOUT_FILENO into save_fd, in order to recovery stdout */
                /* int dup(int oldfd), copy the oldfd to a min unused descriptor */
                save_fd = dup(STDOUT_FILENO);

                /* copy fd[1] to stdout, now you write to stdout, \
                   it write to fd[1] actually */
                dup2(fd[1],STDOUT_FILENO);
                /* parent should close the read pipe fd[0] */
                close(fd[0]);
                /* And we have the stdout as fd[1], now the fd[1] can be closed. */
                close(fd[1]);

                /* Check if have input redirect '<' */
                if(strchr(ps_argv[0],(int)'<'))
			    {
                    /*如果命令包含输入重定向，
                      则分别执行命令和打开重定向文件*/

                    /*对每个管道中的命令提取命令和重定
                      向的文件名，argv1[0]保存命令，
                      argv1[1+]保存文件名*/
                    argv1[0] = strtok(ps_argv[0], " < ");
                    for(i=1; (argv1[i]=strtok(NULL," < ")); i++);

                    int temp=open(argv1[1],O_RDONLY);
                    if (temp < 0)
				    perror("open argv1[1] error");

                    read(temp,bufile,MAX_STR);
                    argv1[1] = strtok(bufile," ");
                    for(i=2; (argv1[i]=strtok(NULL," "));i++);
                    argv1[i-1]=NULL;
			    }
                else {   /* Don't have a redirect, just run command */
                    /* Get command */
                    argv1[0] = strtok(ps_argv[0]," ");

                    /* Get every parameters */
                    for(i=1; (argv1[i]=strtok(NULL," ")); i++);
			    }

                /* Call the execvp functin and run the command */
                if(execvp(argv1[0],argv1) < 0) {
                    perror("unaviable command");
                }

                /* Redirect the stdout to display */
                dup2(save_fd,STDOUT_FILENO);
		    }
            else {              /* in the child process */
                char buff[1400]={0};
                int nn;

                dup2(fd[0],STDIN_FILENO); /* Redirect stdio to fd[0]. */
                close(fd[1]);  /* Child should close the read pipe. */
                close(fd[0]);  /* Now we have the stdio, so close the fd[0]. */


                /* Child process run the second command after pipe operator */
                /* This command be saved in the ps_argv[1] */
                if(ps_argv[1]) { /* If user's input have pipe operator, \
                                    the ps_argv[1] saves the second parameters */
                    /* Check if this command request to redirect to a file */
                    if(strchr(ps_argv[1],(int)'>')) { /* Request to redirect */
                        /* Get the command and file name respectively */
                        /* argv2[0] is command and paramepters. */
                        /* argv2[1] is redirected files name */
                        argv2[0] = strtok(ps_argv[1]," > ");
                        for(i=1; (argv2[i]=strtok(NULL," > ")); i++);

                        int temp=open(argv2[1],O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
                        if(temp < 0) {
                            perror("open argv2[1]");
                        }
                        /* Redirect the output to target file */
                        dup2(temp, STDOUT_FILENO);
                        /* Copy the argv2[0] to the ps_argv[1] which will be run as command. */
                        ps_argv[1]=argv2[0];
                    }

                    /* However ps_argv[1] is the command, get the command and parameters */
                    /* After this, the argv2[0] is the command, and argv2[1+] is parameters. */
                    argv2[0] = strtok(ps_argv[1], " ");
                    for(i=1; (argv2[i]=strtok(NULL, " "));i++);

                    /* This is for support the alias "clr" */
                    if(argv2[0]=="clr") argv2[0]="clean";
                    /* Execute the command with parameters. */
                    execvp(argv2[0],argv2);
                }
                else {
                    nn=read(STDIN_FILENO,buff,1400);
                    write(STDOUT_FILENO,buff,nn);
                }
            }
            break;
	    }
        else {                  /* Our prompt process go to sleep to wait  */
            wait(&wt);          /* child process return. */
        }
    }
    return 0;
}
