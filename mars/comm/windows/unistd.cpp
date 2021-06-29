
#include <xthreads.h>
#define thrd_sleep(tm)			_Thrd_sleep(tm)

static void thread_sleep(unsigned long _sec, unsigned long _nanosec)
{
    struct xtime xt = {0, 0};
    xtime_get(&xt, TIME_UTC);
    xt.sec += _sec;
    xt.nsec += _nanosec;
    thrd_sleep(&xt);
}

unsigned int sleep(unsigned int _sec)
{
    thread_sleep(_sec, 0);
    return 0;
}

void usleep(unsigned long _usec)
{
    thread_sleep(0, _usec);
}

