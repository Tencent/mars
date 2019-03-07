// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * @Author: garryyan
 * @LastEditors: garryyan
 * @Date: 2019-03-05 15:10:24
 * @LastEditTime: 2019-03-05 15:10:50
 */

#ifndef MARS_LOG_XLOGGER_INTERFACE_H_
#define MARS_LOG_XLOGGER_INTERFACE_H_

#include <stdint.h>

#include "appender.h"
#include "mars/comm/xlogger/xloggerbase.h"

namespace mars {
namespace comm {

class XloggerCategory;

}
}

namespace mars {
namespace xlog {


mars::comm::XloggerCategory* NewXloggerInstance(TLogLevel _level, TAppenderMode _mode,
                                                    const char* _cachedir, const char* _logdir,
                                                    const char* _nameprefix, int _cache_days,
                                                    const char* _pub_key);


mars::comm::XloggerCategory* GetXloggerInstance(const char* _nameprefix);

void ReleaseXloggerInstance(const char* _nameprefix);

void XloggerWrite(int64_t _instance_ptr, const XLoggerInfo* _info, const char* _log);

bool IsEnabledFor(int64_t _instance_ptr, TLogLevel _level);

TLogLevel GetLevel(int64_t _instance_ptr);

void SetLevel(int64_t _instance_ptr, TLogLevel _level);

void SetAppenderMode(int64_t _instance_ptr, TAppenderMode _mode);

void Flush(int64_t _instance_ptr, bool _is_sync);

void SetConsoleLogOpen(int64_t _instance_ptr, bool _is_open);

void SetMaxFileSize(int64_t _instance_ptr, long _max_file_size);

void SetMaxAliveTime(int64_t _instance_ptr, long _alive_seconds);

}
}


#endif  // MARS_LOG_XLOGGER_INTERFACE_H_