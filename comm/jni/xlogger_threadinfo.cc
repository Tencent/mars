//
//  xloggr_threadinfo.m
//  MicroMessenger
//
//  Created by 叶润桂 on 13-3-13.
//  Copyright (c) 2013年 Tencent. All rights reserved.
//

#include <unistd.h>
#include "compiler_util.h"

extern "C"
{
EXPORT_FUNC intmax_t xlogger_pid()
{
    static intmax_t pid = getpid();
    return pid;
}

EXPORT_FUNC intmax_t xlogger_tid()
{
    return gettid();
}

EXPORT_FUNC intmax_t xlogger_maintid()
{
    static intmax_t pid = getpid();
    return pid;
}
}
