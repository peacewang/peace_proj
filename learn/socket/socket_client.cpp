#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 4096

int main(int argc, char* argv[])
{
    int iRet = 0;
    do
    {
        if(argc < 3)
        {
            printf("usage: ./%s ip port\n",argv[0]);
            break;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
            printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
            break;
        }

        struct sockaddr_in servaddr;
        memset(&servaddr,0,sizeof(sockaddr_in));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(argv[2]));
        iRet = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
        if(iRet <= 0)
        {
            printf("inet_pton error for %s\n",argv[1]);
            break;
        }

        iRet = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if(iRet < 0)
        {
            printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
            break;
        }

        printf("connect succ, now send msg to server: \n");
        char sendline[MAXLINE];
        char buf[MAXLINE];
        while(true)
        {
            fgets(sendline, 4096, stdin);
            iRet = send(sockfd, sendline, strlen(sendline), 0);
            if(iRet < 0)
            {
                printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                break;
            }

            if(sendline[0] == 'b' && sendline[1] == 'y' && sendline[2] == 'e')
            {
                printf("bye...: \n");
                break;
            }

            iRet = recv(sockfd, buf, MAXLINE,0);
            if(iRet < 0)
            {
                perror("recv error");
                break;
            }

            buf[iRet] = '\0';
            printf("recv succ from server : %s\n",buf);
        }

        close(sockfd);
    }
    while(0);

    return 0;
}
