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

#include <cstdio>
#include <time.h>
#include <algorithm>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "log_zstd_buffer.h"
#include "log/crypt/log_crypt.h"
#include "zstd/zstd.h"
#include "zstd/common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()


#ifdef WIN32
#define snprintf _snprintf
#endif


LogZstdBuffer::LogZstdBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey, int level)
:LogBaseBuffer(_pbuffer, _len, _isCompress, zstdMode, _pubkey) {

    if (is_compress_) {
        cctx = ZSTD_createCCtx();
        ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, level);
    }
}

LogZstdBuffer::~LogZstdBuffer() {

    if (is_compress_ && cctx != nullptr) {
        ZSTD_inBuffer input = {nullptr, 0, 0};
        ZSTD_outBuffer output = {nullptr, 0, 0};
        size_t remaining = ZSTD_compressStream2(cctx, &output , &input, ZSTD_e_end);
        CHECK_ZSTD(remaining);
    }
    
    ZSTD_freeCCtx(cctx);
}

void LogZstdBuffer::Flush(AutoBuffer& _buff) {

    if (is_compress_ && cctx != nullptr) {
        ZSTD_inBuffer input = {nullptr, 0, 0 };
        ZSTD_outBuffer output = {nullptr, 0, 0 };
        size_t remaining = ZSTD_compressStream2(cctx, &output , &input, ZSTD_e_end);
        CHECK_ZSTD(remaining);
    }

    LogBaseBuffer::Flush(_buff);
}


size_t LogZstdBuffer::compress(const void* src, size_t inLen, void* dst, size_t outLen){

    ZSTD_inBuffer input = { src, inLen, 0 };
    ZSTD_outBuffer output = { dst, outLen,  0};
    
    size_t remaining = ZSTD_compressStream2(cctx, &output, &input, ZSTD_e_flush);
    CHECK_ZSTD(remaining);
    
    return output.pos;
}

bool LogZstdBuffer::__Reset() {

    __Clear();
   
    log_crypt_->SetHeaderInfo((char*)buff_.Ptr(), is_compress_, compress_mode_);
    buff_.Length(log_crypt_->GetHeaderLen(), log_crypt_->GetHeaderLen());
    
    if (is_compress_) {
        ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only);
    }

    return true;
}
