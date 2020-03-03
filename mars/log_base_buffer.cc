//
//  log_base_buffer.cc
//  app
//
//  Created by 黄伟烽 on 2020/2/28.
//

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
#include "zstd/zstd.h"
#include "zstd/common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()


#ifdef WIN32
#define snprintf _snprintf
#endif

bool LogBaseBuffer::GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg) {
    printf("\nbase GetPeriodLogs\n");
    return LogCrypt::GetPeriodLogs(_log_path, _begin_hour, _end_hour, _begin_pos, _end_pos, _err_msg);
}

LogBaseBuffer::LogBaseBuffer(void* _pbuffer, size_t _len, bool _isCompress, const char* _pubkey){
    this->is_compress_ = _isCompress;
    this->log_crypt_ = new LogCrypt(_pubkey);
    buff_.Attach(_pbuffer, _len);
    __Fix();
}

LogBaseBuffer::~LogBaseBuffer() {

}

bool LogBaseBuffer::__Reset() {
    printf("\nZSTD __Reset\n");
    __Clear();
    
    log_crypt_->SetHeaderInfo((char*)buff_.Ptr(), is_compress_);
    buff_.Length(log_crypt_->GetHeaderLen(), log_crypt_->GetHeaderLen());

    return true;
}

void LogBaseBuffer::__Flush() {
    printf("\nbase__Flush\n");
    assert(buff_.Length() >= log_crypt_->GetHeaderLen());

    log_crypt_->UpdateLogHour((char*)buff_.Ptr());
    log_crypt_->SetTailerInfo((char*)buff_.Ptr() + buff_.Length());
    buff_.Length(buff_.Length() + log_crypt_->GetTailerLen(), buff_.Length() + log_crypt_->GetTailerLen());

}

bool LogBaseBuffer::Write(const void* _data, size_t _inputlen, AutoBuffer& _out_buff) {
    if (NULL == _data || 0 == _inputlen) {
        return false;
    }

    log_crypt_->CryptSyncLog((char*)_data, _inputlen, _out_buff);

    return true;
}

void LogBaseBuffer::__Clear() {
    printf("\nbase__Clear\n");
    memset(buff_.Ptr(), 0, buff_.MaxLength());
    buff_.Length(0, 0);
    remain_nocrypt_len_ = 0;
}


void LogBaseBuffer::__Fix() {
    printf("\nbase__Fix\n");
    uint32_t raw_log_len = 0;
    bool is_compress = false;
    if (log_crypt_->Fix((char*)buff_.Ptr(), buff_.Length(), is_compress, raw_log_len)) {
        buff_.Length(raw_log_len + log_crypt_->GetHeaderLen(), raw_log_len + log_crypt_->GetHeaderLen());
    } else {
        buff_.Length(0, 0);
    }

}
