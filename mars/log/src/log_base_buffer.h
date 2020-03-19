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
 * log_base_buffer.h
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#ifndef LOGBASEBUFFER_H_
#define LOGBASEBUFFER_H_

#include <zlib.h>
#include <string>
#include <stdint.h>

#include "mars/comm/ptrbuffer.h"
#include "mars/comm/autobuffer.h"

typedef enum {
    ZLIB,
    ZSTD,
} compressMode;

class LogCrypt;

class LogBaseBuffer {
    
public:
    
    LogBaseBuffer(void* _pbuffer, size_t _len, bool _is_compress, int _compress_mode, const char* _pubkey);
    virtual ~LogBaseBuffer();


public:
    
    static bool GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg);

public:
    
    PtrBuffer& GetData();
    virtual size_t Compress(const void* src, size_t inLen, void* dst, size_t outLen) = 0;
    virtual void Flush(AutoBuffer& _buff);
    bool Write(const void* _data, size_t _length);
    bool Write(const void* _data, size_t _inputlen, AutoBuffer& _out_buff);

protected:

    virtual bool __Reset() = 0;
    void __Flush();
    void __Clear();
    void __Fix();

protected:
    
    PtrBuffer buff_;
    bool is_compress_;
    int compress_mode_;
    class LogCrypt* log_crypt_;
    size_t remain_nocrypt_len_;

};


#endif /* LOGBASEBUFFER_H_ */
