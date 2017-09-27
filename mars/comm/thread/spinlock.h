// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


//
//  spinlock.h
//
//

#ifndef spinlock_h
#define spinlock_h

#ifdef __APPLE__
#include <libkern/OSAtomic.h>

#define splock OSSpinLock
#define splockinit(lock) {*lock = OS_SPINLOCK_INIT;}
#define splocklock OSSpinLockLock
#define splockunlock OSSpinLockUnlock
#define splocktrylock OSSpinLockTry

class SpinLock
{
public:
    typedef splock handle_type;
     
public:
     SpinLock(){ splockinit(&lock_);}
     
     bool lock()
     {
         splocklock(&lock_);
         return true;
     }
     
     bool unlock()
     {
         splockunlock(&lock_);
         return true;
     }
     
     bool trylock()
     {
         return splocktrylock(&lock_);
     }
     
    splock* internal() { return &lock_; }
private:
     SpinLock(const SpinLock&);
     SpinLock& operator = (const SpinLock&);
     
private:
     splock lock_;
};

#else


#ifdef __powerpc__
#include "../../arch/powerpc/include/uapi/asm/unistd.h"
#endif

#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER >= 1310 && ( defined(_M_ARM) )
	extern "C" void YieldProcessor();
#else
	extern "C" void _mm_pause();
#endif
#endif

static inline void cpu_relax() {

#if defined(__arc__) || defined(__mips__) || defined(__arm__) || defined(__powerpc__)
	asm volatile("" ::: "memory");
#elif defined(__i386__) || defined(__x86_64__)
	asm volatile("rep; nop" ::: "memory");
#elif defined(__aarch64__)
	asm volatile("yield" ::: "memory");
#elif defined(__ia64__)
	asm volatile ("hint @pause" ::: "memory");

#elif defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER >= 1310 && ( defined(_M_ARM) )
	YieldProcessor();
#else
	_mm_pause();
#endif
#endif

}


#ifdef _WIN32
#include <thr/threads.h>
extern "C" void thrd_yield();
#define sched_yield() thrd_yield()
#else
#include <sched.h>
#endif
#include "atomic_oper.h"

class SpinLock
{
public:
     typedef uint32_t handle_type;
     
private:
     enum state
     {
         initial_pause = 2,
         max_pause = 16
     };

     uint32_t state_;

public:
     SpinLock() : state_(0) {}

     bool trylock()
     {
         return (atomic_cas32((volatile uint32_t *)&state_, 1, 0) == 0);
     }

     bool lock()
     {
         /*register*/ unsigned int pause_count = initial_pause; //'register' storage class specifier is deprecated and incompatible with C++1z
         while (!trylock())
         {
             if (pause_count < max_pause)
             {
                 for (/*register*/ unsigned int i = 0; i < pause_count; ++i) //'register' storage class specifier is deprecated and incompatible with C++1z
                 {
                     cpu_relax();
                 }
                 pause_count += pause_count;
             } else {
                 pause_count = initial_pause;
                 sched_yield();
             }
         }
         return true;
     }

     bool unlock()
     {
         atomic_write32((volatile uint32_t *)&state_, 0);
         return true;
     }
     
    uint32_t* internal() { return &state_; }

private:
     SpinLock(const SpinLock&);
     SpinLock& operator = (const SpinLock&);
};

#endif
#endif
