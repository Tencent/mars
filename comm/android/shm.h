/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#ifndef _SHM_H_
#define _SHM_H_

#ifdef ANDROID

#include <asm/unistd.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

__inline int shmget(key_t key, size_t size, int shmflg)
{
    return syscall(__NR_shmget, key, size, shmflg);
}

__inline int shmdt(const void* shmaddr)
{
    return syscall(__NR_shmdt, shmaddr);
}

__inline void* shmat(int shmid, const void* shmaddr, int shmflg)
{
    return (void*)syscall(__NR_shmat, shmid, shmaddr, shmflg);
}

#ifdef __cplusplus
}
#endif

#endif

#endif
