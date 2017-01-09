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
 * log_buffer.cpp
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#include "log_buffer.h"

#include <cstdio>
#include <time.h>
#include <algorithm>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "log/crypt/log_crypt.h"


#ifdef WIN32
#define snprintf _snprintf
#endif


LogCrypt* LogBuffer::s_log_crypt =  new LogCrypt();

bool LogBuffer::GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg) {
    return s_log_crypt->GetPeriodLogs(_log_path, _begin_hour, _end_hour, _begin_pos, _end_pos, _err_msg);
}


bool LogBuffer::Write(const void* _data, size_t _inputlen, void* _output, size_t& _len) {
    if (NULL == _data || NULL == _output || 0 == _inputlen || _len <= (size_t) s_log_crypt->GetHeaderLen()) {
        return false;
    }
    
    s_log_crypt->CryptSyncLog((char*)_data, _inputlen, (char*)_output, _len);
    
    return true;
}

LogBuffer::LogBuffer(void* _pbuffer, size_t _len, bool _isCompress)
: is_compress_(_isCompress) {
    buff_.Attach(_pbuffer, _len);
    __Fix();

    memset(&cstream_, 0, sizeof(cstream_));
}

LogBuffer::~LogBuffer() {
    if (Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }
}

PtrBuffer& LogBuffer::GetData() {
    return buff_;
}


void LogBuffer::Flush(AutoBuffer& _buff) {
    
    if (Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }

    if (s_log_crypt->GetLogLen((char*)buff_.Ptr(), buff_.Length()) == 0){
        __Clear();
        return;
    }
    
    __Flush();
    _buff.Write(buff_.Ptr(), buff_.Length());
    __Clear();
}


bool LogBuffer::Write(const void* _data, size_t _length) {
    if (NULL == _data || 0 == _length) {
        return false;
    }
    
    if (buff_.Length() == 0) {
        if (!__Reset()) return false;
    }

    size_t before_len = buff_.Length();
    size_t write_len = _length;
    
    if (is_compress_) {
        cstream_.avail_in = (uInt)_length;
        cstream_.next_in = (Bytef*)_data;

        uInt avail_out = (uInt)(buff_.MaxLength() - buff_.Length());
        cstream_.next_out = (Bytef*)buff_.PosPtr();
        cstream_.avail_out = avail_out;

        if (Z_OK != deflate(&cstream_, Z_SYNC_FLUSH)) {
            return false;
        }

        write_len = avail_out - cstream_.avail_out;
    } else {
        buff_.Write(_data, _length);
    }
    

    char crypt_buffer[4096] = {0};
    size_t crypt_buffer_len = sizeof(crypt_buffer);
    
    
    s_log_crypt->CryptAsyncLog((char*)buff_.Ptr() + before_len, write_len, crypt_buffer, crypt_buffer_len);
    
    uint16_t single_log_len = crypt_buffer_len;
    buff_.Write(&single_log_len, sizeof(single_log_len), before_len);
    
    before_len += sizeof(single_log_len);
    buff_.Write(crypt_buffer, crypt_buffer_len, before_len);
    
    before_len += crypt_buffer_len;
    buff_.Length(before_len, before_len);
   
    s_log_crypt->UpdateLogLen((char*)buff_.Ptr(), (uint32_t)crypt_buffer_len + sizeof(single_log_len));

    return true;
}

bool LogBuffer::__Reset() {
    
    __Clear();
    
    if (is_compress_) {
        cstream_.zalloc = Z_NULL;
        cstream_.zfree = Z_NULL;
        cstream_.opaque = Z_NULL;
        
        if (Z_OK != deflateInit2(&cstream_, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
            return false;
        }
        
    }
    
    s_log_crypt->SetHeaderInfo((char*)buff_.Ptr(), is_compress_);
    buff_.Length(s_log_crypt->GetHeaderLen(), s_log_crypt->GetHeaderLen());

    return true;
}

void LogBuffer::__Flush() {
    assert(buff_.Length() >= s_log_crypt->GetHeaderLen());
    
    s_log_crypt->UpdateLogHour((char*)buff_.Ptr());
    s_log_crypt->SetTailerInfo((char*)buff_.Ptr() + buff_.Length());
    buff_.Length(buff_.Length() + s_log_crypt->GetTailerLen(), buff_.Length() + s_log_crypt->GetTailerLen());

}

void LogBuffer::__Clear() {
    memset(buff_.Ptr(), 0, buff_.MaxLength());
    buff_.Length(0, 0);
}


void LogBuffer::__Fix() {
    uint32_t raw_log_len = 0;
    if (s_log_crypt->Fix((char*)buff_.Ptr(), buff_.Length(), is_compress_, raw_log_len)) {
        buff_.Length(raw_log_len + s_log_crypt->GetHeaderLen(), raw_log_len + s_log_crypt->GetHeaderLen());
    } else {
        buff_.Length(0, 0);
    }

}

