#ifndef __SYS__TIME_H__
#define __SYS__TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <winsock2.h>
//#ifndef _WINSOCK2API_
//struct timeval {
//        long    tv_sec;         /* seconds */
//        long    tv_usec;        /* and microseconds */
//};
//#endif

//struct timespec {
//    long    tv_sec;         
//    long    tv_nsec;        
//};
	

struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);

#ifdef __cplusplus
}
#endif

#endif
