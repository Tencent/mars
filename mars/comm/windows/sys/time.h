// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#ifndef __SYS__TIME_H__
#define __SYS__TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
// #ifndef _WINSOCK2API_
// struct timeval {
//        long    tv_sec;         /* seconds */
//        long    tv_usec;        /* and microseconds */
//};
// #endif

#if defined(_MSC_VER) && (_MSC_VER < 1900) // VS2015
struct timespec {
    long    tv_sec;
    long    tv_nsec;
};
#endif

struct timezone {
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};
int gettimeofday(struct timeval* tv, struct timezone* tz);

#ifdef __cplusplus
}
#endif

#endif
