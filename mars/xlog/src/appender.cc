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
 * appender.h
 *
 *  Created on: 2013-3-7
 *      Author: yerungui
 */

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <locale>

#include "boost/filesystem/detail/utf8_codecvt_facet.hpp"
#include "boost/filesystem/path.hpp"
#endif

#include <stdio.h>

#include "mars/xlog/appender.h"

#ifdef _WIN32
#define PRIdMAX "lld"
#define snprintf _snprintf
#define strcasecmp _stricmp
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define fileno _fileno
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sys/mount.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <zlib.h>

#include <algorithm>
#include <string>

#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/iostreams/device/mapped_file.hpp"
#include "mars/comm/autobuffer.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/mmap_util.h"
#include "mars/comm/ptrbuffer.h"
#include "mars/comm/strutil.h"
#include "mars/comm/thread/condition.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/tickcount.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/verinfo.h"
#include "mars/comm/xlogger/xloggerbase.h"

#ifdef __APPLE__
#include "mars/comm/objc/data_protect_attr.h"
#endif

#include "log_base_buffer.h"
#include "log_zlib_buffer.h"
#include "log_zstd_buffer.h"
#include "xlogger_appender.h"

#define LOG_EXT "xlog"

using namespace mars::comm;

namespace mars {
namespace xlog {

extern void log_formater(const XLoggerInfo* _info, const char* _logbody, PtrBuffer& _log);
extern void ConsoleLog(const XLoggerInfo* _info, const char* _log);

static const int kMaxDumpLength = 4096;

static const unsigned int kBufferBlockLength = 150 * 1024;
static const long kMinLogAliveTime = 24 * 60 * 60;  // 1 days in second

static Mutex sg_mutex_dir_attr;

void (*g_log_write_callback)(const XLoggerInfo*, const char*) = nullptr;

namespace {
class ScopeErrno {
 public:
    ScopeErrno() {
        m_errno = errno;
    }
    ~ScopeErrno() {
        errno = m_errno;
    }

 private:
    ScopeErrno(const ScopeErrno&);
    const ScopeErrno& operator=(const ScopeErrno&);

 private:
    int m_errno;
};

#define SCOPE_ERRNO() SCOPE_ERRNO_I(__LINE__)
#define SCOPE_ERRNO_I(line) SCOPE_ERRNO_II(line)
#define SCOPE_ERRNO_II(line) ScopeErrno __scope_errno_##line

}  // namespace

#ifdef WIN32
// sometimes just use xlog
// also can use std::call_once
static void SetConvertToUseUtf8() {
    static std::atomic<bool> once_flag = false;
    if (!once_flag) {
        once_flag = true;
        boost::filesystem::path::imbue(std::locale(std::locale(), new boost::filesystem::detail::utf8_codecvt_facet));
    }
}
#endif

XloggerAppender* XloggerAppender::NewInstance(const XLogConfig& _config, uint64_t _max_byte_size) {
    return new XloggerAppender(_config, _max_byte_size);
}

XloggerAppender* XloggerAppender::NewInstance(const XLogConfig& _config, uint64_t _max_byte_size, bool _one_shot) {
    return new XloggerAppender(_config, _max_byte_size, _one_shot);
}

void XloggerAppender::DelayRelease(XloggerAppender* _appender) {
    if (_appender->log_close_) {
        return;
    }
    _appender->Close();
    Thread(boost::bind(&Release, _appender)).start_after(5000);
}

void XloggerAppender::Release(XloggerAppender*& _appender) {
    _appender->Close();
    delete _appender;
    _appender = nullptr;
}

XloggerAppender::XloggerAppender(const XLogConfig& _config, uint64_t _max_byte_size)
: thread_async_(boost::bind(&XloggerAppender::__AsyncLogThread, this)), max_file_size_(_max_byte_size) {
    Open(_config);
}

XloggerAppender::XloggerAppender(const XLogConfig& _config, uint64_t _max_byte_size, bool _one_shot)
: thread_async_(boost::bind(&XloggerAppender::__AsyncLogThread, this)), max_file_size_(_max_byte_size) {
    config_ = _config;
}

void XloggerAppender::Write(const XLoggerInfo* _info, const char* _log) {
    if (log_close_)
        return;

    SCOPE_ERRNO();

    thread_local uint32_t recursion_count = 0;
    thread_local std::string recursion_str;
    recursion_count++;

    if (consolelog_open_)
        ConsoleLog(_info, _log);
#ifdef ANDROID
    else if (_info && _info->traceLog == 1)
        ConsoleLog(_info, _log);
#endif
    if (g_log_write_callback) {
        g_log_write_callback(_info, _log);
    }

    if (2 <= recursion_count && recursion_str.empty()) {
        if (recursion_count > 10)
            return;

        recursion_str.resize(kMaxDumpLength);
        XLoggerInfo info = *_info;
        info.level = kLevelFatal;

        char recursive_log[256] = {0};
        snprintf(recursive_log,
                 sizeof(recursive_log),
                 "ERROR!!! xlogger_appender Recursive calls!!!, count:%u",
                 recursion_count);

        PtrBuffer tmp((void*)recursion_str.data(), 0, kMaxDumpLength);
        log_formater(&info, recursive_log, tmp);

        if (recursion_str.capacity() >= strnlen(_log, kMaxDumpLength)) {
            recursion_str += _log;
        }

        ConsoleLog(&info, recursion_str.c_str());
    } else {
        if (!recursion_str.empty()) {
            WriteTips2File(recursion_str.c_str());
            recursion_str.clear();
        }

        if (kAppenderSync == config_.mode_)
            __WriteSync(_info, _log);
        else
            __WriteAsync(_info, _log);
    }
    recursion_count--;
}

void XloggerAppender::SetMode(TAppenderMode _mode) {
    config_.mode_ = _mode;

    cond_buffer_async_.notifyAll();

    if (kAppenderAsync == config_.mode_ && !thread_async_.isruning()) {
        thread_async_.start();
    }
}

void XloggerAppender::Flush() {
    cond_buffer_async_.notifyAll();
}

void XloggerAppender::FlushSync() {
    if (kAppenderSync == config_.mode_) {
        return;
    }

    ScopedLock lock_buffer(mutex_buffer_async_);

    if (nullptr == log_buff_)
        return;

    AutoBuffer tmp;
    log_buff_->Flush(tmp);

    lock_buffer.unlock();

    if (tmp.Ptr())
        __Log2File(tmp.Ptr(), tmp.Length(), false);
}

void XloggerAppender::Close() {
    if (thread_timeout_cache_ && thread_timeout_cache_->isruning())
        thread_timeout_cache_->cancel_after();
    if (thread_moveold_ && thread_moveold_->isruning())
        thread_moveold_->cancel_after();
    if (thread_timeout_log_ && thread_timeout_log_->isruning())
        thread_timeout_log_->cancel_after();
    thread_timeout_cache_ = nullptr;
    thread_moveold_ = nullptr;
    thread_timeout_log_ = nullptr;

    if (log_close_) return;

    char mark_info[512] = {0};
    __GetMarkInfo(mark_info, sizeof(mark_info));
    char appender_info[728] = {0};
    snprintf(appender_info, sizeof(appender_info), "$$$$$$$$$$" __DATE__ "$$$" __TIME__ "$$$$$$$$$$%s\n", mark_info);
    Write(nullptr, appender_info);

    log_close_ = true;

    cond_buffer_async_.notifyAll();

    if (thread_async_.isruning())
        thread_async_.join();

    ScopedLock buffer_lock(mutex_buffer_async_);
    if (mmap_file_.is_open()) {
        if (!mmap_file_.operator!())
            memset(mmap_file_.data(), 0, kBufferBlockLength);

        CloseMmapFile(mmap_file_);
    } else {
        if (nullptr != log_buff_) {
            delete[](char*)((log_buff_->GetData()).Ptr());
        }
    }

    delete log_buff_;
    log_buff_ = nullptr;
    buffer_lock.unlock();

    ScopedLock lock(mutex_log_file_);
    __CloseLogFile();
}

void XloggerAppender::Open(const XLogConfig& _config) {
#ifdef WIN32
    // here we make sure boost convert path is utf8 to wide on windows
    SetConvertToUseUtf8();
#endif
    config_ = _config;

    ScopedLock dir_attr_lock(sg_mutex_dir_attr);
    if (!config_.cachedir_.empty()) {
        boost::filesystem::create_directories(config_.cachedir_);

        thread_timeout_cache_ =
            std::make_unique<comm::Thread>(boost::bind(&XloggerAppender::__DelTimeoutFile, this, config_.cachedir_));
        thread_timeout_cache_->start_after(2 * 60 * 1000);
        thread_moveold_ = std::make_unique<comm::Thread>(boost::bind(&XloggerAppender::__MoveOldFiles,
                                                                     this,
                                                                     config_.cachedir_,
                                                                     config_.logdir_,
                                                                     config_.nameprefix_));
        thread_moveold_->start_after(3 * 60 * 1000);
#ifdef __APPLE__
        setAttrProtectionNone(config_.cachedir_.c_str());
#endif
    }

    thread_timeout_log_ =
        std::make_unique<comm::Thread>(boost::bind(&XloggerAppender::__DelTimeoutFile, this, config_.logdir_));
    thread_timeout_log_->start_after(2 * 60 * 1000);
    boost::filesystem::create_directories(config_.logdir_);
#ifdef __APPLE__
    setAttrProtectionNone(config_.logdir_.c_str());
#endif
    dir_attr_lock.unlock();

    tickcount_t tick;
    tick.gettickcount();

    char mmap_file_path[512] = {0};
    snprintf(mmap_file_path,
             sizeof(mmap_file_path),
             "%s/%s.mmap3",
             config_.cachedir_.empty() ? config_.logdir_.c_str() : config_.cachedir_.c_str(),
             config_.nameprefix_.c_str());
    bool use_mmap = false;
    if (OpenMmapFile(mmap_file_path, kBufferBlockLength, mmap_file_)) {
        if (_config.compress_mode_ == kZstd) {
            log_buff_ = new LogZstdBuffer(mmap_file_.data(),
                                          kBufferBlockLength,
                                          true,
                                          _config.pub_key_.c_str(),
                                          _config.compress_level_);
        } else {
            log_buff_ = new LogZlibBuffer(mmap_file_.data(), kBufferBlockLength, true, _config.pub_key_.c_str());
        }
        use_mmap = true;
    } else {
        char* buffer = new char[kBufferBlockLength];
        if (_config.compress_mode_ == kZstd) {
            log_buff_ =
                new LogZstdBuffer(buffer, kBufferBlockLength, true, _config.pub_key_.c_str(), _config.compress_level_);
        } else {
            log_buff_ = new LogZlibBuffer(buffer, kBufferBlockLength, true, _config.pub_key_.c_str());
        }
        use_mmap = false;
    }

    if (nullptr == log_buff_->GetData().Ptr()) {
        if (use_mmap && mmap_file_.is_open())
            CloseMmapFile(mmap_file_);
        return;
    }

    AutoBuffer buffer;
    log_buff_->Flush(buffer);

    ScopedLock lock(mutex_log_file_);
    log_close_ = false;
    SetMode(config_.mode_);
    lock.unlock();

    char mark_info[512] = {0};
    __GetMarkInfo(mark_info, sizeof(mark_info));

    if (buffer.Ptr()) {
        WriteTips2File("~~~~~ begin of mmap ~~~~~\n");
        __Log2File(buffer.Ptr(), buffer.Length(), false);
        WriteTips2File("~~~~~ end of mmap ~~~~~%s\n", mark_info);
    }

    tickcountdiff_t get_mmap_time = tickcount_t().gettickcount() - tick;

    char appender_info[728] = {0};
    snprintf(appender_info, sizeof(appender_info), "^^^^^^^^^^" __DATE__ "^^^" __TIME__ "^^^^^^^^^^%s", mark_info);

    Write(nullptr, appender_info);
    char logmsg[256] = {0};
    snprintf(logmsg, sizeof(logmsg), "get mmap time: %" PRIu64, (int64_t)get_mmap_time);
    Write(nullptr, logmsg);

    Write(nullptr, "MARS_URL: " MARS_URL);
    Write(nullptr, "MARS_PATH: " MARS_PATH);
    Write(nullptr, "MARS_REVISION: " MARS_REVISION);
    Write(nullptr, "MARS_BUILD_TIME: " MARS_BUILD_TIME);
    Write(nullptr, "MARS_BUILD_JOB: " MARS_TAG);

    snprintf(logmsg, sizeof(logmsg), "log appender mode:%d, use mmap:%d", (int)config_.mode_, use_mmap);
    Write(nullptr, logmsg);

    if (!config_.cachedir_.empty()) {
        boost::filesystem::space_info info = boost::filesystem::space(config_.cachedir_);
        snprintf(logmsg,
                 sizeof(logmsg),
                 "cache dir space info, capacity:%" PRIuMAX " free:%" PRIuMAX " available:%" PRIuMAX,
                 info.capacity,
                 info.free,
                 info.available);
        Write(nullptr, logmsg);
    }

    boost::filesystem::space_info info = boost::filesystem::space(config_.logdir_);
    snprintf(logmsg,
             sizeof(logmsg),
             "log dir space info, capacity:%" PRIuMAX " free:%" PRIuMAX " available:%" PRIuMAX,
             info.capacity,
             info.free,
             info.available);
    Write(nullptr, logmsg);
}

std::string XloggerAppender::__MakeLogFileNamePrefix(const timeval& _tv, const char* _prefix) {
    time_t sec = _tv.tv_sec;
    tm tcur = *localtime((const time_t*)&sec);

    char temp[64] = {0};
    snprintf(temp, 64, "_%d%02d%02d", 1900 + tcur.tm_year, 1 + tcur.tm_mon, tcur.tm_mday);

    std::string filenameprefix = _prefix;
    filenameprefix += temp;

    return filenameprefix;
}

void XloggerAppender::__GetFileNamesByPrefix(const std::string& _logdir,
                                             const std::string& _fileprefix,
                                             const std::string& _fileext,
                                             std::vector<std::string>& _filename_vec) {
    boost::filesystem::path path(_logdir);
    if (!boost::filesystem::is_directory(path)) {
        return;
    }

    boost::filesystem::directory_iterator end_iter;
    std::string filename;

    for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter) {
        if (boost::filesystem::is_regular_file(iter->status())) {
            filename = iter->path().filename().string();
            if (strutil::StartsWith(filename, _fileprefix) && strutil::EndsWith(filename, _fileext)) {
                _filename_vec.push_back(filename);
            }
        }
    }
}

void XloggerAppender::__GetFilePathsFromTimeval(const timeval& _tv,
                                                const std::string& _logdir,
                                                const char* _prefix,
                                                const std::string& _fileext,
                                                std::vector<std::string>& _filepath_vec) {
    std::string fileprefix = __MakeLogFileNamePrefix(_tv, _prefix);
    std::vector<std::string> filename_vec;
    __GetFileNamesByPrefix(_logdir, fileprefix, _fileext, filename_vec);

    for (std::vector<std::string>::iterator iter = filename_vec.begin(); iter != filename_vec.end(); ++iter) {
        _filepath_vec.push_back(_logdir + "/" + (*iter));
    }
}

static bool __string_compare_greater(const std::string& s1, const std::string& s2) {
    if (s1.length() == s2.length()) {
        return s1 > s2;
    }
    return s1.length() > s2.length();
}

long XloggerAppender::__GetNextFileIndex(const std::string& _fileprefix, const std::string& _fileext) {
    std::vector<std::string> filename_vec;
    __GetFileNamesByPrefix(config_.logdir_, _fileprefix, _fileext, filename_vec);
    if (!config_.cachedir_.empty()) {
        __GetFileNamesByPrefix(config_.cachedir_, _fileprefix, _fileext, filename_vec);
    }

    long index = 0;  // long is enought to hold all indexes in one day.
    if (filename_vec.empty()) {
        return index;
    }
    // high -> low
    std::sort(filename_vec.begin(), filename_vec.end(), __string_compare_greater);
    std::string last_filename = *(filename_vec.begin());
    std::size_t ext_pos = last_filename.rfind("." + _fileext);
    std::size_t index_len = ext_pos - _fileprefix.length();
    if (index_len > 0) {
        std::string index_str = last_filename.substr(_fileprefix.length(), index_len);
        if (strutil::StartsWith(index_str, "_")) {
            index_str = index_str.substr(1);
        }
        index = atol(index_str.c_str());
    }

    uint64_t filesize = 0;
    std::string logfilepath = config_.logdir_ + "/" + last_filename;
    if (boost::filesystem::exists(logfilepath)) {
        filesize += boost::filesystem::file_size(logfilepath);
    }
    if (!config_.cachedir_.empty()) {
        logfilepath = config_.cachedir_ + "/" + last_filename;
        if (boost::filesystem::exists(logfilepath)) {
            filesize += boost::filesystem::file_size(logfilepath);
        }
    }
    return (filesize > max_file_size_) ? index + 1 : index;
}

void XloggerAppender::__MakeLogFileName(const timeval& _tv,
                                        const std::string& _logdir,
                                        const char* _prefix,
                                        const std::string& _fileext,
                                        char* _filepath,
                                        unsigned int _len) {
    long index = 0;
    std::string logfilenameprefix = __MakeLogFileNamePrefix(_tv, _prefix);
    if (max_file_size_ > 0) {
        index = __GetNextFileIndex(logfilenameprefix, _fileext);
    }

    std::string logfilepath = _logdir;
    logfilepath += "/";
    logfilepath += logfilenameprefix;

    if (index > 0) {
        char temp[24] = {0};
        snprintf(temp, 24, "_%ld", index);
        logfilepath += temp;
    }

    logfilepath += ".";
    logfilepath += _fileext;

    strncpy(_filepath, logfilepath.c_str(), _len - 1);
    _filepath[_len - 1] = '\0';
}

void XloggerAppender::__DelTimeoutFile(const std::string& _log_path) {
    ScopedLock dir_attr_lock(sg_mutex_dir_attr);
    time_t now_time = time(nullptr);

    boost::filesystem::path path(_log_path);

    if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
        boost::filesystem::directory_iterator end_iter;
        for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter) {
            time_t file_modify_time = boost::filesystem::last_write_time(iter->path());

            if (now_time > file_modify_time && now_time - file_modify_time > max_alive_time_) {
                if (boost::filesystem::is_regular_file(iter->status())
                    && iter->path().extension() == (std::string(".") + LOG_EXT)) {
                    boost::filesystem::remove(iter->path());
                }
                if (boost::filesystem::is_directory(iter->status())) {
                    std::string filename = iter->path().filename().string();
                    if (filename.size() == 8 && filename.find_first_not_of("0123456789") == std::string::npos) {
                        boost::filesystem::remove_all(iter->path());
                    }
                }
            }
        }
    }
}

bool XloggerAppender::__AppendFile(const std::string& _src_file, const std::string& _dst_file) {
    if (_src_file == _dst_file) {
        return false;
    }

    if (!boost::filesystem::exists(_src_file)) {
        return false;
    }

    if (0 == boost::filesystem::file_size(_src_file)) {
        return true;
    }

    FILE* src_file = fopen(_src_file.c_str(), "rb");

    if (nullptr == src_file) {
        return false;
    }

    FILE* dest_file = fopen(_dst_file.c_str(), "ab");

    if (nullptr == dest_file) {
        fclose(src_file);
        return false;
    }

    fseek(src_file, 0, SEEK_END);
    long src_file_len = ftell(src_file);
    long dst_file_len = ftell(dest_file);
    fseek(src_file, 0, SEEK_SET);

    char buffer[4096] = {0};

    while (true) {
        if (feof(src_file))
            break;

        size_t read_ret = fread(buffer, 1, sizeof(buffer), src_file);

        if (read_ret == 0)
            break;

        if (ferror(src_file))
            break;

        fwrite(buffer, 1, read_ret, dest_file);

        if (ferror(dest_file))
            break;
    }

    if (dst_file_len + src_file_len > ftell(dest_file)) {
        ftruncate(fileno(dest_file), dst_file_len);
        fclose(src_file);
        fclose(dest_file);
        return false;
    }

    fclose(src_file);
    fclose(dest_file);

    return true;
}

void XloggerAppender::__MoveOldFiles(const std::string& _src_path,
                                     const std::string& _dest_path,
                                     const std::string& _nameprefix) {
    ScopedLock dir_attr_lock(sg_mutex_dir_attr);
    if (_src_path == _dest_path) {
        return;
    }

    boost::filesystem::path path(_src_path);
    if (!boost::filesystem::is_directory(path)) {
        return;
    }

    ScopedLock lock_file(mutex_log_file_);
    time_t now_time = time(nullptr);

    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter) {
        if (!strutil::StartsWith(iter->path().filename().string(), _nameprefix)
            || !strutil::EndsWith(iter->path().string(), LOG_EXT)) {
            continue;
        }

        if (config_.cache_days_ > 0) {
            time_t file_modify_time = boost::filesystem::last_write_time(iter->path());
            if (now_time > file_modify_time && (now_time - file_modify_time) < config_.cache_days_ * 24 * 60 * 60) {
                continue;
            }
        }

        if (!__AppendFile(iter->path().string(), config_.logdir_ + "/" + iter->path().filename().string())) {
            break;
        }

        boost::filesystem::remove(iter->path());
    }
}

void XloggerAppender::__GetMarkInfo(char* _info, size_t _info_len) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t sec = tv.tv_sec;
    struct tm tm_tmp = *localtime((const time_t*)&sec);
    char tmp_time[64] = {0};
    strftime(tmp_time, sizeof(tmp_time), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);
    snprintf(_info, _info_len, "[%" PRIdMAX ",%" PRIdMAX "][%s]", xlogger_pid(), xlogger_tid(), tmp_time);
}

void XloggerAppender::__WriteTips2Console(const char* _tips_format, ...) {
    if (nullptr == _tips_format) {
        return;
    }

    XLoggerInfo info = XLOGGER_INFO_INITIALIZER;
    info.level = kLevelError;

    char tips_info[4096] = {0};
    va_list ap;
    va_start(ap, _tips_format);
    vsnprintf(tips_info, sizeof(tips_info), _tips_format, ap);
    va_end(ap);
    ConsoleLog(&info, tips_info);
}

bool XloggerAppender::__WriteFile(const void* _data, size_t _len, FILE* _file) {
    if (nullptr == _file) {
        assert(false);
        return false;
    }

    long before_len = ftell(_file);
    if (before_len < 0)
        return false;

    if (1 != fwrite(_data, _len, 1, _file)) {
        int err = ferror(_file);

        __WriteTips2Console("write file error:%d", err);

        ftruncate(fileno(_file), before_len);
        fseek(_file, 0, SEEK_END);

        char err_log[256] = {0};
        snprintf(err_log, sizeof(err_log), "\nwrite file error:%d\n", err);

        AutoBuffer tmp_buff;
        log_buff_->Write(err_log, strnlen(err_log, sizeof(err_log)), tmp_buff);

        fwrite(tmp_buff.Ptr(), tmp_buff.Length(), 1, _file);

        return false;
    }

    return true;
}

bool XloggerAppender::__OpenLogFile(const std::string& _log_dir) {
    if (config_.logdir_.empty())
        return false;

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    if (nullptr != logfile_) {
        time_t sec = tv.tv_sec;
        tm tcur = *localtime((const time_t*)&sec);
        tm filetm = *localtime(&openfiletime_);

        if (filetm.tm_year == tcur.tm_year && filetm.tm_mon == tcur.tm_mon && filetm.tm_mday == tcur.tm_mday) {
            return true;
        }

        fclose(logfile_);
        logfile_ = nullptr;
    }

    uint64_t now_tick = gettickcount();
    time_t now_time = tv.tv_sec;

    openfiletime_ = tv.tv_sec;

    char logfilepath[1024] = {0};
    __MakeLogFileName(tv, _log_dir, config_.nameprefix_.c_str(), LOG_EXT, logfilepath, 1024);

    if (now_time < last_time_) {
        logfile_ = fopen(last_file_path_, "ab");

        if (nullptr == logfile_) {
            __WriteTips2Console("open file error:%d %s, path:%s", errno, strerror(errno), last_file_path_);
        }

#ifdef __APPLE__
        assert(logfile_);
#endif
        return nullptr != logfile_;
    }

    logfile_ = fopen(logfilepath, "ab");

    if (nullptr == logfile_) {
        __WriteTips2Console("open file error:%d %s, path:%s", errno, strerror(errno), logfilepath);
    }

    if (0 != last_time_ && (now_time - last_time_) > (time_t)((now_tick - last_tick_) / 1000 + 300)) {
        struct tm tm_tmp = *localtime((const time_t*)&last_time_);
        char last_time_str[64] = {0};
        strftime(last_time_str, sizeof(last_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);

        tm_tmp = *localtime((const time_t*)&now_time);
        char now_time_str[64] = {0};
        strftime(now_time_str, sizeof(now_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);

        char log[1024] = {0};
        snprintf(log,
                 sizeof(log),
                 "[F][ last log file:%s from %s to %s, time_diff:%ld, tick_diff:%" PRIu64 "\n",
                 last_file_path_,
                 last_time_str,
                 now_time_str,
                 now_time - last_time_,
                 now_tick - last_tick_);

        AutoBuffer tmp_buff;
        log_buff_->Write(log, strnlen(log, sizeof(log)), tmp_buff);
        __WriteFile(tmp_buff.Ptr(), tmp_buff.Length(), logfile_);
    }

    memcpy(last_file_path_, logfilepath, sizeof(last_file_path_));
    last_tick_ = now_tick;
    last_time_ = now_time;

#ifdef __APPLE__
    assert(logfile_);
#endif
    return nullptr != logfile_;
}

void XloggerAppender::__CloseLogFile() {
    if (nullptr == logfile_)
        return;

    openfiletime_ = 0;
    fclose(logfile_);
    logfile_ = nullptr;
}

bool XloggerAppender::__CacheLogs() {
    if (config_.cachedir_.empty() || config_.cache_days_ <= 0) {
        return false;
    }

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    char logfilepath[1024] = {0};
    __MakeLogFileName(tv, config_.logdir_, config_.nameprefix_.c_str(), LOG_EXT, logfilepath, 1024);
    if (boost::filesystem::exists(logfilepath)) {
        return false;
    }

    static const uintmax_t kAvailableSizeThreshold = (uintmax_t)1 * 1024 * 1024 * 1024;  // 1G
    boost::filesystem::space_info info = boost::filesystem::space(config_.cachedir_);
    if (info.available < kAvailableSizeThreshold) {
        return false;
    }

    return true;
}

void XloggerAppender::__Log2File(const void* _data, size_t _len, bool _move_file) {
    if (nullptr == _data || 0 == _len || config_.logdir_.empty()) {
        return;
    }

    ScopedLock lock_file(mutex_log_file_);

    if (config_.cachedir_.empty()) {
        if (__OpenLogFile(config_.logdir_)) {
            __WriteFile(_data, _len, logfile_);
            if (kAppenderAsync == config_.mode_) {
                __CloseLogFile();
            }
        }
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    char logcachefilepath[1024] = {0};

    __MakeLogFileName(tv, config_.cachedir_, config_.nameprefix_.c_str(), LOG_EXT, logcachefilepath, 1024);

    bool cache_logs = __CacheLogs();
    if ((cache_logs || boost::filesystem::exists(logcachefilepath)) && __OpenLogFile(config_.cachedir_)) {
        __WriteFile(_data, _len, logfile_);
        if (kAppenderAsync == config_.mode_) {
            __CloseLogFile();
        }

        if (cache_logs || !_move_file) {
            return;
        }

        char logfilepath[1024] = {0};
        __MakeLogFileName(tv, config_.logdir_, config_.nameprefix_.c_str(), LOG_EXT, logfilepath, 1024);
        if (__AppendFile(logcachefilepath, logfilepath)) {
            if (kAppenderSync == config_.mode_) {
                __CloseLogFile();
            }
            boost::filesystem::remove(logcachefilepath);
        }
        return;
    }

    bool write_success = false;
    bool open_success = __OpenLogFile(config_.logdir_);
    if (open_success) {
        write_success = __WriteFile(_data, _len, logfile_);
        if (kAppenderAsync == config_.mode_) {
            __CloseLogFile();
        }
    }

    if (!write_success) {
        if (open_success && kAppenderSync == config_.mode_) {
            __CloseLogFile();
        }

        if (__OpenLogFile(config_.cachedir_)) {
            __WriteFile(_data, _len, logfile_);
            if (kAppenderAsync == config_.mode_) {
                __CloseLogFile();
            }
        }
    }
}

void XloggerAppender::WriteTips2File(const char* _tips_format, ...) {
    if (nullptr == _tips_format) {
        return;
    }

    char tips_info[4096] = {0};
    va_list ap;
    va_start(ap, _tips_format);
    vsnprintf(tips_info, sizeof(tips_info), _tips_format, ap);
    va_end(ap);

    AutoBuffer tmp_buff;
    log_buff_->Write(tips_info, strnlen(tips_info, sizeof(tips_info)), tmp_buff);

    __Log2File(tmp_buff.Ptr(), tmp_buff.Length(), false);
}

void XloggerAppender::__AsyncLogThread() {
    while (true) {
        ScopedLock lock_buffer(mutex_buffer_async_);

        if (nullptr == log_buff_)
            break;

        AutoBuffer tmp;
        log_buff_->Flush(tmp);
        lock_buffer.unlock();

        if (nullptr != tmp.Ptr())
            __Log2File(tmp.Ptr(), tmp.Length(), true);

        if (log_close_)
            break;

        cond_buffer_async_.wait(15 * 60 * 1000);
    }
}

void XloggerAppender::__WriteSync(const XLoggerInfo* _info, const char* _log) {
    char temp[16 * 1024] = {0};  // tell perry,ray if you want modify size.
    PtrBuffer log(temp, 0, sizeof(temp));
    log_formater(_info, _log, log);

    AutoBuffer tmp_buff;
    if (!log_buff_->Write(log.Ptr(), log.Length(), tmp_buff))
        return;

    __Log2File(tmp_buff.Ptr(), tmp_buff.Length(), false);
}

void XloggerAppender::__WriteAsync(const XLoggerInfo* _info, const char* _log) {
    char temp[16 * 1024] = {0};  // tell perry,ray if you want modify size.
    PtrBuffer log_buff(temp, 0, sizeof(temp));
    log_formater(_info, _log, log_buff);

    ScopedLock lock(mutex_buffer_async_);
    if (nullptr == log_buff_)
        return;

    if (log_buff_->GetData().Length() >= kBufferBlockLength * 4 / 5) {
        int ret = snprintf(temp,
                           sizeof(temp),
                           "[F][ sg_buffer_async.Length() >= BUFFER_BLOCK_LENTH*4/5, len: %d\n",
                           (int)log_buff_->GetData().Length());
        log_buff.Length(ret, ret);
    }

    if (!log_buff_->Write(log_buff.Ptr(), (unsigned int)log_buff.Length()))
        return;

    if (log_buff_->GetData().Length() >= kBufferBlockLength * 1 / 3
        || (nullptr != _info && kLevelFatal == _info->level)) {
        cond_buffer_async_.notifyAll();
    }
}

#define HEX_STRING "0123456789abcdef"
static unsigned int to_string(const void* signature, int len, char* str) {
    char* str_p = str;
    const unsigned char* sig_p;

    for (sig_p = (const unsigned char*)signature; sig_p - (const unsigned char*)signature < len; sig_p++) {
        char high, low;
        high = *sig_p / 16;
        low = *sig_p % 16;

        *str_p++ = HEX_STRING[(unsigned char)high];
        *str_p++ = HEX_STRING[(unsigned char)low];
        *str_p++ = ' ';
    }

    *str_p++ = '\n';

    for (sig_p = (const unsigned char*)signature; sig_p - (const unsigned char*)signature < len; sig_p++) {
        *str_p++ = char(isgraph(*sig_p) ? *sig_p : ' ');
        *str_p++ = ' ';
        *str_p++ = ' ';
    }

    return (unsigned int)(str_p - str);
}

const char* XloggerAppender::Dump(const void* _dumpbuffer, size_t _len) {
    if (nullptr == _dumpbuffer || 0 == _len) {
        return "";
    }

    if (config_.logdir_.empty()) {
        return "";
    }

    SCOPE_ERRNO();

    thread_local std::string buffer;
    if (!buffer.empty()) {
        buffer.clear();
    }

    struct timeval tv = {0};
    gettimeofday(&tv, nullptr);
    time_t sec = tv.tv_sec;
    tm tcur = *localtime((const time_t*)&sec);

    char forder_name[128] = {0};
    snprintf(forder_name, sizeof(forder_name), "%d%02d%02d", 1900 + tcur.tm_year, 1 + tcur.tm_mon, tcur.tm_mday);

    std::string filepath = config_.logdir_ + "/" + forder_name + "/";

    if (!boost::filesystem::exists(filepath))
        boost::filesystem::create_directory(filepath);

    char file_name[128] = {0};
    snprintf(file_name,
             sizeof(file_name),
             "%d%02d%02d%02d%02d%02d_%d.dump",
             1900 + tcur.tm_year,
             1 + tcur.tm_mon,
             tcur.tm_mday,
             tcur.tm_hour,
             tcur.tm_min,
             tcur.tm_sec,
             (int)_len);
    filepath += file_name;

    FILE* fileid = fopen(filepath.c_str(), "wb");

    if (nullptr == fileid) {
        ASSERT2(nullptr != fileid, "%s, errno:(%d, %s)", filepath.c_str(), errno, strerror(errno));
        return "";
    }

    fwrite(_dumpbuffer, _len, 1, fileid);
    fclose(fileid);

    buffer += "\n dump file to ";
    buffer += filepath + " :\n";

    int dump_len = 0;

    for (int x = 0; x < 32 && dump_len < (int)_len; ++x) {
        char dump_log[128] = {0};
        to_string((const char*)_dumpbuffer + dump_len, std::min(int(_len) - dump_len, 16), dump_log);
        dump_len += std::min((int)_len - dump_len, 16);
        buffer += dump_log;
        buffer += "\n";
    }

    return buffer.c_str();
}

static int calc_dump_required_length(int srcbytes) {
    // MUST CHANGE THIS IF YOU CHANGE `to_string` function.
    return srcbytes * 6 + 1;
}

bool XloggerAppender::GetCurrentLogPath(char* _log_path, unsigned int _len) {
    if (nullptr == _log_path || 0 == _len)
        return false;

    if (config_.logdir_.empty())
        return false;
    strncpy(_log_path, config_.logdir_.c_str(), _len - 1);
    _log_path[_len - 1] = '\0';
    return true;
}

bool XloggerAppender::GetCurrentLogCachePath(char* _logPath, unsigned int _len) {
    if (nullptr == _logPath || 0 == _len)
        return false;

    if (config_.cachedir_.empty())
        return false;
    strncpy(_logPath, config_.cachedir_.c_str(), _len - 1);
    _logPath[_len - 1] = '\0';
    return true;
}

void XloggerAppender::SetConsoleLog(bool _is_open) {
    consolelog_open_ = _is_open;
}

void XloggerAppender::SetMaxFileSize(uint64_t _max_byte_size) {
    max_file_size_ = _max_byte_size;
}

void XloggerAppender::SetMaxAliveDuration(long _max_time) {
    if (_max_time >= kMinLogAliveTime) {
        max_alive_time_ = _max_time;
    }
}

bool XloggerAppender::GetfilepathFromTimespan(int _timespan,
                                              const char* _prefix,
                                              std::vector<std::string>& _filepath_vec) {
    if (config_.logdir_.empty())
        return false;

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tv.tv_sec -= _timespan * (24 * 60 * 60);

    __GetFilePathsFromTimeval(tv, config_.logdir_, _prefix, LOG_EXT, _filepath_vec);
    if (!config_.cachedir_.empty()) {
        __GetFilePathsFromTimeval(tv, config_.cachedir_, _prefix, LOG_EXT, _filepath_vec);
    }
    return true;
}

bool XloggerAppender::MakeLogfileName(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec) {
    if (config_.logdir_.empty())
        return false;

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tv.tv_sec -= _timespan * (24 * 60 * 60);

    char log_path[2048] = {0};
    __MakeLogFileName(tv, config_.logdir_, _prefix, LOG_EXT, log_path, sizeof(log_path));

    if (config_.cachedir_.empty()) {
        _filepath_vec.push_back(log_path);
        return true;
    }

    char cache_log_path[2048] = {0};
    __MakeLogFileName(tv, config_.cachedir_, _prefix, LOG_EXT, cache_log_path, sizeof(cache_log_path));

    if (boost::filesystem::exists(log_path)) {
        _filepath_vec.push_back(log_path);
    }

    if (boost::filesystem::exists(cache_log_path)) {
        _filepath_vec.push_back(cache_log_path);
    }

    if (!boost::filesystem::exists(log_path) && !boost::filesystem::exists(cache_log_path)) {
        _filepath_vec.push_back(log_path);
    }

    return true;
}

void XloggerAppender::TreatMappingAsFileAndFlush(TFileIOAction* _result) {
#ifdef WIN32
    // here we make sure boost convert path is utf8 to wide on windows
    SetConvertToUseUtf8();
#endif
    char mmap_file_path[512] = {0};
    snprintf(mmap_file_path,
             sizeof(mmap_file_path),
             "%s/%s.mmap3",
             config_.cachedir_.empty() ? config_.logdir_.c_str() : config_.cachedir_.c_str(),
             config_.nameprefix_.c_str());

    // exist
    if (!boost::filesystem::exists(mmap_file_path)) {
        if (_result)
            *_result = kActionUnnecessary;
        return;
    }

    // read buffer
    FILE* treat_mapping_as_file = fopen(mmap_file_path, "rb");
    if (!treat_mapping_as_file) {
        if (_result)
            *_result = kActionOpenFailed;
        return;
    }

    std::unique_ptr<char[]> data(new char[kBufferBlockLength]());
    size_t bytes_read = fread(data.get(), 1, kBufferBlockLength, treat_mapping_as_file);
    if (kBufferBlockLength != bytes_read || ferror(treat_mapping_as_file)) {
        if (_result)
            *_result = kActionReadFailed;
        fclose(treat_mapping_as_file);
        return;
    }
    fclose(treat_mapping_as_file);

    // init and set flag
    if (config_.compress_mode_ == kZstd) {
        log_buff_ = new LogZstdBuffer(data.release(),
                                      kBufferBlockLength,
                                      true,
                                      config_.pub_key_.c_str(),
                                      config_.compress_level_);
    } else {
        log_buff_ = new LogZlibBuffer(data.release(), kBufferBlockLength, true, config_.pub_key_.c_str());
    }
	
    log_close_ = false;

    // try write mapping to logfile
    AutoBuffer buffer;
    log_buff_->Flush(buffer);

    // invalid data or empty
    if (!buffer.Ptr() || !buffer.Length()) {
        if (_result)
            *_result = kActionUnnecessary;
        return;
    }

    char mark_info[512] = {0};
    __GetMarkInfo(mark_info, sizeof(mark_info));

    WriteTips2File("~~~~~ begin of mmap from other process ~~~~~\n");
    __Log2File(buffer.Ptr(), buffer.Length(), false);
    WriteTips2File("~~~~~ end of mmap from other process ~~~~~%s\n", mark_info);

    // clean mmaping
    if (!boost::filesystem::remove(mmap_file_path)) {
        if (_result)
            *_result = kActionRemoveFailed;
        return;
    }

    if (_result)
        *_result = kActionSuccess;
}

////////////////////////////////////////////////////////////////////////////////////
static XloggerAppender* sg_default_appender = nullptr;
static bool sg_release_guard = true;
static bool sg_default_console_log_open = false;
static Mutex sg_mutex;
static uint64_t sg_max_byte_size = 0;
static long sg_max_alive_time = 0;
void xlogger_appender(const XLoggerInfo* _info, const char* _log) {
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->Write(_info, _log);
}

static void appender_release_default_appender() {
    if (sg_release_guard) {
        return;
    }
    sg_release_guard = true;
    sg_default_appender->Close();
    //  本函数只会在 exit 的时候调用，所以干脆不释放对象了，防止多线程导致的 crash
    // XloggerAppender::Release(sg_default_appender);
}

void appender_open(const XLogConfig& _config) {
    assert(!_config.logdir_.empty());

    if (nullptr != sg_default_appender) {
        sg_default_appender->WriteTips2File("appender has already been opened. _dir:%s _nameprefix:%s",
                                            _config.logdir_.c_str(),
                                            _config.nameprefix_.c_str());
        return;
    }

    sg_default_appender = XloggerAppender::NewInstance(_config, sg_max_byte_size);
    sg_default_appender->SetConsoleLog(sg_default_console_log_open);
    if (sg_max_alive_time > 0) {
        sg_default_appender->SetMaxAliveDuration(sg_max_alive_time);
    }
    sg_release_guard = false;
    xlogger_SetAppender(&xlogger_appender);
    BOOT_RUN_EXIT(appender_release_default_appender);
}

void appender_flush() {
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->Flush();
}

void appender_flush_sync() {
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->FlushSync();
}

void appender_close() {
    ScopedLock lock(sg_mutex);
    if (sg_release_guard) {
        return;
    }
    sg_release_guard = true;
    sg_default_appender->Close();
    XloggerAppender::DelayRelease(sg_default_appender);
    sg_default_appender = nullptr;
}

void appender_setmode(TAppenderMode _mode) {
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->SetMode(_mode);
}

bool appender_get_current_log_path(char* _log_path, unsigned int _len) {
    if (sg_release_guard) {
        return false;
    }
    return sg_default_appender->GetCurrentLogPath(_log_path, _len);
}

bool appender_get_current_log_cache_path(char* _logPath, unsigned int _len) {
    if (sg_release_guard) {
        return false;
    }
    return sg_default_appender->GetCurrentLogCachePath(_logPath, _len);
}

void appender_set_console_log(bool _is_open) {
    sg_default_console_log_open = _is_open;
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->SetConsoleLog(_is_open);
}

void appender_set_max_file_size(uint64_t _max_byte_size) {
    sg_max_byte_size = _max_byte_size;
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->SetMaxFileSize(_max_byte_size);
}

void appender_set_max_alive_duration(long _max_time) {
    sg_max_alive_time = _max_time;
    if (sg_release_guard) {
        return;
    }
    sg_default_appender->SetMaxAliveDuration(_max_time);
}

bool appender_getfilepath_from_timespan(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec) {
    if (sg_release_guard) {
        return false;
    }
    return sg_default_appender->GetfilepathFromTimespan(_timespan, _prefix, _filepath_vec);
}

bool appender_make_logfile_name(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec) {
    if (sg_release_guard) {
        return false;
    }
    return sg_default_appender->MakeLogfileName(_timespan, _prefix, _filepath_vec);
}

void appender_oneshot_flush(const XLogConfig& _config, TFileIOAction* _result) {
    auto* scoped_appender = XloggerAppender::NewInstance(_config, sg_max_byte_size, true);
    scoped_appender->TreatMappingAsFileAndFlush(_result);
    scoped_appender->Close();
    XloggerAppender::DelayRelease(scoped_appender);
}

}  // namespace xlog
}  // namespace mars

using namespace mars::xlog;

const char* xlogger_dump(const void* _dumpbuffer, size_t _len) {
    if (sg_release_guard) {
        return "";
    }
    return sg_default_appender->Dump(_dumpbuffer, _len);
}

const char* xlogger_memory_dump(const void* _dumpbuffer, size_t _len) {
    if (NULL == _dumpbuffer || 0 == _len) {
        //        ASSERT(NULL!=_dumpbuffer);
        //        ASSERT(0!=_len);
        return "";
    }

    SCOPE_ERRNO();

    thread_local std::string buffer;
    if (!buffer.empty()) {
        buffer.clear();
    }

    const char* src_buffer = reinterpret_cast<const char*>(_dumpbuffer);
    buffer += "\n";
    buffer += std::to_string(_len) + " bytes:\n";

    int calc_dst_buffer_len = calc_dump_required_length(32) + 1;
    char* dst_buffer = new char[calc_dst_buffer_len];

    for (int src_offset = 0; src_offset < (int)_len && buffer.size() < kMaxDumpLength;) {
        int dst_leftbytes = kMaxDumpLength - buffer.size();
        int bytes = std::min((int)_len - src_offset, 32);

        while (bytes > 0 && calc_dump_required_length(bytes) >= dst_leftbytes) {
            --bytes;
        }
        if (bytes <= 0) {
            break;
        }

        memset(dst_buffer, 0, calc_dst_buffer_len);
        to_string(src_buffer + src_offset, bytes, dst_buffer);
        buffer += dst_buffer;

        src_offset += bytes;

        // next line
        buffer += "\n";
    }

    delete[] dst_buffer;
    return buffer.c_str();
}
