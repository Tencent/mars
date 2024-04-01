
#ifndef MARS_COMM_XLOGGER_CATEGORY_H_
#define MARS_COMM_XLOGGER_CATEGORY_H_

#include <functional>

#include "xloggerbase.h"

namespace mars {
namespace comm {

class XloggerCategory {
 public:
    static XloggerCategory* NewInstance(uintptr_t _appender,
                                        std::function<void(const XLoggerInfo* _info, const char* _log)> _appender_func);
    static void DelayRelease(XloggerCategory* _category);

 private:
    XloggerCategory(uintptr_t _appender,
                    std::function<void(const XLoggerInfo* _info, const char* _log)> _appender_func);
    static void __Release(XloggerCategory* _category);

 public:
    intptr_t GetAppender();
    TLogLevel GetLevel();
    void SetLevel(TLogLevel _level);
    bool IsEnabledFor(TLogLevel _level);
    void VPrint(const XLoggerInfo* _info, const char* _format, va_list _list);
    void Print(const XLoggerInfo* _info, const char* _format, ...);
    void Write(const XLoggerInfo* _info, const char* _log);

 private:
    void __WriteImpl(const XLoggerInfo* _info, const char* _log);

 private:
    TLogLevel level_ = kLevelNone;
    uintptr_t appender_ = 0;
    std::function<void(const XLoggerInfo* _info, const char* _log)> appender_func_ = nullptr;
};

}  // namespace comm
}  // namespace mars

#endif  // MARS_COMM_XLOGGER_CATEGORY_H_
