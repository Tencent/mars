//
//  log_zstd_buffer.h
//  app
//
//  Created by 黄伟烽 on 2020/3/1.
//

#ifndef log_zstd_buffer_h
#define log_zstd_buffer_h

#include <string>
#include <stdint.h>
#include "zstd/zstd.h"
#include "log_base_buffer.h"

#include "mars/comm/ptrbuffer.h"
#include "mars/comm/autobuffer.h"

class LogCrypt;

class LogZstdBuffer : public LogBaseBuffer{
public:
    LogZstdBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey, int level);
    
    ~LogZstdBuffer();
    
public:
    PtrBuffer& GetData();
    
    void Flush(AutoBuffer& _buff);
    bool Write(const void* _data, size_t _length);

private:
    
    bool __Reset();


private:
    ZSTD_CCtx* cctx;
};

#endif /* log_zstd_buffer_h */
