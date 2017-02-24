#include <iostream>
#include <string>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/time.h>

using namespace std;

static std::string g_UdpSockPath = "/tmp/peacewang/thunder";

void InitDaemon(void)
{
    pid_t pid;
    int i;
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
    {
        perror("fail to fork.");
        exit(1);
    }
    /* now the process is the leader of the session, the leader of its
     process grp, and have no controlling tty */
    if (setsid() == -1)
    {
        perror("omail: setsid() fail. It seems impossible.");
        exit(1);
    }
    setpgid(0, 0);
    /*** change umask */
    umask(0027);
    /* ignore the SIGHUP, in case the session leader sending it when terminating.  */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGURG, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
    {
        perror("fail to fork.");
        exit(1);
    }
    /*** close all inherited file/socket descriptors */
    for (i = 3; i < getdtablesize(); i++)
        close(i);
    close(0);
}

char* StrMov(register char *dst, register const char *src)
{
    while((*dst++ = *src++)) ;
    return dst-1;
}

int main(int argc,char** argv)
{
    if(argc < 3)
    {
        printf("usage:%s procsize reqsize \n", argv[0]);
        return 1;
    }

    InitDaemon();

    int iProcSize = atoi(argv[1]);
    int iReqSize = atoi(argv[2]);
    int cliFd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if(cliFd < 0 )
    {
        printf("create socket failed\n");
        return -1;
    }

    int iSocketOptReceiveBufferSize = 2 * 1024 * 1024;
    setsockopt(cliFd, SOL_SOCKET, SO_RCVBUF, (char*)&iSocketOptReceiveBufferSize,  sizeof(iSocketOptReceiveBufferSize));

/*    int flags = 0;
    flags = fcntl(cliFd, F_GETFL, 0);
    if (flags != -1)
    {
        fcntl(cliFd, F_SETFL, flags | O_NONBLOCK);
    }*/

    struct sockaddr_un stUNIXAddr;
    memset(&stUNIXAddr, 0, sizeof(stUNIXAddr));
    stUNIXAddr.sun_family = AF_LOCAL;
    std::string strBindCmd = g_UdpSockPath + "/thunder_0";
    StrMov(stUNIXAddr.sun_path, strBindCmd.data());

    for(int i = 0; i < iProcSize; i++)
    {
        int fpid = fork();
        if(fpid < 0)
        {
            printf("fork failed\n");
            continue;
        }

        if(fpid == 0)
        {
            int iTotal = 0;
            struct timeval tv_begin, tv_end;
            gettimeofday(&tv_begin, NULL);
            for(int j = 0; j < iReqSize; j++)
            {
                ssize_t iBytesSent = sendto(cliFd,"h",(size_t)sizeof("h"),0,(struct sockaddr *)&(stUNIXAddr),SUN_LEN(&stUNIXAddr));
                if((int)iBytesSent == -1 || static_cast<uint32_t>(iBytesSent) != sizeof("h"))
                {
                    std::ostringstream oss;
                    oss << errno << ", strerror:" << strerror(errno);
                    printf("sendto peer failed errno = %s\n",oss.str().c_str());
                    continue;
                }
                iTotal++;
            }

            gettimeofday(&tv_end, NULL);
            long lSpendTime = (tv_end.tv_sec - tv_begin.tv_sec)*1000000 + (tv_end.tv_usec - tv_begin.tv_usec);
            double dSpendTime =  lSpendTime / 1000000.0;
            printf("send finish,total[%d] cost[%0.4f]\n",iTotal,dSpendTime);
            return 0;
        }
    }
}
