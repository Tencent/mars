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
 * appender.h
 *
 *  Created on: 2013-3-7
 *      Author: yerungui
 */

#ifndef APPENDER_H_
#define APPENDER_H_

#include <string>
#include <vector>
#include <stdint.h>

enum TAppenderMode
{
    kAppednerAsync,
    kAppednerSync,
};

enum TCompressMode{
    kZlib,
    kZstd,
};

struct XLogConfig{
    TAppenderMode mode_ = kAppednerAsync;
    std::string logdir_;
    std::string nameprefix_;
    std::string pub_key_;
    TCompressMode compress_mode_ = kZlib;
    int compress_level_ = 6;
    std::string cachedir_;
    int cache_days_ = 0;
};

void appender_open(const XLogConfig& _config);

void appender_flush();
void appender_flush_sync();
void appender_close();
void appender_setmode(TAppenderMode _mode);
bool appender_getfilepath_from_timespan(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);
bool appender_make_logfile_name(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);
bool appender_get_current_log_path(char* _log_path, unsigned int _len);
bool appender_get_current_log_cache_path(char* _logPath, unsigned int _len);
void appender_set_console_log(bool _is_open);

/*
 * By default, all logs will write to one file everyday. You can split logs to multi-file by changing max_file_size.
 * 
 * @param _max_byte_size    Max byte size of single log file, default is 0, meaning do not split.
 */
void appender_set_max_file_size(uint64_t _max_byte_size);

/*
 * By default, all logs lives 10 days at most.
 *
 * @param _max_time    Max alive duration of a single log file in seconds, default is 10 days
 */
void appender_set_max_alive_duration(long _max_time);

#endif /* APPENDER_H_ */
