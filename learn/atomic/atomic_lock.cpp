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
#include "atomic_lock.h"

using namespace std;

static std::string g_UdpSockPath = "/tmp/peacewang/thunder";
static bool m_isStop = false;

void sig_handle(int signo)
{
    m_isStop = true;
}

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
    signal(SIGTERM, sig_handle);

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
    while ((*dst++ = *src++))
        ;
    return dst - 1;
}

/*int32_t trylock_resv_mutex(int srvfd, fd_set* pfdset)
{
    pid_t pid = getpid();
    if (pf_shmtx_trylock(&g_shmtx))
    {
        printf("pf_shmtx_trylock succ 0,pid[%d]\n", pid);
        if (g_resv_mutex_held)
        {
            printf("pf_shmtx_trylock succ 1,pid[%d]\n", pid);
            return 0;
        }

        printf("pf_shmtx_trylock succ 2,pid[%d]\n", pid);
        FD_SET(srvfd, pfdset);
        g_resv_mutex_held = true;

        return 0;
    }

    if (g_resv_mutex_held)
    {
        printf("pf_shmtx_trylock fail 0,pid[%d]\n", pid);
        FD_CLR(srvfd, pfdset);
        g_resv_mutex_held = false;
    }

    printf("pf_shmtx_trylock fail 1,pid[%d]\n", pid);
    return 0;
}*/

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage:%s procsize \n", argv[0]);
        return 1;
    }

    InitDaemon();

    pf_shmtx_t g_shmtx;
    if (pf_shmtx_create(&g_shmtx) != 0)
    {
        printf("create shmtx failed\n");
        return -1;
    }

    int iProcSize = atoi(argv[1]);

    char szCmd[1024] = { 0 };
    snprintf(szCmd, sizeof(szCmd), "rm -rf %s && mkdir -p %s", g_UdpSockPath.c_str(), g_UdpSockPath.c_str());
    system(szCmd);

    int srvFd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (srvFd < 0)
    {
        printf("create socket failed\n");
        return -1;
    }

    mode_t old_mod = umask(S_IRWXO);

    struct sockaddr_un stUNIXAddr;
    memset(&stUNIXAddr, 0, sizeof(stUNIXAddr));
    stUNIXAddr.sun_family = AF_LOCAL;
    std::string strBindCmd = g_UdpSockPath + "/thunder_0";
    StrMov(stUNIXAddr.sun_path, strBindCmd.data());

    const int on = 1;
    setsockopt(srvFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    unlink(g_UdpSockPath.data()); // unlink it if exist

    if ((bind(srvFd, (struct sockaddr *) &(stUNIXAddr), SUN_LEN(&(stUNIXAddr)))) == -1)
    {
        close(srvFd);
        std::stringstream oss;
        oss << "bind socket failed";
        oss << ", sun_path:" << stUNIXAddr.sun_path;
        oss << ", errno:" << errno << ", strerror:" << strerror(errno);
        printf("%s\n", oss.str().c_str());
        //return -1;
    }

    int iSocketOptReceiveBufferSize = 2 * 1024 * 1024;
    setsockopt(srvFd, SOL_SOCKET, SO_RCVBUF, (char*) &iSocketOptReceiveBufferSize, sizeof(iSocketOptReceiveBufferSize));

    umask(old_mod); // Set back umask

    int iCurrentFlag = 0;
    if ((iCurrentFlag = fcntl(srvFd, F_GETFL, 0)) == -1)
    {
        close(srvFd);
        printf("fcntl F_GETFL failed\n");
        return -1;
    }

    if (fcntl(srvFd, F_SETFL, iCurrentFlag | FNDELAY) == -1)
    {
        close(srvFd);
        printf("fcntl F_SETFL failed\n");
        return -1;
    }

    int flags = 0;
    flags = fcntl(srvFd, F_GETFL, 0);
    if (flags != -1)
    {
        fcntl(srvFd, F_SETFL, flags | O_NONBLOCK);
    }

    for (int i = 0; i < iProcSize; i++)
    {
        int fpid = fork();
        if (fpid < 0)
        {
            printf("fork failed\n");
            continue;
        }

        if (fpid == 0)
        {
            int iAcceptCnt = 0;
            char resvBuff[1024] = { 0 };
            bool g_resv_mutex_held = false;
            pid_t pid = getpid();
            while (!m_isStop)
            {
                fd_set setReadFds;
                FD_ZERO(&setReadFds);
                if (pf_shmtx_trylock(&g_shmtx,pid))
                {
                    FD_SET(srvFd, &setReadFds);
                    if (g_resv_mutex_held)
                    {
                        //printf("pf_shmtx_trylock succ 1,pid[%d]\n", pid);
                    }
                    else
                    {
                        printf("pf_shmtx_trylock succ 2,pid[%d]\n", pid);
                        g_resv_mutex_held = true;
                    }
                }
                else
                {
                    if (g_resv_mutex_held)
                    {
                        printf("pf_shmtx_trylock fail 2,pid[%d]\n", pid);
                        FD_CLR(srvFd, &setReadFds);
                        g_resv_mutex_held = false;
                    }
                    else
                    {
                        //printf("pf_shmtx_trylock fail 1,pid[%d]\n", pid);
                    }
                }

                struct timeval m_stTimeVal;
                m_stTimeVal.tv_sec = 0;
                m_stTimeVal.tv_usec = 10000;

                int iSelect = select(srvFd+1, &setReadFds, NULL, NULL, &m_stTimeVal);
                if (g_resv_mutex_held)
                {
                    pf_shmtx_unlock(&g_shmtx,pid);
                }
                if (iSelect > 0)
                {
                    iAcceptCnt++;
                    printf("select succ count[%d],pidx[%d] pid[%d]\n", iAcceptCnt, i, getpid());

                    //usleep(10000);
                    if (FD_ISSET(srvFd, &setReadFds))
                    {
                        int m_iRecvBufLen = recvfrom(srvFd, resvBuff, 1024, 0, NULL, NULL);
                        if (m_iRecvBufLen > 0)
                        {
                            printf("accept succ,data[%s]\n", resvBuff);
                        }
                        else
                        {
                            printf("accept fail\n");
                        }
                    }
                }
            }

            return 0;
        }
    }
}
