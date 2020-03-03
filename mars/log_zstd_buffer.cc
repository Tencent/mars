//
//  log_zstd_buffer.cc
//  app
//
//  Created by 黄伟烽 on 2020/3/1.
//

#include "log_zstd_buffer.h"


#include <cstdio>
#include <time.h>
#include <algorithm>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include "log/crypt/log_crypt.h"
#include "zstd/zstd.h"
#include "zstd/common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()


#ifdef WIN32
#define snprintf _snprintf
#endif


LogZstdBuffer::LogZstdBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey, int level)
:LogBaseBuffer(_pbuffer, _len, _isCompress, _pubkey){

    printf("\nLogZSTDBuffer-构造函数\n");
    buff_.Attach(_pbuffer, _len);
    __Fix();

    cctx = ZSTD_createCCtx();
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, level);

}

LogZstdBuffer::~LogZstdBuffer() {

    printf("\n~LogZSTDBuffer\n");

    if (is_compress_ && cctx != nullptr){
        ZSTD_inBuffer input = {nullptr, 0, 0 };
        ZSTD_outBuffer output = {nullptr, 0, 0 };
        size_t remaining = ZSTD_compressStream2(cctx, &output , &input, ZSTD_e_end);
        CHECK_ZSTD(remaining);
    }
    
    delete log_crypt_;
}

PtrBuffer& LogZstdBuffer::GetData() {
    printf("\nZSTD GetData\n");
    return buff_;
}


void LogZstdBuffer::Flush(AutoBuffer& _buff) {
    printf("\nZSTD Flush\n");

    if (is_compress_ && cctx != nullptr){
        ZSTD_inBuffer input = {nullptr, 0, 0 };
        ZSTD_outBuffer output = {nullptr, 0, 0 };
        size_t remaining = ZSTD_compressStream2(cctx, &output , &input, ZSTD_e_end);
        CHECK_ZSTD(remaining);
    }


    if (log_crypt_->GetLogLen((char*)buff_.Ptr(), buff_.Length()) == 0){
        __Clear();
        return;
    }

    __Flush();
    _buff.Write(buff_.Ptr(), buff_.Length());
    __Clear();
}



bool LogZstdBuffer::Write(const void* _data, size_t _length) {
    printf("\nZSTD Write\n");
    if (NULL == _data || 0 == _length) {
        return false;
    }


    if (buff_.Length() == 0) {
        if (!__Reset()) return false;
    }

    size_t before_len = buff_.Length();
    size_t write_len = _length;
    
    if (is_compress_) {

        ZSTD_inBuffer input = { _data, _length, 0 };
        
        uInt avail_out = (uInt)(buff_.MaxLength() - buff_.Length());
        ZSTD_outBuffer output = {buff_.PosPtr(), avail_out,  0};
        size_t remaining = ZSTD_compressStream2(cctx, &output, &input, ZSTD_e_flush);
        CHECK_ZSTD(remaining);
        write_len = avail_out - (output.size - output.pos);
        
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

bool LogZstdBuffer::__Reset() {
    
    LogBaseBuffer::__Reset();
    
    if (is_compress_) {
        ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only);
    }

    return true;
}
