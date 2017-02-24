#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_PORT 9999
#define MAXLINE 4096

void sigchld_handler(int signo)
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
}

int main(int argc, char** argv)
{
    signal(SIGCHLD, sigchld_handler);//加了个子程序退出的信号
    do
    {
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(socket_fd == -1)
        {
            printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
            break;
        }

        struct sockaddr_in servaddr;
        memset(&servaddr,0,sizeof(sockaddr_in));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址
        servaddr.sin_port = htons(DEFAULT_PORT);//设置的端口为DEFAULT_PORT

        int iRet = bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if(iRet == -1)
        {
            printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
            break;
        }

        iRet = listen(socket_fd,100);
        if(iRet == -1)
        {
            printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
            break;
        }

        printf("======waiting for client's request======\n");
        int connect_fd;
        char buff[4096];
        int iResvLen = 0;
        while(true)
        {
            connect_fd = accept(socket_fd,(struct sockaddr*)NULL, NULL);
            if(connect_fd == -1)
            {
                printf("accept socket error: %s(errno: %d)\n",strerror(errno),errno);
                continue;
            }

            printf("======one client connected======\n");
            pid_t pid = fork();
            int status;
            if(pid < 0)
            {
                printf("fork error: %s(errno: %d)\n",strerror(errno),errno);
                continue;
            }
            else if(pid == 0)
            {
                //子进程
                while(true)
                {
                    iResvLen = recv(connect_fd,buff,MAXLINE,0);
                    if(iRet == -1)
                    {
                        perror("recv error");
                        break;
                    }

                    buff[iResvLen] = '\0';
                    printf("recv msg from client: %s\n", buff);

                    if(buff[0] == 'b' && buff[1] == 'y' && buff[2] == 'e')
                    {
                        printf("one connect socket exit...: \n");
                        break;
                    }

                    iRet = send(connect_fd, "msg arrived", 11,0);
                    if(iRet == -1)
                    {
                        perror("send error");
                        break;
                    }
                }

                close(connect_fd);
                exit(0);
            }

            close(connect_fd);
        }

        close(socket_fd);
    }
    while(0);

    return 0;
}
