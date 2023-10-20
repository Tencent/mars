#include "boost/iostreams/device/mapped_file.hpp"
#include "mars/comm/thread/condition.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/xlog/appender.h"

namespace mars {
namespace xlog {

class LogBaseBuffer;
class XloggerAppender {
 public:
    static XloggerAppender* NewInstance(const XLogConfig& _config, uint64_t _max_byte_size);
    static XloggerAppender* NewInstance(const XLogConfig& _config, uint64_t _max_byte_size, bool _one_shot);
    static void DelayRelease(XloggerAppender* _appender);
    static void Release(XloggerAppender*& _appender);

    void Open(const XLogConfig& _config);

    void Write(const XLoggerInfo* _info, const char* _log);
    void SetMode(TAppenderMode _mode);
    void Flush();
    void FlushSync();
    void Close();
    void WriteTips2File(const char* _tips_format, ...);
    const char* Dump(const void* _dumpbuffer, size_t _len);

    bool GetCurrentLogPath(char* _log_path, unsigned int _len);
    bool GetCurrentLogCachePath(char* _logPath, unsigned int _len);
    void SetConsoleLog(bool _is_open);
    void SetMaxFileSize(uint64_t _max_byte_size);
    void SetMaxAliveDuration(long _max_time);
    bool GetfilepathFromTimespan(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);
    bool MakeLogfileName(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);

    void TreatMappingAsFileAndFlush(TFileIOAction* _result);

 private:
    XloggerAppender(const XLogConfig& _config, uint64_t _max_byte_size);
    XloggerAppender(const XLogConfig& _config, uint64_t _max_byte_size, bool _one_shot);

    std::string __MakeLogFileNamePrefix(const timeval& _tv, const char* _prefix);
    void __GetFileNamesByPrefix(const std::string& _logdir,
                                const std::string& _fileprefix,
                                const std::string& _fileext,
                                std::vector<std::string>& _filename_vec);
    void __GetFilePathsFromTimeval(const timeval& _tv,
                                   const std::string& _logdir,
                                   const char* _prefix,
                                   const std::string& _fileext,
                                   std::vector<std::string>& _filepath_vec);
    long __GetNextFileIndex(const std::string& _fileprefix, const std::string& _fileext);
    void __MakeLogFileName(const timeval& _tv,
                           const std::string& _logdir,
                           const char* _prefix,
                           const std::string& _fileext,
                           char* _filepath,
                           unsigned int _len);

    void __GetMarkInfo(char* _info, size_t _info_len);
    void __WriteTips2Console(const char* _tips_format, ...);
    bool __WriteFile(const void* _data, size_t _len, FILE* _file);
    bool __OpenLogFile(const std::string& _log_dir);
    void __CloseLogFile();
    bool __CacheLogs();
    void __Log2File(const void* _data, size_t _len, bool _move_file);
    void __AsyncLogThread();
    void __WriteSync(const XLoggerInfo* _info, const char* _log);
    void __WriteAsync(const XLoggerInfo* _info, const char* _log);
    void __DelTimeoutFile(const std::string& _log_path);
    bool __AppendFile(const std::string& _src_file, const std::string& _dst_file);
    void __MoveOldFiles(const std::string& _src_path, const std::string& _dest_path, const std::string& _nameprefix);

 private:
    XLogConfig config_;
    LogBaseBuffer* log_buff_ = nullptr;
    boost::iostreams::mapped_file mmap_file_;
    comm::Thread thread_async_;
    comm::Mutex mutex_buffer_async_;
    comm::Mutex mutex_log_file_;
    FILE* logfile_ = nullptr;
    time_t openfiletime_ = 0;
#ifdef DEBUG
    bool consolelog_open_ = true;
#else
    bool consolelog_open_ = false;
#endif
    bool log_close_ = true;
    comm::Condition cond_buffer_async_;
    uint64_t max_file_size_ = 0;               // 0, will not split log file.
    long max_alive_time_ = 10 * 24 * 60 * 60;  // 10 days in second

    time_t last_time_ = 0;
    uint64_t last_tick_ = 0;
    char last_file_path_[1024] = {0};

    std::unique_ptr<comm::Thread> thread_timeout_cache_;
    std::unique_ptr<comm::Thread> thread_moveold_;
    std::unique_ptr<comm::Thread> thread_timeout_log_;
};

}  // namespace xlog
}  // namespace mars
