#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>

/*
    struct ipc_perm
    {
        key_t  key;
        ushort uid;    owner euid and egid
        ushort gid;
        ushort cuid;   creator euid and egid
        ushort cgid;
        ushort mode;   access modes see mode flags below
        ushort seq;    slot usage sequence number
    }

    struct shmid_ds
    {
        struct ipc_perm shm_perm;  operation perms
        int shm_segsz;  size of segment (bytes)
        time_t shm_atime;  last attach time
        time_t shm_dtime;  last detach time
        time_t shm_ctime;  last change time
        unsigned short shm_cpid;  pid of creator
        unsigned short shm_lpid;  pid of last operator
        short shm_nattch;  no. of current attaches

         the following are private

        unsigned short shm_npages;  size of segment (pages)
        unsigned long *shm_pages;  array of ptrs to frames -> SHMMAX
        struct vm_area_struct *attaches;  descriptors for attaches
    };
*/

int main()
{
    int iKey = 0x1024;
    int iSize = 1024;
    int shmid = shmget((key_t) iKey, iSize, 0666|IPC_CREAT);
    if (shmid < 0)
    {
        printf("shmget failed,error[%d],errmsg[%s]\n", errno, strerror(errno));
        return -1;
    }

    struct shmid_ds ds;
    struct ipc_perm shm_perm;
    if(shmctl(shmid, IPC_STAT, &ds) < 0)
    {
        printf("shmctl failed,error[%d],errmsg[%s]\n", errno, strerror(errno));
        return -1;
    }

    shm_perm = ds.shm_perm;
    int iFinalSize = 2048;
    if(ds.shm_segsz != iFinalSize)
    {
        if(shmctl(shmid, IPC_RMID, NULL))
        {
            printf("shmctl failed,error[%d],errmsg[%s]\n", errno, strerror(errno));
            return -1;
        }

        shmid = shmget(iKey, iFinalSize, 0666|IPC_CREAT);
        if(shmid<0)
        {
            printf("shmget failed,error[%d],errmsg[%s]\n", errno, strerror(errno));
            return -1;
        }

        if(shmctl(shmid, IPC_STAT, &ds) < 0)
        {
            printf("shmctl failed,error[%d],errmsg[%s]\n", errno, strerror(errno));
            return -1;
        }

        shm_perm = ds.shm_perm;
    }

    void* shm = shmat(shmid, 0, 0);
    if (shm == (void*) -1)
    {
        printf("shmat failed,error[%d],errmsg[%s]\n", errno, strerror(errno));
        return -1;
    }

    memset(shm,'a',iFinalSize);

    shmdt(shm);
    return 0;
}
