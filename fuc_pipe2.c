#include<unistd.h>
#include<stdio.h>
int fd[2];
void run_ls()
{
    char *argv1[]={"ls",NULL};

    dup2(fd[1],1);
    close(fd[0]);
    close(fd[1]);

    execve("/bin/ls",argv1,NULL);
}
void run_wc()
{
    char *argv2[]={"wc",NULL};

    dup2(fd[0],0);
    close(fd[0]);
    close(fd[1]);

    execve("/usr/bin/wc",argv2,NULL);
}
int main()
{
    pipe(fd);
    if(fork()==0)
	run_ls();
    else
	run_wc();

    return 0;
}
