#include "unistd.h"
#include <thread>
#include <chrono>

static void thread_sleep(unsigned long _sec, unsigned long _nanosec)
{
	std::this_thread::sleep_for(std::chrono::nanoseconds(_nanosec)+std::chrono::seconds(_sec));
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

#ifdef WP8
#include <windows.h>

int getpid(void)
{
    return GetCurrentProcessId();
}

#endif
