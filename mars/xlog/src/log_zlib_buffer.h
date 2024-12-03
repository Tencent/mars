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
 * log_zlib_buffer.h
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#ifndef LOGZLIBBUFFER_H_
#define LOGZLIBBUFFER_H_

#include <stdint.h>
#ifdef _WIN32
#include <zlib/zlib.h> // mars/mars/comm/windows/zlib/zlib.h
#else
#include <zlib.h>
#endif

#include <string>

#include "log_base_buffer.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/ptrbuffer.h"

namespace mars {
namespace xlog {

class LogCrypt;

class LogZlibBuffer : public LogBaseBuffer {
 public:
    LogZlibBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey);
    ~LogZlibBuffer();

 public:
    virtual size_t Compress(const void* src, size_t inLen, void* dst, size_t outLen);
    virtual void Flush(AutoBuffer& _buff);

 private:
    bool __Reset();
    char __GetMagicSyncStart();
    char __GetMagicAsyncStart();

 private:
    z_stream cstream_;
};

}  // namespace xlog
}  // namespace mars

#endif /* LOGBUFFER_H_ */
