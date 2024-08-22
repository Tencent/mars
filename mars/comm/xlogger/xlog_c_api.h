//
// Created by gjt on 8/22/24.
//

#ifndef MARS_XLOG_C_API_H
#define MARS_XLOG_C_API_H

// xlog_c_api.cc在mars/xlog目录，原因见xlog_c_api.cc注释
#include <stdint.h>

#include "xloggerbase.h"

#ifdef __cplusplus
extern "C" {
#endif

enum CXlogTAppenderMode {
    kAppenderAsync,
    kAppenderSync,
};
enum CXlogTCompressMode {
    kZlib,
    kZstd,
};

struct CXlogConfig {
    CXlogTAppenderMode mode_ = kAppenderAsync;
    char* logdir_;
    char* nameprefix_;
    char* pub_key_;
    CXlogTCompressMode compress_mode_ = kZlib;
    int compress_level_ = 6;
    char* cachedir_;
    int cache_days_ = 0;
};

uintptr_t CXlogNewXloggerInstance(const CXlogConfig& _config, TLogLevel _level);
uintptr_t CXlogGetXloggerInstance(const char* _nameprefix);
struct CStrVec {
    char** vec;
    int length;
};
bool CXlogAppenderMakeLogFileName(int _timespan, char const* _prefix, CStrVec& c_str_vec);
void CXlogAppenderSetMode(CXlogTAppenderMode _mode);
bool CXlogAppenderGetCurrentLogPath(char* path, unsigned int length);
bool CXlogAppenderGetCurrentLogCachePath(char* path, unsigned int length);
void CXlogAppenderClose();
void CXlogAppenderFlushSync();
void CXlogFreeCStrVec(const CStrVec& c_str_vec);
bool CXlogGetPeriodLogs(char const* path,
                        int _begin_hour,
                        int _end_hour,
                        uint64_t& _begin_pos,
                        uint64_t& _end_pos,
                        char*& _err_msg);
void CXlogZlibBufferWrite(char const* in, int in_size, void*& out, int& out_size);
bool CXlogIsEnabledFor(uintptr_t _instance_ptr, TLogLevel _level);
void CXlogXloggerWrite(uintptr_t _instance_ptr, const XLoggerInfo* _info, const char* _log);
void CXlogFlush(uintptr_t _instance_ptr, bool _is_sync);
void CXlogFlushAll(bool _is_sync);
TLogLevel CXlogGetLevel(uintptr_t _instance_ptr);
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // MARS_XLOG_C_API_H
