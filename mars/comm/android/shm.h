// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#ifndef _SHM_H_
#define _SHM_H_

#ifdef ANDROID

#include <asm/unistd.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

__inline int shmget(key_t key, size_t size, int shmflg) {
    return syscall(__NR_shmget, key, size, shmflg);
}

__inline int shmdt(const void* shmaddr) {
    return syscall(__NR_shmdt, shmaddr);
}

__inline void* shmat(int shmid, const void* shmaddr, int shmflg) {
    return (void*)syscall(__NR_shmat, shmid, shmaddr, shmflg);
}

#ifdef __cplusplus
}
#endif

#endif

#endif
