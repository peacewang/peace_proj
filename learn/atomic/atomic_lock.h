#ifndef _ATOMIC_LOCK_H_
#define _ATOMIC_LOCK_H_

#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

typedef volatile uint32_t  pf_atomic_t;

typedef struct
{
    pf_atomic_t  *lock;
} pf_shmtx_t;

static inline uint32_t pf_atomic_cmp_set(pf_atomic_t *lock, uint32_t old, uint32_t set)
{
    unsigned char  res;

    __asm__ volatile (

    "lock;"
    "    cmpxchgl  %3, %1;   "
    "    sete      %0;       "

    : "=a" (res) : "m" (*lock), "a" (old), "r" (set) : "cc", "memory");
    return res;
}

static inline int32_t pf_shmtx_create(pf_shmtx_t *mtx)
{
    void * pShmAddr = mmap(NULL, 128, PROT_READ|PROT_WRITE,MAP_ANON|MAP_SHARED, -1, 0);
    if (pShmAddr == MAP_FAILED)
    {
        return -1;
    }

    mtx->lock = (pf_atomic_t*)pShmAddr;
    return 0;
}

static inline bool pf_shmtx_trylock(pf_shmtx_t *mtx,int pid)
{
    return (*(mtx->lock) == 0 && pf_atomic_cmp_set(mtx->lock, 0, pid));
}

#define pf_shmtx_unlock(mtx,pid) (void) pf_atomic_cmp_set((mtx)->lock, pid, 0)

#endif
