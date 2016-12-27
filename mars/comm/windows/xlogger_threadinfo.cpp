  //
//  xloggr_threadinfo.m
//  MicroMessenger
//
//  Created by 叶润桂 on 13-3-13.
//  Copyright (c) 2013年 Tencent. All rights reserved.
//


#include <windows.h>
#include <stdint.h>


extern "C"
{
intmax_t xlogger_pid()
{
    static intmax_t pid = GetCurrentProcessId();
    return pid;
}

intmax_t xlogger_tid()
{
    return GetCurrentThreadId();
}

static intmax_t sg_maintid = GetCurrentThreadId();
intmax_t xlogger_maintid()
{
    return sg_maintid;
}
}

