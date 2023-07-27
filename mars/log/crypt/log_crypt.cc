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
//  log_crypt.cc
//  mars-ext
//
//  Created by garry on 16/6/14.
//

#include "log_crypt.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "log_magic_num.h"

#ifdef WIN32
#include <algorithm>
#endif  // WIN32

#ifndef XLOG_NO_CRYPT
#include "micro-ecc-master/uECC.h"
#endif

namespace mars {
namespace xlog {

const static int TEA_BLOCK_LEN = 8;

static void __TeaEncrypt(uint32_t* v, uint32_t* k) {
    uint32_t v0 = v[0], v1 = v[1], sum = 0, i;
    const static uint32_t delta = 0x9e3779b9;
    uint32_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3];
    for (i = 0; i < 16; i++) {
        sum += delta;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }
    v[0] = v0;
    v[1] = v1;
}

static uint16_t __GetSeq(bool _is_async) {
    if (!_is_async) {
        return 0;
    }

    static uint16_t s_seq = 0;

    s_seq++;

    if (0 == s_seq) {
        s_seq++;
    }

    return s_seq;
}

#ifndef XLOG_NO_CRYPT
static bool Hex2Buffer(const char* _str, size_t _len, unsigned char* _buffer) {
    if (NULL == _str || _len == 0 || _len % 2 != 0) {
        return -1;
    }

    char tmp[3] = {0};
    for (size_t i = 0; i < _len - 1; i += 2) {
        for (size_t j = 0; j < 2; ++j) {
            tmp[j] = _str[i + j];
            if (!(('0' <= tmp[j] && tmp[j] <= '9') || ('a' <= tmp[j] && tmp[j] <= 'f')
                  || ('A' <= tmp[j] && tmp[j] <= 'F'))) {
                return false;
            }
        }

        _buffer[i / 2] = (unsigned char)strtol(tmp, NULL, 16);
    }
    return true;
}
#endif

LogCrypt::LogCrypt(const char* _pubkey) : seq_(0), is_crypt_(false) {
#ifndef XLOG_NO_CRYPT
    const static size_t PUB_KEY_LEN = 64;

    if (NULL == _pubkey || PUB_KEY_LEN * 2 != strnlen(_pubkey, 256)) {
        return;
    }

    unsigned char svr_pubkey[PUB_KEY_LEN] = {0};

    if (!Hex2Buffer(_pubkey, PUB_KEY_LEN * 2, svr_pubkey)) {
        return;
    }

    uint8_t client_pri[32] = {0};
    if (0 == uECC_make_key((uint8_t*)client_pubkey_, client_pri, uECC_secp256k1())) {
        return;
    }

    uint8_t ecdh_key[32] = {0};
    if (0 == uECC_shared_secret(svr_pubkey, client_pri, ecdh_key, uECC_secp256k1())) {
        return;
    }

    memcpy(tea_key_, ecdh_key, sizeof(tea_key_));

    is_crypt_ = true;

#endif
}

/*
 * |magic start(char)|seq(uint16_t)|begin hour(char)|end hour(char)|length(uint32_t)|crypt key(char*64)|
 */

uint32_t LogCrypt::GetHeaderLen() {
    return sizeof(char) * 3 + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(char) * 64;
}

uint32_t LogCrypt::GetTailerLen() {
    return sizeof(char);
}

bool LogCrypt::GetLogHour(const char* const _data, size_t _len, int& _begin_hour, int& _end_hour) {
    if (_len < GetHeaderLen())
        return false;

    char start = _data[0];
    if (!LogMagicNum::MagicStartIsValid(start))
        return false;

    char begin_hour = _data[sizeof(char) + sizeof(uint16_t)];
    char end_hour = _data[sizeof(char) + sizeof(uint16_t) + sizeof(char)];

    _begin_hour = (int)begin_hour;
    _end_hour = (int)end_hour;
    return true;
}

void LogCrypt::UpdateLogHour(char* _data) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t sec = tv.tv_sec;
    struct tm tm_tmp = *localtime((const time_t*)&sec);

    char hour = (char)tm_tmp.tm_hour;
    memcpy(_data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char) * 64 - sizeof(char), &hour, sizeof(hour));
}

uint32_t LogCrypt::GetLogLen(const char* const _data, size_t _len) {
    if (_len < GetHeaderLen())
        return 0;

    char start = _data[0];
    if (!LogMagicNum::MagicStartIsValid(start)) {
        return 0;
    }

    uint32_t len = 0;
    memcpy(&len, _data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char) * 64, sizeof(len));
    return len;
}

void LogCrypt::UpdateLogLen(char* _data, uint32_t _add_len) {
    uint32_t currentlen = (uint32_t)(GetLogLen(_data, GetHeaderLen()) + _add_len);
    memcpy(_data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char) * 64, &currentlen, sizeof(currentlen));
}

void LogCrypt::SetHeaderInfo(char* _data, bool _is_async, char _magic_start) {
    memcpy(_data, &_magic_start, sizeof(_magic_start));
    seq_ = __GetSeq(_is_async);
    memcpy(_data + sizeof(_magic_start), &seq_, sizeof(seq_));

    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t sec = tv.tv_sec;
    tm tm_tmp = *localtime((const time_t*)&sec);

    char hour = (char)tm_tmp.tm_hour;
    memcpy(_data + sizeof(_magic_start) + sizeof(seq_), &hour, sizeof(hour));
    memcpy(_data + sizeof(_magic_start) + sizeof(seq_) + sizeof(hour), &hour, sizeof(hour));

    uint32_t len = 0;
    memcpy(_data + sizeof(_magic_start) + sizeof(seq_) + sizeof(hour) * 2, &len, sizeof(len));
    memcpy(_data + sizeof(_magic_start) + sizeof(seq_) + sizeof(hour) * 2 + sizeof(len),
           client_pubkey_,
           sizeof(client_pubkey_));
}

void LogCrypt::SetTailerInfo(char* _data, char _magic_end) {
    memcpy(_data, &_magic_end, sizeof(_magic_end));
}

void LogCrypt::CryptSyncLog(const char* const _log_data,
                            size_t _input_len,
                            AutoBuffer& _out_buff,
                            char _magic_start,
                            char _magic_end) {
    _out_buff.AllocWrite(GetHeaderLen() + GetTailerLen() + _input_len);

    SetHeaderInfo((char*)_out_buff.Ptr(), false, _magic_start);

    uint32_t header_len = GetHeaderLen();

    UpdateLogLen((char*)_out_buff.Ptr(), (uint32_t)_input_len);

    SetTailerInfo((char*)_out_buff.Ptr() + _input_len + header_len, _magic_end);

    //  if (!is_crypt_) {
    memcpy((char*)_out_buff.Ptr() + header_len, _log_data, _input_len);
    //    return;
    //}

#ifndef XLOG_NO_CRYPT
    /*   uint32_t tmp[2] = {0};
       size_t cnt = _input_len / TEA_BLOCK_LEN;
       size_t remain_len = _input_len % TEA_BLOCK_LEN;

       for (size_t i = 0; i < cnt; ++i) {
           memcpy(tmp, _log_data + i * TEA_BLOCK_LEN, TEA_BLOCK_LEN);
           __TeaEncrypt(tmp, tea_key_);
           memcpy((char*)_out_buff.Ptr() + header_len + i * TEA_BLOCK_LEN, tmp, TEA_BLOCK_LEN);
       }

       memcpy((char*)_out_buff.Ptr() + header_len + _input_len - remain_len, _log_data + _input_len - remain_len,
       remain_len);*/
#endif
}

void LogCrypt::CryptAsyncLog(const char* const _log_data,
                             size_t _input_len,
                             std::string& _out_buff,
                             size_t& _remain_nocrypt_len) {
    if (!is_crypt_) {
        _out_buff.assign(_log_data, _input_len);
        _remain_nocrypt_len = 0;
        return;
    }
#ifndef XLOG_NO_CRYPT
    uint32_t tmp[2] = {0};
    size_t cnt = _input_len / TEA_BLOCK_LEN;
    _remain_nocrypt_len = _input_len % TEA_BLOCK_LEN;

    for (size_t i = 0; i < cnt; ++i) {
        memcpy(tmp, _log_data + i * TEA_BLOCK_LEN, TEA_BLOCK_LEN);
        __TeaEncrypt(tmp, tea_key_);
        _out_buff.append((char*)tmp, TEA_BLOCK_LEN);
    }
    _out_buff.append(_log_data + _input_len - _remain_nocrypt_len, _remain_nocrypt_len);
#endif
}

bool LogCrypt::Fix(char* _data, size_t _data_len, uint32_t& _raw_log_len) {
    if (_data_len < GetHeaderLen()) {
        return false;
    }

    char start = _data[0];
    if (!LogMagicNum::MagicStartIsValid(start)) {
        return false;
    }

    _raw_log_len = GetLogLen(_data, _data_len);

    memcpy(&seq_, _data + 1, sizeof(seq_));
    return true;
}

bool LogCrypt::IsCrypt() {
    return is_crypt_;
}

}  // namespace xlog
}  // namespace mars
