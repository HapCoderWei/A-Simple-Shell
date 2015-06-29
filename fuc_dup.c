#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
int main(void)
{
     int fd, save_fd;
     char msg[] = "This is a test of dup() & dup2()\n";
     int test;
     fd = open("somefile", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
     if(fd<0) {
         perror("open");
         exit(1);
     }

     //运行后save_fd指向STDOUT_FILENO，即save_fd指向标准输出
     save_fd = dup(STDOUT_FILENO);     
     //测试用              
     printf("save_fd=%d\n",save_fd);          
     
     //运行后STDOUT_FILENO指向fd所指向的文件，即STDOUT_FILENO指向somefile
     test=dup2(fd, STDOUT_FILENO);   
     //测试用 此时的标准输出不再指向显示器，因此该段测试将写入somefile文件中                 
     printf("dup2_1=%d\n",test); 
                                  
     close(fd);

     //此时STDOUT_FILENO所指向的是somefile文件不再是标准输出流,因此该段将     
     write(STDOUT_FILENO, msg, strlen(msg)); 
     
     //运行后STDOUT_FILENO指向save_fd所指向的文件,即标准输出流          
     test=dup2(save_fd, STDOUT_FILENO);         
      
     //测试用 此时标准输出流重新指回显示器，因此该段测试将写入显示器
     printf("dup2_2=%d\n",test); 
     //此时STDOUT_FILENO所指向的便回标准输出流该段将写入显示器
     write(STDOUT_FILENO, msg, strlen(msg));           

     close(save_fd);

     return 0;
}
