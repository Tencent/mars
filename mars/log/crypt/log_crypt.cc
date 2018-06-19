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
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <algorithm>
#endif // WIN32

#ifndef XLOG_NO_CRYPT
#include "micro-ecc-master/uECC.h"
#endif

static const char kMagicSyncStart = '\x06';
static const char kMagicSyncNoCryptStart ='\x08';
static const char kMagicAsyncStart ='\x07';
static const char kMagicAsyncNoCryptStart ='\x09';

static const char kMagicEnd  = '\0';

const static int TEA_BLOCK_LEN = 8;

static void __TeaEncrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0, i;
    const static uint32_t delta=0x9e3779b9;
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];
    for (i=0; i < 16; i++) {
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }
    v[0]=v0; v[1]=v1;
}

static uint16_t __GetSeq(bool _is_async) {
    
    if (!_is_async) {
        return 0;
    }
    
    static uint16_t s_seq = 0;
    
    s_seq ++;
    
    if (0 == s_seq) {
        s_seq ++;
    }
    
    return s_seq;
}

#ifndef XLOG_NO_CRYPT
static bool Hex2Buffer(const char* _str, size_t _len, unsigned char* _buffer) {
    
    if (NULL == _str || _len ==0 || _len % 2 != 0) {
        return -1;
    }
    
    char tmp[3] = {0};
    for (size_t i=0; i<_len-1; i+=2) {
        for (size_t j=0; j<2; ++j) {
            tmp[j] = _str[i+j];
            if (!(('0' <= tmp[j] && tmp[j] <= '9')
                  || ('a' <= tmp[j] && tmp[j] <= 'f')
                  || ('A' <= tmp[j] && tmp[j] <= 'F'))) {
                
                return false;
            }
        }
        
        _buffer[i/2] = (unsigned char)strtol(tmp, NULL, 16);
        
    }
    return true;
}
#endif

LogCrypt::LogCrypt(const char* _pubkey): seq_(0), is_crypt_(false) {
    
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
    return sizeof(kMagicEnd);
}

bool LogCrypt::GetLogHour(const char* const _data, size_t _len, int& _begin_hour, int& _end_hour) {
    
    if (_len < GetHeaderLen()) return false;
    
    char start = _data[0];
    if (kMagicAsyncStart != start && kMagicSyncStart != start) return false;
    
    char begin_hour = _data[sizeof(char)+sizeof(uint16_t)];
    char end_hour = _data[sizeof(char)+sizeof(uint16_t)+sizeof(char)];
    
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

uint32_t LogCrypt::GetLogLen(const char*  const _data, size_t _len) {
    if (_len < GetHeaderLen()) return 0;
    
    char start = _data[0];
    if (kMagicAsyncStart != start && kMagicSyncStart != start
        && kMagicAsyncNoCryptStart != start && kMagicSyncNoCryptStart != start) {
        return 0;
    }
    
    uint32_t len = 0;
    memcpy(&len, _data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char)*64, sizeof(len));
    return len;
}

void LogCrypt::UpdateLogLen(char* _data, uint32_t _add_len) {
    
    uint32_t currentlen = (uint32_t)(GetLogLen(_data, GetHeaderLen()) + _add_len);
    memcpy(_data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char) * 64, &currentlen, sizeof(currentlen));
}

void LogCrypt::SetHeaderInfo(char* _data, bool _is_async) {
    if (_is_async) {
        if (is_crypt_) {
            memcpy(_data, &kMagicAsyncStart, sizeof(kMagicAsyncStart));
        } else {
            memcpy(_data, &kMagicAsyncNoCryptStart, sizeof(kMagicAsyncNoCryptStart));
        }
    } else {
        if (is_crypt_) {
            memcpy(_data, &kMagicSyncStart, sizeof(kMagicSyncStart));
        } else {
            memcpy(_data, &kMagicSyncNoCryptStart, sizeof(kMagicSyncNoCryptStart));
        }
    }
    
    seq_ = __GetSeq(_is_async);
    memcpy(_data + sizeof(kMagicAsyncStart), &seq_, sizeof(seq_));

    
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t sec = tv.tv_sec;
    tm tm_tmp = *localtime((const time_t*)&sec);
    
    char hour = (char)tm_tmp.tm_hour;
    memcpy(_data + sizeof(kMagicAsyncStart) + sizeof(seq_), &hour, sizeof(hour));
    memcpy(_data + sizeof(kMagicAsyncStart) + sizeof(seq_) + sizeof(hour), &hour, sizeof(hour));

    
    uint32_t len = 0;
    memcpy(_data + sizeof(kMagicAsyncStart) + sizeof(seq_) + sizeof(hour) * 2, &len, sizeof(len));
    memcpy(_data + sizeof(kMagicAsyncStart) + sizeof(seq_) + sizeof(hour) * 2 + sizeof(len), client_pubkey_, sizeof(client_pubkey_));
}

void LogCrypt::SetTailerInfo(char* _data) {
    memcpy(_data, &kMagicEnd, sizeof(kMagicEnd));
}

bool LogCrypt::GetPeriodLogs(const char* const _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg) {
    
    char msg[1024] = {0};
    
    
    if (NULL == _log_path || _end_hour <= _begin_hour) {
        snprintf(msg, sizeof(msg), "NULL == _logPath || _endHour <= _beginHour, %d, %d", _begin_hour, _end_hour);
        return false;
    }
    
    FILE* file = fopen(_log_path, "rb");
    if (NULL == file) {
        snprintf(msg, sizeof(msg), "open file fail:%s", strerror(errno));
        _err_msg += msg;
        return false;
    }
    
    if (0 != fseek(file, 0, SEEK_END)) {
        snprintf(msg, sizeof(msg), "fseek(file, 0, SEEK_END):%s", strerror(ferror(file)));
        _err_msg += msg;
        fclose(file);
        return false;
    }
    
    long file_size = ftell(file);
    
    if (0 != fseek(file, 0, SEEK_SET)) {
        snprintf(msg, sizeof(msg), "fseek(file, 0, SEEK_SET) error:%s", strerror(ferror(file)));
        _err_msg += msg;
        fclose(file);
        return false;
    }
    
    _begin_pos = _end_pos = 0;
    
    bool find_begin_pos = false;
    int last_end_hour = -1;
    unsigned long last_end_pos = 0;
    
    char* header_buff = new char[GetHeaderLen()];
    
    while (!feof(file) && !ferror(file)) {
        
        if ((long)(ftell(file) + GetHeaderLen() + GetTailerLen()) > file_size) {
            snprintf(msg, sizeof(msg), "ftell(file) + __GetHeaderLen() + sizeof(kMagicEnd)) > file_size error");
            break;
        }
        
        long before_len = ftell(file);
        if (GetHeaderLen() != fread(header_buff, 1, GetHeaderLen(), file)) {
            snprintf(msg, sizeof(msg), "fread(buff.Ptr(), 1, __GetHeaderLen(), file) error:%s, before_len:%ld.", strerror(ferror(file)), before_len);
            break;
        }
        
        
        bool fix = false;
        
        char start = *header_buff;
        if (start != kMagicSyncStart && start != kMagicSyncNoCryptStart
                && start != kMagicAsyncStart && start != kMagicAsyncNoCryptStart) {
            fix = true;
        } else {
            uint32_t len = GetLogLen(header_buff, GetHeaderLen());
            if ((long)(ftell(file) + len + sizeof(kMagicEnd)) > file_size) {
                fix = true;
            } else {
                if (0 != fseek(file, len, SEEK_CUR)) {
                    snprintf(msg, sizeof(msg), "fseek(file, len, SEEK_CUR):%s, before_len:%ld, len:%u.", strerror(ferror(file)), before_len, len);
                    break;
                }
                char end;
                if (1 != fread(&end, 1, 1, file)) {
                    snprintf(msg, sizeof(msg), "fread(&end, 1, 1, file) err:%s, before_len:%ld, len:%u.", strerror(ferror(file)), before_len, len);
                    break;
                }
                if (end != kMagicEnd) {
                    fix = true;
                }
            }
        }
        
        if (fix) {
            if (0 !=fseek(file, before_len+1, SEEK_SET)) {
                snprintf(msg, sizeof(msg), "fseek(file, before_len+1, SEEK_SET) err:%s, before_len:%ld.", strerror(ferror(file)), before_len);
                break;
            }
            continue;
        }
        
        
        int begin_hour = 0;
        int end_hour = 0;
        if (!GetLogHour(header_buff, GetHeaderLen(), begin_hour, end_hour)) {
            snprintf(msg, sizeof(msg), "__GetLogHour(buff.Ptr(), buff.Length(), beginHour, endHour) err, before_len:%ld.", before_len);
            break;
        }
        
        if (begin_hour > end_hour)  begin_hour = end_hour;
        
        
        if (!find_begin_pos) {
            if (_begin_hour > begin_hour && _begin_hour <= end_hour) {
                _begin_pos = before_len;
                find_begin_pos = true;
            }
            
            if (_begin_hour > last_end_hour && _begin_hour <= begin_hour) {
                _begin_pos = before_len;
                find_begin_pos = true;
            }
        }
        
        if (find_begin_pos) {
            if (_end_hour > begin_hour && _end_hour <= end_hour) {
                _end_pos = ftell(file);
            }
            
            if (_end_hour > last_end_hour && _end_hour <= begin_hour) {
                _end_pos = last_end_pos;
            }
        }
        
        last_end_hour = end_hour;
        last_end_pos = ftell(file);
    }
    
    delete[] header_buff;
    
    if (find_begin_pos && _end_hour > last_end_hour) {
        _end_pos = file_size;
    }
    
    fclose(file);
    
    bool ret = _end_pos > _begin_pos;
    
    if (!ret) {
        _err_msg += msg;
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "begintpos:%lu, endpos:%lu, filesize:%ld.", _begin_pos, _end_pos, file_size);
        _err_msg += msg;
    }
    
    return ret;
}


void LogCrypt::CryptSyncLog(const char* const _log_data, size_t _input_len, AutoBuffer& _out_buff) {
	_out_buff.AllocWrite(GetHeaderLen() + GetTailerLen() + _input_len);

    SetHeaderInfo((char*)_out_buff.Ptr(), false);
    
    uint32_t header_len = GetHeaderLen();
    
    UpdateLogLen((char*)_out_buff.Ptr(), (uint32_t)_input_len);
    
    SetTailerInfo((char*)_out_buff.Ptr() + _input_len + header_len);
    
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

    memcpy((char*)_out_buff.Ptr() + header_len + _input_len - remain_len, _log_data + _input_len - remain_len, remain_len);*/
#endif

}

void LogCrypt::CryptAsyncLog(const char* const _log_data, size_t _input_len, AutoBuffer& _out_buff, size_t& _remain_nocrypt_len) {
    
	_out_buff.AllocWrite(_input_len);
    
    if (!is_crypt_) {
        memcpy(_out_buff.Ptr(), _log_data, _input_len);
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
        memcpy((char*)_out_buff.Ptr() + i * TEA_BLOCK_LEN, tmp, TEA_BLOCK_LEN);
    }
    
    memcpy((char*)_out_buff.Ptr() + _input_len - _remain_nocrypt_len, _log_data + _input_len - _remain_nocrypt_len, _remain_nocrypt_len);
#endif
}

bool LogCrypt::Fix(char* _data, size_t _data_len, bool& _is_async, uint32_t& _raw_log_len) {
    if (_data_len < GetHeaderLen()) {
        return false;
    }
    
    char start = _data[0];
    if (kMagicAsyncStart != start && kMagicSyncStart != start
        && kMagicAsyncNoCryptStart != start && kMagicSyncNoCryptStart != start) {
        return false;
    }
    
    if (kMagicSyncStart == start || kMagicSyncNoCryptStart) {
        _is_async = false;
    } else {
        _is_async = true;
    }
    
    _raw_log_len = GetLogLen(_data, _data_len);
    
    memcpy(&seq_, _data + 1, sizeof(seq_));
    return true;

}
