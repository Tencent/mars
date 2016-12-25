/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 ============================================================================
 ============================================================================
 */

#ifndef XLOGGERBASE_H_
#define XLOGGERBASE_H_

#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kLevelAll = 0,
    kLevelVerbose = 0,
    kLevelDebug,    // Detailed information on the flow through the system.
    kLevelInfo,     // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
    kLevelWarn,     // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
    kLevelError,    // Other runtime errors or unexpected conditions.
    kLevelFatal,    // Severe errors that cause premature termination.
    kLevelNone,     // Special level used to disable all log messages.
} TLogLevel;

typedef struct XLoggerInfo_t {
    TLogLevel level;
    const char* tag;
    const char* filename;
    const char* func_name;
    int line;

    struct timeval timeval;
    intmax_t pid;
    intmax_t tid;
    intmax_t maintid;
} XLoggerInfo;

extern intmax_t xlogger_pid();
extern intmax_t xlogger_tid();
extern intmax_t xlogger_maintid();
typedef void (*xlogger_appender_t)(const XLoggerInfo* _info, const char* _log);
extern const char* xlogger_dump(const void* _dumpbuffer, size_t _len);

TLogLevel   xlogger_Level();
void xlogger_SetLevel(TLogLevel _level);
int  xlogger_IsEnabledFor(TLogLevel _level);
xlogger_appender_t xlogger_SetAppender(xlogger_appender_t _appender);

// no level filter
#ifdef __GNUC__
__attribute__((__format__(printf, 3, 4)))
#endif
void        xlogger_AssertP(const XLoggerInfo* _info, const char* _expression, const char* _format, ...);
void        xlogger_Assert(const XLoggerInfo* _info, const char* _expression, const char* _log);
#ifdef __GNUC__
__attribute__((__format__(printf, 2, 0)))
#endif
void        xlogger_VPrint(const XLoggerInfo* _info, const char* _format, va_list _list);
#ifdef __GNUC__
__attribute__((__format__(printf, 2, 3)))
#endif
void        xlogger_Print(const XLoggerInfo* _info, const char* _format, ...);
void        xlogger_Write(const XLoggerInfo* _info, const char* _log);

#ifdef __cplusplus
}
#endif

#endif
