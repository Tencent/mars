
// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under
// the License.

#import <Foundation/Foundation.h>
#import <os/log.h>
#import "comm/objc/scope_autoreleasepool.h"
#include "comm/xlogger/loginfo_extract.h"
#include "comm/xlogger/xlogger.h"
#include "mars/xlog/appender.h"

namespace mars {
namespace xlog {

static TConsoleFun sg_console_fun = TConsoleFun::kConsoleOSLog;

void appender_set_console_fun(TConsoleFun _fun) { sg_console_fun = _fun; }

void ConsoleLog(const XLoggerInfo* _info, const char* _log) {
    SCOPE_POOL();

    if (NULL == _info || NULL == _log) return;

    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"   // fatal
    };

    const char* strFuncName = NULL == _info->func_name ? "" : _info->func_name;
    const char* file_name = ExtractFileName(_info->filename);

    if (kConsoleOSLog == sg_console_fun) {
        os_log_t log_t = os_log_create("", NULL == _info->tag ? "" : _info->tag);
        os_log_type_t type = OS_LOG_TYPE_DEFAULT;
        switch (_info->level) {
            case kLevelVerbose:
            case kLevelDebug:
                type = OS_LOG_TYPE_DEBUG;
                break;
            case kLevelInfo:
            case kLevelWarn:
                type = OS_LOG_TYPE_INFO;
                break;
            case kLevelError:
                type = OS_LOG_TYPE_ERROR;
                break;
            case kLevelFatal:
                type = OS_LOG_TYPE_FAULT;
                break;
            default:
                break;
        }
        os_log_with_type(log_t, type, "[%s:%d, %s][%s", file_name, _info->line, strFuncName, _log);
    } else if (kConsoleNSLog == sg_console_fun) {
        NSLog(@"[%s][%s][%s:%d, %s][%s", levelStrings[_info->level],
              NULL == _info->tag ? "" : _info->tag, file_name, _info->line, strFuncName, _log);
    } else {
        char log[16 * 1024] = {0};
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        time_t sec = _info->timeval.tv_sec;
        tm tm = *localtime((const time_t*)&sec);
        char temp_time[64] = {0};
        snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d",
                 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_gmtoff / 3600.0, tm.tm_hour,
                 tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);

        snprintf(log, sizeof(log),
                 "[%s][%s][%" PRIdMAX ", %" PRIdMAX "%s][%s][%s:%d, %s][%s",  // **CPPLINT SKIP**
                 levelStrings[_info->level], temp_time, _info->pid, _info->tid,
                 _info->tid == _info->maintid ? "*" : "", _info->tag ? _info->tag : "", file_name,
                 _info->line, strFuncName, _log);
        printf("%s\n", log);
    }
}

}  // xlog
}  // mars
