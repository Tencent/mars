// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * fatal_assert.h
 *
 *  Created on: 2014-4-23
 *      Author: yanguoyue
 */

#ifndef FATAL_ASSERT_H_
#define FATAL_ASSERT_H_

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>

# if (!__ISO_C_VISIBLE >= 1999)
#error "C Version < C99"
# endif

#define    FATAL_ASSERT2(e, fmt, ...)     ((e) ? (void)0 : __FATAL_ASSERT2(__FILE__, __LINE__, __func__, #e, fmt, ##__VA_ARGS__))
#define    FATAL_ASSERT(e)     FATAL_ASSERT2(e, "")

__BEGIN_DECLS

__inline int __sigqueue(pid_t pid, int sig, const union sigval value) {
    siginfo_t info;
    memset(&info, 0, sizeof(siginfo_t));
    info.si_signo = sig;
    info.si_code = SI_QUEUE;
    info.si_pid = getpid();
    info.si_uid = getuid();
    info.si_value = value;

    return  syscall(__NR_rt_sigqueueinfo, pid, sig, &info);
}

__attribute__((__nonnull__(1, 3, 4, 5))) __attribute__((__format__(printf, 5, 6)))
__inline  void __FATAL_ASSERT2(const char* _file, int _line, const char* _func, const char* _expression, const char* _format, ...) {
#pragma pack(push, 1)
    static struct {
        char magiccode[4];
        int  len;
        char data[4096 - sizeof(len) - sizeof(magiccode)];
    } extra_info = {{'a', '\x02', '\x02', 'd'}, sizeof(extra_info.data), {0}};
#pragma pack(pop)

    va_list valist;
    va_start(valist, _format);

    memset(extra_info.data, 0, sizeof(extra_info.data));
    int offset = snprintf(extra_info.data, sizeof(extra_info.data), "%s, %s, %d:[%s],", _file, _func, _line, _expression);
    vsnprintf(extra_info.data + offset, sizeof(extra_info.data) - (size_t)offset, _format, valist);

    va_end(valist);

    union sigval value;
    value.sival_ptr = (void*)&extra_info;

#ifndef NDEBUG
    raise(SIGTRAP);
#endif
    __sigqueue(getpid(), SIGABRT, value);
    abort();

    // __assert2(_file, _line, _func, _expression);
}
__END_DECLS

#endif /* FATAL_ASSERT_H_ */
