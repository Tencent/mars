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
 * log_zlib_buffer.cpp
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#include "log_zlib_buffer.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <algorithm>
#include <cstdio>

#include "xlog/crypt/log_crypt.h"
#include "xlog/crypt/log_magic_num.h"
#include "log_base_buffer.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace mars {
namespace xlog {

LogZlibBuffer::LogZlibBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey)
: LogBaseBuffer(_pbuffer, _len, _isCompress, _pubkey) {
    if (is_compress_) {
        memset(&cstream_, 0, sizeof(cstream_));
    }
}

LogZlibBuffer::~LogZlibBuffer() {
    if (is_compress_ && Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }
}

void LogZlibBuffer::Flush(AutoBuffer& _buff) {
    if (is_compress_ && Z_NULL != cstream_.state) {
        deflateEnd(&cstream_);
    }

    LogBaseBuffer::Flush(_buff);
}

size_t LogZlibBuffer::Compress(const void* src, size_t inLen, void* dst, size_t outLen) {
    cstream_.avail_in = (uInt)inLen;
    cstream_.next_in = (Bytef*)src;

    cstream_.next_out = (Bytef*)dst;
    cstream_.avail_out = (uInt)outLen;

    if (Z_OK != deflate(&cstream_, Z_SYNC_FLUSH)) {
        return -1;
    }

    return outLen - cstream_.avail_out;
}

bool LogZlibBuffer::__Reset() {
    if (!LogBaseBuffer::__Reset()) {
        return false;
    }

    if (is_compress_) {
        cstream_.zalloc = Z_NULL;
        cstream_.zfree = Z_NULL;
        cstream_.opaque = Z_NULL;

        if (Z_OK
            != deflateInit2(&cstream_, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
            return false;
        }
    }

    return true;
}

char LogZlibBuffer::__GetMagicSyncStart() {
    return is_crypt_ ? LogMagicNum::kMagicSyncZlibStart : LogMagicNum::kMagicSyncNoCryptZlibStart;
}

char LogZlibBuffer::__GetMagicAsyncStart() {
    return is_crypt_ ? LogMagicNum::kMagicAsyncZlibStart : LogMagicNum::kMagicAsyncNoCryptZlibStart;
}

}  // namespace xlog
}  // namespace mars
