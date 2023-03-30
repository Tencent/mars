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
 * log_formater.cpp
 *
 *  Created on: 2013-3-8
 *      Author: yerungui
 */


#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <algorithm>
#include <sys/time.h>


#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/ptrbuffer.h"
#include "mars/comm/tl_varint_v.h"

#ifdef _WIN32
#define PRIdMAX "lld"
#define snprintf _snprintf
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

namespace mars {
namespace xlog {

static char kBinaryMagicNum = 1;
static char kStrNotNullInfoMagicNum = 2;
static char kStrNullInfoMagicNum = 3;

static std::atomic_uint64_t sg_log_seq(0);


inline size_t varint_decode(char *input, size_t inputSize, uint64_t& value) {
    value = 0;
    size_t i = 0;
    for (; i < inputSize; i++) {
        value |= (input[i] & 127) << (7 * i);
        // If the next-byte flag is set
        if (!(input[i] & 128)) {
            break;
        }
    }
    return i+1;
}

void log_formater(const XBLoggerInfo *_info, const char *_logbody, size_t _size,
                  PtrBuffer& _log) {
    // |char(magic)|uint16_t(len)|char(level)|uint32_t(file)|uint32_t(fun)|uint32_t(line)|int64_t(pid)|int64_t(tid)|char(ismaintid)|
    char *ptr = (char *)_log.PosPtr();
    ptr[0] = kBinaryMagicNum;
    uint16_t len = 3;
    uint64_t seq = sg_log_seq++;

    using mars::comm::varint_encode;
    len += varint_encode(seq, ptr + len);
    len += varint_encode((int)_info->level, ptr + len);
    len += varint_encode(_info->filename, ptr + len);
    len += varint_encode(_info->func_name, ptr + len);
    len += varint_encode(_info->line, ptr + len);
    len += varint_encode(_info->pid, ptr + len);
    len += varint_encode(_info->tid, ptr + len);
    len += varint_encode(_info->tid == _info->maintid, ptr + len);
    time_t sec = _info->timeval.tv_sec;
    tm tm = *localtime((const time_t *)&sec);
    len += varint_encode(tm.tm_year, ptr + len);
    len += varint_encode(tm.tm_mon, ptr + len);
    len += varint_encode(tm.tm_mday, ptr + len);
    len += varint_encode(tm.tm_gmtoff, ptr + len);
    len += varint_encode(tm.tm_hour, ptr + len);
    len += varint_encode(tm.tm_min, ptr + len);
    len += varint_encode(tm.tm_sec, ptr + len);
    len += varint_encode(_info->timeval.tv_usec / 1000, ptr + len);

//    len += varint_encode(123, ptr + len);
//    len += varint_encode(2, ptr + len);
//    len += varint_encode(17, ptr + len);
//    len += varint_encode(28800, ptr + len);
//    len += varint_encode(15, ptr + len);
//    len += varint_encode(56, ptr + len);
//    len += varint_encode(30, ptr + len);
//    len += varint_encode(562000 / 1000, ptr + len);

    if (nullptr != _info->tag && strnlen(_info->tag, 512) > 0) {
        size_t tag_size = strnlen(_info->tag, 512);
        len += varint_encode(tag_size, ptr + len);
        memcpy(ptr + len, _info->tag, tag_size);
        len += tag_size;
    } else {
        len += varint_encode(0, ptr + len);
    }
    _size = std::min(_size, _log.MaxLength() - 4 - len);
//    len += varint_encode(_size, ptr + len);
    memcpy(ptr + len, _logbody, _size);
    len += _size;
    ptr[len] = kBinaryMagicNum;
    len += 1;
    _log.Length((off_t)len, (size_t)len);
    len -= 4;
    memcpy(ptr + 1, &len, sizeof(len));

    ////////////// 测试代码
//    uint16_t dlen = 3;
//    uint64_t level = 0;
//    uint64_t file = 0;
//    uint64_t fun = 0;
//    uint64_t line = 0;
//    uint64_t pid = 0;
//    uint64_t tid = 0;
//    uint64_t ismaintid = 0;
//    uint64_t year = 0;
//    uint64_t month = 0;
//    uint64_t day = 0;
//    uint64_t gmtoff = 0;
//    uint64_t hour = 0;
//    uint64_t min = 0;
//    uint64_t second = 0;
//    uint64_t millsecond = 0;
//    uint64_t tag_size = 0;
//    uint64_t format_id = 0;
//    uint64_t type = 0;
//    len += 3;
//    dlen += varint_decode(ptr + dlen, len - dlen, level);
//    dlen += varint_decode(ptr + dlen, len - dlen, file);
//    dlen += varint_decode(ptr + dlen, len - dlen, fun);
//    dlen += varint_decode(ptr + dlen, len - dlen, line);
//    dlen += varint_decode(ptr + dlen, len - dlen, pid);
//    dlen += varint_decode(ptr + dlen, len - dlen, tid);
//    dlen += varint_decode(ptr + dlen, len - dlen, ismaintid);
//    dlen += varint_decode(ptr + dlen, len - dlen, year);
//    dlen += varint_decode(ptr + dlen, len - dlen, month);
//    dlen += varint_decode(ptr + dlen, len - dlen, day);
//    dlen += varint_decode(ptr + dlen, len - dlen, gmtoff);
//    dlen += varint_decode(ptr + dlen, len - dlen, hour);
//    dlen += varint_decode(ptr + dlen, len - dlen, min);
//    dlen += varint_decode(ptr + dlen, len - dlen, second);
//    dlen += varint_decode(ptr + dlen, len - dlen, millsecond);
//    dlen += varint_decode(ptr + dlen, len - dlen, tag_size);
//    if (tag_size > 0) {
//
//    }
//    dlen += varint_decode(ptr + dlen, len - dlen, format_id);
//    dlen += varint_decode(ptr + dlen, len - dlen, type);
//    if (2 == type) {
//        uint64_t str_size = 0;
//        dlen += varint_decode(ptr + dlen, len - dlen, str_size);
//        char* log_buffer = new char[str_size];
//        memcpy(log_buffer, ptr + dlen, str_size);
//        printf("123");
//    }


}


void log_formater(const XLoggerInfo* _info, const char* _logbody, PtrBuffer& _log) {
    assert((unsigned int)_log.Pos() == _log.Length());

    static int error_count = 0;
    static int error_size = 0;
    XLoggerInfo* info = const_cast<XLoggerInfo*>(_info);

    // TODO 处理异常错误信息的附加信息
    if (_log.MaxLength() <= _log.Length() + 5 * 1024) {  // allowed len(_log) <= 11K(16K - 5K)
        ++error_count;
        error_size = (int)strnlen(_logbody, 1024 * 1024);

        if (_log.MaxLength() >= _log.Length() + 128) {
            int ret = snprintf((char*)_log.PosPtr(), 1024, "[F]log_size <= 5*1024, err(%d, %d)\n", error_count, error_size);  // **CPPLINT SKIP**
            _log.Length(_log.Pos() + ret, _log.Length() + ret);
            _log.Write("");

            error_count = 0;
            error_size = 0;
        }

        assert(false);
        return;
    }

    char *ptr = (char *)_log.PosPtr();
    uint16_t len = 3;
    uint64_t seq = sg_log_seq++;

    using mars::comm::varint_encode;
    if (NULL != info) {
        if (0 != info->timeval.tv_sec) {
            gettimeofday(&info->timeval, NULL);
        }
        const char* filename = ExtractFileName(info->filename);

#if _WIN32
        char strFuncName [128] = {0};
        ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));
#else
        const char* strFuncName = NULL == info->func_name ? "" : info->func_name;
#endif

        time_t sec = info->timeval.tv_sec;
        tm tm = *localtime((const time_t*)&sec);

        ptr[0] = kStrNotNullInfoMagicNum;
        len += varint_encode(seq, ptr + len);
        len += varint_encode((int)info->level, ptr + len);

        if (filename != NULL && strnlen(filename, 512) > 0) {
            size_t filename_size = strnlen(filename, 512);
            len += varint_encode(filename_size, ptr + len);
            memcpy(ptr + len, filename, filename_size);
            len += filename_size;
        } else {
            len += varint_encode(0, ptr + len);
        }
        if (strFuncName != NULL && strnlen(strFuncName, 512) > 0) {
            size_t fun_size = strnlen(strFuncName, 512);
            len += varint_encode(fun_size, ptr + len);
            memcpy(ptr + len, strFuncName, fun_size);
            len += fun_size;
        } else {
            len += varint_encode(0, ptr + len);
        }
        len += varint_encode(info->line, ptr + len);
        len += varint_encode(info->pid, ptr + len);
        len += varint_encode(info->tid, ptr + len);
        len += varint_encode(info->tid == info->maintid, ptr + len);
        len += varint_encode(tm.tm_year, ptr + len);
        len += varint_encode(tm.tm_mon, ptr + len);
        len += varint_encode(tm.tm_mday, ptr + len);
        len += varint_encode(tm.tm_gmtoff, ptr + len);
        len += varint_encode(tm.tm_hour, ptr + len);
        len += varint_encode(tm.tm_min, ptr + len);
        len += varint_encode(tm.tm_sec, ptr + len);
        len += varint_encode(info->timeval.tv_usec / 1000, ptr + len);
        if (nullptr != info->tag && strnlen(info->tag, 512) > 0) {
            size_t tag_size = strnlen(info->tag, 512);
            len += varint_encode(tag_size, ptr + len);
            memcpy(ptr + len, info->tag, tag_size);
            len += tag_size;
        } else {
            len += varint_encode(0, ptr + len);
        }
    } else {
        ptr[0] = kStrNullInfoMagicNum;
        len += varint_encode(seq, ptr + len);
    }

    if (NULL != _logbody) {
        size_t log_size = std::min(strnlen(_logbody, 64 * 1024), _log.MaxLength() - 4 - len);
        memcpy(ptr + len, _logbody, log_size);
        len += log_size;
    } else {
        std::string err_log = "error!! NULL==_logbody";
        memcpy(ptr + len, err_log.data(), err_log.size());
        len += err_log.size();
    }
    ptr[len] = ptr[0];
    len += 1;
    _log.Length((off_t)len, (size_t)len);
    len -= 4;
    memcpy(ptr + 1, &len, sizeof(len));
}

void log_formater2(const XLoggerInfo* _info, const char* _logbody, PtrBuffer& _log) {
    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"  // fatal
    };

    assert((unsigned int)_log.Pos() == _log.Length());

    static int error_count = 0;
    static int error_size = 0;

    if (_log.MaxLength() <= _log.Length() + 5 * 1024) {  // allowed len(_log) <= 11K(16K - 5K)
        ++error_count;
        error_size = (int)strnlen(_logbody, 1024 * 1024);

        if (_log.MaxLength() >= _log.Length() + 128) {
            int ret = snprintf((char*)_log.PosPtr(), 1024, "[F]log_size <= 5*1024, err(%d, %d)\n", error_count, error_size);  // **CPPLINT SKIP**
            _log.Length(_log.Pos() + ret, _log.Length() + ret);
            _log.Write("");

            error_count = 0;
            error_size = 0;
        }

        assert(false);
        return;
    }

    if (NULL != _info) {
        const char* filename = ExtractFileName(_info->filename);

#if _WIN32
        char strFuncName [128] = {0};
        ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));
#else
        const char* strFuncName = NULL == _info->func_name ? "" : _info->func_name;
#endif

        char temp_time[64] = {0};

        if (0 != _info->timeval.tv_sec) {
            time_t sec = _info->timeval.tv_sec;
            tm tm = *localtime((const time_t*)&sec);


#ifdef ANDROID
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3ld", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#elif _WIN32
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     (-_timezone) / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#else
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#endif
        }

        // _log.AllocWrite(30*1024, false);
        int ret = snprintf((char*)_log.PosPtr(), 1024, "[%s][%s][%" PRIdMAX ", %" PRIdMAX "%s][%s][%s:%d, %s][",  // **CPPLINT SKIP**
                           _logbody ? levelStrings[_info->level] : levelStrings[kLevelFatal], temp_time,
                           _info->pid, _info->tid, _info->tid == _info->maintid ? "*" : "", _info->tag ? _info->tag : "",
                           filename, _info->line, strFuncName);

        assert(0 <= ret);
        _log.Length(_log.Pos() + ret, _log.Length() + ret);
        //      memcpy((char*)_log.PosPtr() + 1, "\0", 1);

        assert((unsigned int)_log.Pos() == _log.Length());
    }

    if (NULL != _logbody) {
        // in android 64bit, in strnlen memchr,  const unsigned char*  end = p + n;  > 4G!!!!! in stack array

        size_t bodylen =  _log.MaxLength() - _log.Length() > 130 ? _log.MaxLength() - _log.Length() - 130 : 0;
        bodylen = bodylen > 0xFFFFU ? 0xFFFFU : bodylen;
        bodylen = strnlen(_logbody, bodylen);
        bodylen = bodylen > 0xFFFFU ? 0xFFFFU : bodylen;
        _log.Write(_logbody, bodylen);
    } else {
        _log.Write("error!! NULL==_logbody");
    }

    char nextline = '\n';

    if (*((char*)_log.PosPtr() - 1) != nextline) _log.Write(&nextline, 1);
}

}
}
