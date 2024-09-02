/*
 * Tencent is pleased to support the open source community by making owl available.
 * Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the MIT License (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 * http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

#include <hilog/log.h>

#include <algorithm>
#include <cstdio>

#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/xlog/appender.h"

namespace mars {
namespace xlog {

static const char* kHiLogFormat = "#%{public}llu %{public}s %{public}d: %{public}s\n";
static const char* kHiLogFormatTs = "%{public}s\n";
static const unsigned int kHiLogDomain = 0x0121;  // wechat birthday ^_^

#if 0
typedef enum {
    /** Debug level to be used by {@link OH_LOG_DEBUG} */
    LOG_DEBUG = 3,
    /** Informational level to be used by {@link OH_LOG_INFO} */
    LOG_INFO = 4,
    /** Warning level to be used by {@link OH_LOG_WARN} */
    LOG_WARN = 5,
    /** Error level to be used by {@link OH_LOG_ERROR} */
    LOG_ERROR = 6,
    /** Fatal level to be used by {@link OH_LOG_FATAL} */
    LOG_FATAL = 7,
} LogLevel;
#endif

// 这里不能加日志，会导致循环调用
void ConsoleLog(const XLoggerInfo* _info, const char* _log) {
    // NOTE: OH_LOG do not have verbose and none level
    int level = (_info->level == TLogLevel::kLevelVerbose ? LOG_DEBUG : _info->level + 2);
    if (!_info->filename || _info->filename[0] == '\0' || _info->line == 0) {
        OH_LOG_Print(LOG_APP, (LogLevel)level, kHiLogDomain, _info->tag, kHiLogFormatTs, _log);
    } else {
        OH_LOG_Print(LOG_APP,
                     (LogLevel)level,
                     kHiLogDomain,
                     _info->tag,
                     kHiLogFormat,
                     0,
                     _info->filename,
                     _info->line,
                     _log);
    }
}

}  // namespace xlog
}  // namespace mars
