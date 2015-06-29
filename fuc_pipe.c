#include <unistd.h>  
#include <stdio.h>  
#include <sys/types.h>  
      
int main()  
{  
    int n, fd[2];  
    pid_t pid;  
    char buffer[BUFSIZ+1];  

    /*pipe failed*/
    if(pipe(fd)<0)  
        {  
            printf("pipe failed!\n ");  
            exit(1);  
        }  
    
    /*fork failed*/
    if((pid=fork())<0)  
        {  
            printf("fork failed!\n ");  
            exit(1);  
        }  

    /*parent process write some words into pipe*/
    else if (pid>0)  
        {  
            close(fd[0]);  
            write(fd[1],"How are you?\n",12);  
        }  

    /*child process receive words from pipe*/
    else  
        {  
            close(fd[1]);  
            n=read(fd[0],buffer,BUFSIZ);  
            write(STDOUT_FILENO,buffer,n);  
        }  

    exit(0);  
}  
