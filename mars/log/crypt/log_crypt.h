// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  log_crypt.h
//  mars-ext
//
//  Created by yanguoyue on 16/6/14.
//

#ifndef LOG_CRYPT_H_
#define LOG_CRYPT_H_

#include <stdint.h>

#include <string>

#include "mars/comm/autobuffer.h"

namespace mars {
namespace xlog {

class LogCrypt {
 public:
    LogCrypt(const char* _pubkey);
    virtual ~LogCrypt() {
    }

 private:
    LogCrypt(const LogCrypt&);
    LogCrypt& operator=(const LogCrypt&);

 public:
    static uint32_t GetHeaderLen();
    static uint32_t GetTailerLen();

    static bool GetLogHour(const char* const _data, size_t _len, int& _begin_hour, int& _end_hour);
    static void UpdateLogHour(char* _data);

    static uint32_t GetLogLen(const char* const _data, size_t _len);
    static void UpdateLogLen(char* _data, uint32_t _add_len);

 public:
    void SetHeaderInfo(char* _data, bool _is_async, char _magic_start);
    void SetTailerInfo(char* _data, char _magic_end);

    void CryptSyncLog(const char* const _log_data,
                      size_t _input_len,
                      AutoBuffer& _out_buff,
                      char _magic_start,
                      char _magic_end);
    void CryptAsyncLog(const char* const _log_data,
                       size_t _input_len,
                       std::string& _out_buff,
                       size_t& _remain_nocrypt_len);

    bool Fix(char* _data, size_t _data_len, uint32_t& _raw_log_len);
    bool IsCrypt();

 private:
    uint16_t seq_;
    uint32_t tea_key_[4];
    char client_pubkey_[64];
    bool is_crypt_;
};

}  // namespace xlog
}  // namespace mars

#endif /* LOG_CRYPT_H_ */
