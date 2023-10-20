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
 * log_zstd_buffer.cpp
 *
 *  Created on: 2020-3-5
 *      Author: yanguoyue
 */

#include "log_zstd_buffer.h"

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

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace mars {
namespace xlog {

LogZstdBuffer::LogZstdBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey, int level)
: LogBaseBuffer(_pbuffer, _len, _isCompress, _pubkey) {
    if (is_compress_) {
        cctx_ = ZSTD_createCCtx();
        ZSTD_CCtx_setParameter(cctx_, ZSTD_c_compressionLevel, level);
        ZSTD_CCtx_setParameter(cctx_, ZSTD_c_windowLog, 16);
    }
}

LogZstdBuffer::~LogZstdBuffer() {
    if (is_compress_ && cctx_ != nullptr) {
        ZSTD_inBuffer input = {nullptr, 0, 0};
        ZSTD_outBuffer output = {nullptr, 0, 0};
        ZSTD_compressStream2(cctx_, &output, &input, ZSTD_e_end);
        ZSTD_freeCCtx(cctx_);
    }
}

void LogZstdBuffer::Flush(AutoBuffer& _buff) {
    if (is_compress_ && cctx_ != nullptr) {
        ZSTD_inBuffer input = {nullptr, 0, 0};
        ZSTD_outBuffer output = {nullptr, 0, 0};
        ZSTD_compressStream2(cctx_, &output, &input, ZSTD_e_end);
    }

    LogBaseBuffer::Flush(_buff);
}

size_t LogZstdBuffer::Compress(const void* src, size_t inLen, void* dst, size_t outLen) {
    ZSTD_inBuffer input = {src, inLen, 0};
    ZSTD_outBuffer output = {dst, outLen, 0};

    ZSTD_compressStream2(cctx_, &output, &input, ZSTD_e_flush);

    return output.pos;
}

bool LogZstdBuffer::__Reset() {
    if (!LogBaseBuffer::__Reset()) {
        return false;
    }

    if (is_compress_) {
        ZSTD_CCtx_reset(cctx_, ZSTD_reset_session_only);
    }

    return true;
}

char LogZstdBuffer::__GetMagicSyncStart() {
    return is_crypt_ ? LogMagicNum::kMagicSyncZstdStart : LogMagicNum::kMagicSyncNoCryptZstdStart;
}

char LogZstdBuffer::__GetMagicAsyncStart() {
    return is_crypt_ ? LogMagicNum::kMagicAsyncZstdStart : LogMagicNum::kMagicAsyncNoCryptZstdStart;
}

}  // namespace xlog
}  // namespace mars
