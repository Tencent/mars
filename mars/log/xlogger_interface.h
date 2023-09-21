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
#include "xloggerbase.h"

namespace mars {
namespace comm {

class XloggerCategory;

}
}  // namespace mars

namespace mars {
namespace xlog {

mars::comm::XloggerCategory* NewXloggerInstance(const XLogConfig& _config, TLogLevel _level);

mars::comm::XloggerCategory* GetXloggerInstance(const char* _nameprefix);

void ReleaseXloggerInstance(const char* _nameprefix);

void XloggerWrite(uintptr_t _instance_ptr, const XLoggerInfo* _info, const char* _log);

bool IsEnabledFor(uintptr_t _instance_ptr, TLogLevel _level);

TLogLevel GetLevel(uintptr_t _instance_ptr);

void SetLevel(uintptr_t _instance_ptr, TLogLevel _level);

void SetAppenderMode(uintptr_t _instance_ptr, TAppenderMode _mode);

void Flush(uintptr_t _instance_ptr, bool _is_sync);

void FlushAll(bool _is_sync);

void SetConsoleLogOpen(uintptr_t _instance_ptr, bool _is_open);

void SetMaxFileSize(uintptr_t _instance_ptr, long _max_file_size);

void SetMaxAliveTime(uintptr_t _instance_ptr, long _alive_seconds);

}  // namespace xlog
}  // namespace mars

#endif  // MARS_LOG_XLOGGER_INTERFACE_H_