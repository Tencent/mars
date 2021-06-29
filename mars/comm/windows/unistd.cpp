
<<<<<<< HEAD:mars/comm/windows/unistd.cpp
#include <xthreads.h>
#define thrd_sleep(tm)			_Thrd_sleep(tm)
=======
#include "unistd.h"
#include <chrono>
#include <thread>
>>>>>>> alanzyzhang/master/windows_vs2019:mars/comm/windows/unistd.cc

static void thread_sleep(unsigned long _sec, unsigned long _nanosec)
{
    std::this_thread::sleep_for(std::chrono::seconds(_sec)+std::chrono::nanoseconds(_nanosec));
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

