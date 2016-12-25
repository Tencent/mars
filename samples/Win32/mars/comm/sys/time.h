/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
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
