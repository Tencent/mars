//
// Created by gjt on 8/22/24.
//
#include "mars/comm/xlogger/xlog_c_api.h"

#include "mars/comm/strutil.h"
#include "mars/xlog/appender.h"
#include "mars/xlog/src/log_base_buffer.h"
#include "mars/xlog/src/log_zlib_buffer.h"
#include "mars/xlog/xlogger_interface.h"
#include "xlogger/xlogger.h"

CStrVec VectorString2CStrVec(const std::vector<std::string>& vec_str) {
    CStrVec c_str_vec;
    c_str_vec.length = vec_str.size();
    c_str_vec.vec = new char*[c_str_vec.length];
    for (int i = 0; i < c_str_vec.length; ++i) {
        c_str_vec.vec[i] = new char[vec_str[i].size()];
        memcpy(c_str_vec.vec[i], vec_str[i].c_str(), vec_str[i].length());
    }
    return c_str_vec;
}

// xlog_c_api.h依赖了xloggerbase.h，xloggerbase.h在编译mars时位于mars/comm/xlogger，导出后位于mars/xlog
// 因此xlog_c_api.h和xloggerbase.h必须在同一个文件夹中。
// 但是xlog_c_api.cc只能在xlog.a中，不能在comm.a中。否则某些只链接了comm.a没有链接xlog.a的so会找不到mars::xlog::xxx符号
extern "C" {

uintptr_t CXlogNewXloggerInstance(const CXlogConfig& _config, TLogLevel _level) {
    mars::xlog::XLogConfig cfg;
    cfg.mode_ = static_cast<mars::xlog::TAppenderMode>(_config.mode_);
    cfg.logdir_ = strutil::CStr2StringSafe(_config.logdir_);
    cfg.nameprefix_ = strutil::CStr2StringSafe(_config.nameprefix_);
    cfg.pub_key_ = strutil::CStr2StringSafe(_config.pub_key_);
    cfg.compress_mode_ = static_cast<mars::xlog::TCompressMode>(_config.compress_mode_);
    cfg.compress_level_ = _config.compress_level_;
    cfg.cache_days_ = _config.cache_days_;
    cfg.cachedir_ = strutil::CStr2StringSafe(_config.cachedir_);
    return (uintptr_t)(mars::xlog::NewXloggerInstance(cfg, _level));
}
uintptr_t CXlogGetXloggerInstance(const char* _nameprefix) {
    return (uintptr_t)(mars::xlog::GetXloggerInstance(_nameprefix));
}
TLogLevel CXlogGetLevel(uintptr_t _instance_ptr) {
    return mars::xlog::GetLevel(_instance_ptr);
}
void CXlogFlush(uintptr_t _instance_ptr, bool _is_sync) {
    mars::xlog::Flush(_instance_ptr, _is_sync);
}
void CXlogFlushAll(bool _is_sync) {
    mars::xlog::FlushAll(_is_sync);
}
bool CXlogAppenderMakeLogFileName(int _timespan, char const* _prefix, CStrVec& c_str_vec) {
    std::vector<std::string> str_vec;
    bool rtn = mars::xlog::appender_make_logfile_name(_timespan, _prefix, str_vec);
    c_str_vec = VectorString2CStrVec(str_vec);
    return rtn;
}
void CXlogAppenderSetMode(CXlogTAppenderMode mode) {
    mars::xlog::appender_setmode(static_cast<mars::xlog::TAppenderMode>(mode));
}
bool CXlogAppenderGetCurrentLogPath(char* path, unsigned int length) {
    return mars::xlog::appender_get_current_log_path(path, length);
}
bool CXlogAppenderGetCurrentLogCachePath(char* path, unsigned int length) {
    return mars::xlog::appender_get_current_log_cache_path(path, length);
}
void CXlogAppenderClose() {
    mars::xlog::appender_close();
}
void CXlogAppenderFlushSync() {
    mars::xlog::appender_flush_sync();
}
void CXlogFreeCStrVec(const CStrVec& c_str_vec) {
    for (int i = 0; i < c_str_vec.length; ++i) {
        free(c_str_vec.vec[i]);
    }
    free(c_str_vec.vec);
}
bool CXlogGetPeriodLogs(char const* path,
                        int _begin_hour,
                        int _end_hour,
                        uint64_t& _begin_pos,
                        uint64_t& _end_pos,
                        char*& _err_msg) {
    std::string err;
    bool rtn = mars::xlog::LogBaseBuffer::GetPeriodLogs(path, _begin_pos, _end_pos, _begin_pos, _end_pos, err);
    if (_err_msg != nullptr) {
        return rtn;
    }
    _err_msg = new char[err.length()];
    memcpy(_err_msg, err.c_str(), err.length());
    return rtn;
}
void CXlogZlibBufferWrite(char const* in, int in_size, void*& out, int& out_size) {
    AutoBuffer tmp_buff;
    mars::xlog::LogZlibBuffer log_buffer(NULL, 0, false, "");
    log_buffer.Write(in, in_size, tmp_buff);
    out_size = 0;
    xassert2(out == nullptr);
    if (out != nullptr) {
        return;
    }
    out = new char[tmp_buff.Length()];
    out_size = tmp_buff.Length();
    memcpy(out, tmp_buff.Ptr(), out_size);
}
bool CXlogIsEnabledFor(uintptr_t _instance_ptr, TLogLevel _level) {
    return mars::xlog::IsEnabledFor(_instance_ptr, _level);
}
void CXlogXloggerWrite(uintptr_t _instance_ptr, const XLoggerInfo* _info, const char* _log) {
    mars::xlog::XloggerWrite(_instance_ptr, _info, _log);
}

}  // extern "C"
