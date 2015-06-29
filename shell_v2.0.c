#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAX_STR 30
#define MAX_PARA 5
int main()
{
    pid_t p1,p2;
    char buf[MAX_STR],bufile[MAX_STR], *ps_argv[MAX_PARA],*argv1[MAX_PARA],*argv2[MAX_PARA];
    int i,n,fd[2],save_fd, wt;

    while(1)
    {
	    write(STDOUT_FILENO,"myshell> ",9);
	    memset(buf,0,MAX_STR);
	    n = read(STDIN_FILENO, buf, MAX_STR-1);
	    buf[strlen(buf)-1]='\0';
	    if (n < 0) {
            perror("read error");
            _exit(1);
	    }
	    else if (n == 1) continue;    //n=1表示输入只有换行键

	    /******************************************************/
	    if ( !(strncmp(buf, "quit", 4))) {
            write(STDOUT_FILENO,"bye-bye!\n",9);
            break;
	    }

	    /******************************************************/
	    p1 = fork();   //通过fork()创建一个子进程,用来执行用户输入的命令
	    if (p1 < 0) {
            perror(" fork error");
            _exit(1);
	    }

	    //在子进程中执行下列代码，父进程进入睡眠状态pipe
	    if (p1 == 0) {
            //以管道符为界提取命令，放在ps_argv[]数组中
            ps_argv[0] = strtok(buf, "|");

            //maybe there has a problem in strtok
            for(i=1; (ps_argv[i]=strtok(NULL,"|")); i++);

            if (pipe(fd)<0)
		    {
                perror("pipe");
                _exit(1);
		    }

            //调用fork()创建子进程，进行管道通信
            if ( (p2=fork()) <0)
		    {
                perror("fork 2");
                _exit(1);
		    }

            if (p2 > 0) //parent
		    {
                //save STDOUT_FILENO into save_fd
                save_fd = dup(STDOUT_FILENO);

                //将STDOUT_FILENO重定向到fd[1]，即管道的写入端
                dup2(fd[1],STDOUT_FILENO);
                //父进程先关闭读取端
                close(fd[0]);
                close(fd[1]);

                //查看命令中是否有输入重定向符号 '<'
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
                else //如果命令不包含输入重定向，则只需执行命令及其参数
			    {
                    //提取命令
                    argv1[0] = strtok(ps_argv[0]," ");

                    //提取命令的每个参数
                    for(i=1; (argv1[i]=strtok(NULL," ")); i++);
			    }

                //将每个参数传给命令并执行
                execvp(argv1[0],argv1);

                perror("unaviable command");

                //将STDOUT_FILENO重新定向到标准输出即屏幕
                dup2(save_fd,STDOUT_FILENO);
		    }
            else { //child

                char buff[1400]={0};
                int nn;

                //将STDIN_FILENO重定向到fd[0]，即管道的读出端
                dup2(fd[0],STDIN_FILENO);
                //子进程关闭写入端
                close(fd[1]);
                close(fd[0]);
                //子进程执行管道的第二个命令，此命令保存在ps_argv[1]中
                if(ps_argv[1])
                {
                    //检测这个命令中有输出重定向文件要求
                    if(strchr(ps_argv[1],(int)'>'))
                    {
                        //分别提取命令部分和输出文件名
                        argv2[0] = strtok(ps_argv[1]," > ");
                        for(i=1; (argv2[i]=strtok(NULL," > ")); i++);

                        int temp=open(argv2[1],O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
                        if(temp < 0)
                        perror("open argv2[1]");
                        //将输出重定向到目标文件
                        dup2(temp, STDOUT_FILENO);

                        ps_argv[1]=argv2[0];
                    }

                    //分别提取这个命令和参数，argv2[0]为命令，argv2[1+]为执行参数
                    argv2[0] = strtok(ps_argv[1], " ");
                    for(i=1; (argv2[i]=strtok(NULL, " "));i++);


                    if(argv2[0]=="clr") argv2[0]="clean";
                    execvp(argv2[0],argv2);
                }
                else
                nn=read(STDIN_FILENO,buff,1400);
                write(STDOUT_FILENO,buff,nn);
            }
            break;
	    }
        else {  //父进程进入睡眠态，睡眠1s，等待子进程返回
            wait(&wt);
        }
    }
    return 0;
}
