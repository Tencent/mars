/*
 * comm_assert.c
 *
 *  Created on: 2012-9-5
 *      Author: yerungui
 */

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "comm/compiler_util.h"
#include "comm/xlogger/xloggerbase.h"

#ifndef XLOGGER_TAG
#define XLOGGER_TAG ""
#endif

#if defined(__APPLE__) && (defined(NDEBUG))
void __assert_rtn(const char*, const char*, int, const char*) __dead2;
#endif

#ifdef WIN32
#define __assert(__Expression, __FILE, __LINE) (void)((_assert(__Expression, __FILE, __LINE), 0))
#define snprintf _snprintf
#endif

#ifndef NDEBUG
#define IS_ASSERT_ENABLE() 1
#else
#define IS_ASSERT_ENABLE() 0
#endif

void __ASSERT(const char* _pfile, int _line, const char* _pfunc, const char* _pexpression) {
    XLoggerInfo info = XLOGGER_INFO_INITIALIZER;
    char assertlog[4096] = {'\0'};
    snprintf(assertlog, sizeof(assertlog), "[ASSERT(%s)]", _pexpression);

    //#ifdef ANDROID
    //    android_callstack(assertlog+offset, sizeof(assertlog)-offset);
    //#endif

    info.level = kLevelFatal;
    info.tag = XLOGGER_TAG;
    info.filename = _pfile;
    info.func_name = _pfunc;
    info.line = _line;
    gettimeofday(&info.timeval, NULL);
    info.pid = xlogger_pid();
    info.tid = xlogger_tid();
    info.maintid = xlogger_maintid();

    xlogger_Write(&info, assertlog);

    if (IS_ASSERT_ENABLE()) {
#if defined(ANDROID)  //&& (defined(DEBUG))
        raise(SIGTRAP);
        __assert2(_pfile, _line, _pfunc, _pexpression);
#endif

#if defined(__APPLE__)  //&& (defined(DEBUG))
        __assert_rtn(_pfunc, _pfile, _line, _pexpression);
#endif
    }
}

void __ASSERTV2(const char* _pfile,
                int _line,
                const char* _pfunc,
                const char* _pexpression,
                const char* _format,
                va_list _list) {
    char assertlog[4096] = {'\0'};
    XLoggerInfo info = XLOGGER_INFO_INITIALIZER;
    info.level = kLevelFatal;

    do {
        int offset = snprintf(assertlog, sizeof(assertlog), "[ASSERT(%s)]", _pexpression);
        if (offset < 0) {
            strncpy(assertlog, "[ASSERT] FAILED!!!", sizeof(assertlog));
            break;
        }
        if ((size_t)offset >= sizeof(assertlog)) {
            break;
        }

        size_t leftbytes = sizeof(assertlog) - offset;
        int offset2 = vsnprintf(assertlog + offset, leftbytes, _format, _list);
        if (offset2 < 0) {
            strncat(assertlog + offset, "[ASSERT2] FAILED!!!", leftbytes);
            break;
        }
        if ((size_t)offset2 >= leftbytes) {
            break;
        }
        offset += offset2;
    } while (0);

    //#ifdef ANDROID
    //    android_callstack(assertlog+offset, sizeof(assertlog)-offset);
    //#endif

    info.level = kLevelFatal;
    info.tag = XLOGGER_TAG;
    info.filename = _pfile;
    info.func_name = _pfunc;
    info.line = _line;
    gettimeofday(&info.timeval, NULL);
    info.pid = xlogger_pid();
    info.tid = xlogger_tid();
    info.maintid = xlogger_maintid();

    xlogger_Write(&info, assertlog);

    if (IS_ASSERT_ENABLE()) {
#if defined(ANDROID)  //&& (defined(DEBUG))
        raise(SIGTRAP);
        __assert2(_pfile, _line, _pfunc, _pexpression);
#endif

#if defined(__APPLE__)  //&& (defined(DEBUG))
        __assert_rtn(_pfunc, _pfile, _line, _pexpression);
#endif

#if defined(WIN32)  //&& (defined(DEBUG))
        __assert(_pexpression, _pfile, _line);
#endif
    }
}

void __ASSERT2(const char* _pfile, int _line, const char* _pfunc, const char* _pexpression, const char* _format, ...) {
    va_list valist;
    va_start(valist, _format);
    __ASSERTV2(_pfile, _line, _pfunc, _pexpression, _format, valist);
    va_end(valist);
}
