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
        int i,n,fd[2],save_fd;

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
                if (p1 == 0) {  //在子进程中执行下列代码，父进程进入睡眠状态pipe

                    ps_argv[0] = strtok(buf, "|");   //以管道符为界提取命令，放在ps_argv[]数组中
                    for(i=1; (ps_argv[i]=strtok(NULL,"|")); i++);   //如果有多个管道符

                    if (pipe(fd)<0)
                    {
                        perror("pipe");
                        _exit(1);
                    }
                    if ( (p2=fork()) <0)  //调用fork()创建子进程，进行管道通信
                    {
                        perror("fork 2");
                        _exit(1);
                    }
                    if (p2 > 0) //parent
                    {
                        close(fd[0]);   //父进程先关闭读取端
                        save_fd = dup(STDOUT_FILENO);  //save STDOUT_FILENO into save_fd
                        if(strchr(ps_argv[0],(int)'<'))    //查看命令中是否有输入重定向符号 '<'
                        {
                            //如果命令包含输入重定向，则分别执行命令和打开重定向文件
                            //对每个管道中的命令提取命令和重定向的文件名，argv1[0]保存命令，argv1[1+]保存文件名
                            argv1[0] = strtok(ps_argv[0], "<");
                            for(i=1; (argv1[i]=strtok(NULL,"<")); i++);

                            int temp=open(argv1[1],O_RDONLY);
                            if (temp < 0)
                            perror("open argv1[1] error");

                            read(temp,bufile,MAX_STR);
                            argv1[1] = strtok(bufile," ");
                            for(i=2; (argv1[i]=strtok(NULL," "));i++);
                            argv1[i-1]=NULL;
                        }
                        else
                        {
                            //如果命令不包含输入重定向，则只需执行命令及其参数

                            argv1[0] = strtok(ps_argv[0]," ");   //提取命令
                            for(i=1; (argv1[i]=strtok(NULL," ")); i++);   //提取命令的每个参数
                        }

                                //for(i=0;i<=3;i++) { write(1,argv1[i],sizeof(argv1[i])); }

                        dup2(fd[1],STDOUT_FILENO);  //将STDOUT_FILENO重定向到fd[1]，即管道的写入端
                        execvp(argv1[0],argv1);   //将每个参数传给命令并执行
                        perror("unaviable command");
                        dup2(save_fd,STDOUT_FILENO);   //将STDOUT_FILENO重新定向到标准输出即屏幕
                    }
                    else { //child

                        char buff[1400]={0};
                        int nn;

                        close(fd[1]);   //子进程关闭写入端
                        nn=read(fd[0],buff,1400);
                        if(ps_argv[1])   //子进程执行管道的第二个命令，此命令保存在ps_argv[1]中
                        {
                            if(strchr(ps_argv[1],(int)'>'))   //检测这个命令中有输出重定向文件要求
                            {
                                argv2[0] = strtok(ps_argv[1],">");   //分别提取命令部分和输出文件名
                                for(i=1; (argv2[i]=strtok(NULL,">")); i++);

                                int temp=open(argv2[1],O_WRONLY);
                                if(temp < 0)
                                    perror("open argv2[1]");

                                dup2(temp, STDOUT_FILENO);
                                ps_argv[1]=argv2[0];
                            }

                            printf("test:%s\n",ps_argv[1]);   //打印第二个命令

                            argv2[0] = strtok(ps_argv[1], " ");   //分别提取这个命令和参数，argv2[0]为命令，argv2[1+]为执行参数
                            for(i=1; (argv2[i]=strtok(NULL, " "));i++);

                            argv2[i]="fd[0]";
                            argv2[i+1]=NULL;
                            write(1,argv2[0],sizeof(argv2[0]));
                            write(1,"|",1);
                            write(1,argv2[1],sizeof(argv2[1]));
                            write(1,"|",1);
                            write(1,argv2[2],sizeof(argv2[2]));
                            write(1,"|",1);
                            write(1,argv2[3],sizeof(argv2[3]));
                            write(1,"|",1);

                            printf("\n");
                            execvp(argv2[0],argv2);
                        } else
                        write(STDOUT_FILENO,buff,nn);
                    }
                    break;
                }
                else {
                    sleep(1);  //父进程进入睡眠态，睡眠1s，等待子进程返回
                }
        }
        return 0;
}
