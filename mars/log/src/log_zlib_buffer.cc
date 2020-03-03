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

#include "log_zlib_buffer.h"
#include "log_base_buffer.h"

#include <cstdio>
#include <time.h>
#include <algorithm>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "log/crypt/log_crypt.h"



#ifdef WIN32
#define snprintf _snprintf
#endif


LogZlibBuffer::LogZlibBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey)
    :LogBaseBuffer(_pbuffer, _len, _isCompress, _pubkey){


    if (is_compress_) {
        memset(&cstream_, 0, sizeof(cstream_));
    }
}

LogZlibBuffer::~LogZlibBuffer() {
    if (is_compress_ && Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }
    
    delete log_crypt_;
}

PtrBuffer& LogZlibBuffer::GetData() {
    return buff_;
}


void LogZlibBuffer::Flush(AutoBuffer& _buff) {
    if (is_compress_ && Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }

    if (log_crypt_->GetLogLen((char*)buff_.Ptr(), buff_.Length()) == 0){
        LogBaseBuffer::__Clear();
        return;
    }
    __Flush();
    _buff.Write(buff_.Ptr(), buff_.Length());
    LogBaseBuffer::__Clear();
}

bool LogZlibBuffer::Write(const void* _data, size_t _length) {
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

    before_len -= remain_nocrypt_len_;

    AutoBuffer out_buffer;
    size_t last_remain_len = remain_nocrypt_len_;

    log_crypt_->CryptAsyncLog((char*)buff_.Ptr() + before_len, write_len + remain_nocrypt_len_, out_buffer, remain_nocrypt_len_);

    buff_.Write(out_buffer.Ptr(), out_buffer.Length(), before_len);

    before_len += out_buffer.Length();
    buff_.Length(before_len, before_len);

    log_crypt_->UpdateLogLen((char*)buff_.Ptr(), (uint32_t)(out_buffer.Length() - last_remain_len));

    return true;
}

bool LogZlibBuffer::__Reset() {
    
    LogBaseBuffer::__Reset();

    if (is_compress_) {
        cstream_.zalloc = Z_NULL;
        cstream_.zfree = Z_NULL;
        cstream_.opaque = Z_NULL;

        if (Z_OK != deflateInit2(&cstream_, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
            return false;
        }

    }

    return true;
}

