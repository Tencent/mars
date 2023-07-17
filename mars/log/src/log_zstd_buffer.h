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
 * log_zstd_buffer.h
 *
 *  Created on: 2020-3-5
 *      Author: yanguoyue
 */

#ifndef LOGZSTDBUFFER_H_
#define LOGZSTDBUFFER_H_

#include <stdint.h>

#include <string>

#include "log_base_buffer.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/ptrbuffer.h"
#include "zstd/lib/zstd.h"

namespace mars {
namespace xlog {

class LogCrypt;

class LogZstdBuffer : public LogBaseBuffer {
 public:
    LogZstdBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey, int level);
    ~LogZstdBuffer();

 public:
    virtual size_t Compress(const void* src, size_t inLen, void* dst, size_t outLen);
    virtual void Flush(AutoBuffer& _buff);

 private:
    bool __Reset();
    char __GetMagicSyncStart();
    char __GetMagicAsyncStart();

 private:
    ZSTD_CCtx* cctx_;
};

}  // namespace xlog
}  // namespace mars

#endif /* LOGZSTDBUFFER_H_ */
