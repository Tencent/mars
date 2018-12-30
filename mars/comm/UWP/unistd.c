
#include <thr/threads.h>
#include "unistd.h"

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

#ifdef WP8
#include <windows.h>

int getpid(void)
{
    return GetCurrentProcessId();
}


#ifdef UWP
int _mkdir4UWP(const char *pathname, mode_t mode)
{
	wchar_t* wszRet;
	int len = strlen(pathname);
	int size = MultiByteToWideChar(CP_ACP, 0, (char*)pathname, len, NULL, 0);
	if (size <= 0)
		return -1;

	wszRet = malloc((size + 1)*sizeof(wchar_t));
	memset(wszRet, 0, (size + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, (char*)pathname, len, wszRet, size);

	//wchar_t *oldwpath = toWcs(pathname, strlen(pathname));

	BOOL ret = CreateDirectoryW(wszRet, NULL);
	free(wszRet);
	wszRet = NULL;

	if (ret == FALSE)
	{
		//LOGE(tag,"error: create dir failed. %s",strPath.c_str());
		return -1;
	}

	return 0;
}
#endif

#endif
