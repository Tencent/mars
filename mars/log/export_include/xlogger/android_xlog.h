// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __ANDROID_XLOG__
#define __ANDROID_XLOG__

#include "xlogger/xloggerbase.h"

/*
 * Android log priority values, in ascending priority order.
 */
typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
} android_LogPriority;

static void __ComLogV(int level, const char *tag, const char* file, const char* func, int line, const char* fmt, va_list args) {
 	struct XLoggerInfo_t info;
	info.level = (TLogLevel)level;
	info.tag = tag;
	info.filename = file;
	info.func_name = func;
	info.line = line;

    gettimeofday(&info.timeval, NULL);
	info.pid = -1;
	info.tid = -1;
	info.maintid = -1;

	xlogger_VPrint(&info, fmt, args);

	//important: do not use like  this:xlogger2((TLogLevel)level, tag, file, func, line, fmt, args);
	//xlogger2((TLogLevel)level, tag, file, func, line).VPrintf(fmt, args);
}

static void __ComLog(int level, const char *tag, const char* file, const char* func, int line, const char* fmt, ...) {
	va_list args;

	va_start(args,fmt);
	__ComLogV(level, tag, file, func, line,fmt, args);
	va_end(args);
}



#define __LOG__(LEVEL, LOG_TAG, FMT, ...)  if ((!xlogger_IsEnabledFor(LEVEL))); else __ComLog(LEVEL, LOG_TAG , __FILE__, __FUNCTION__, __LINE__, FMT,## __VA_ARGS__)
#define __LOGV__(LEVEL, LOG_TAG, FMT, VA_LIST)  if ((!xlogger_IsEnabledFor(LEVEL))); else __ComLogV(LEVEL, LOG_TAG , __FILE__, __FUNCTION__, __LINE__, FMT, VA_LIST)

#define LOGV(LOG_TAG, FMT, ...)  __LOG__(kLevelVerbose, LOG_TAG, FMT, ##__VA_ARGS__)
#define LOGD(LOG_TAG, FMT, ...)  __LOG__(kLevelDebug, LOG_TAG, FMT, ##__VA_ARGS__)
#define LOGI(LOG_TAG, FMT, ...)  __LOG__(kLevelInfo, LOG_TAG, FMT, ##__VA_ARGS__)
#define LOGW(LOG_TAG, FMT, ...)  __LOG__(kLevelWarn, LOG_TAG, FMT, ##__VA_ARGS__)
#define LOGE(LOG_TAG, FMT, ...)  __LOG__(kLevelError, LOG_TAG, FMT, ##__VA_ARGS__)

#define __android_log_print(PRIO, TAG, FMT, ...) __LOG__((TLogLevel)(PRIO-2), TAG , FMT, ##__VA_ARGS__)
#define __android_log_write(PRIO, TAG, TEXT)	__LOG__((TLogLevel)(PRIO-2), TAG, TEXT)
#define __android_log_vprint(PRIO, TAG, FMT, VA_LIST)	__LOG__((TLogLevel)(PRIO-2), TAG, FMT, VA_LIST)
#define __android_log_assert(COND, TAG, FMT, ...)	if (((COND) || !xlogger_IsEnabledFor(kLevelFatal)));else {\
                                   					XLoggerInfo info= {kLevelFatal, TAG, __FILE__, __FUNCTION__, __LINE__,\
                                    				{0, 0}, -1, -1, -1};\
                                    				gettimeofday(&info.timeval, NULL);\
                                    				xlogger_AssertP(&info, #COND, FMT, ##__VA_ARGS__);}

#endif

