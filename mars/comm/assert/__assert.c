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

#include "comm/xlogger/xloggerbase.h"
#include "comm/compiler_util.h"

#ifdef ANDROID
#include "comm/android/callstack.h"
#endif

#ifndef XLOGGER_TAG
#define XLOGGER_TAG ""
#endif

#if defined(__APPLE__) && (defined(NDEBUG))
void __assert_rtn(const char *, const char *, int, const char *) __dead2;
#endif

#ifdef WIN32
#define __assert(__Expression, __FILE, __LINE) (void)((_assert(__Expression, __FILE, __LINE), 0) )
#define snprintf _snprintf
#endif

#ifdef DEBUG
static int sg_enable_assert = 1;
#else
static int sg_enable_assert = 0;
#endif

void ENABLE_ASSERT() { sg_enable_assert = 1;}
void DISABLE_ASSERT() { sg_enable_assert = 0; }
int IS_ASSERT_ENABLE() { return sg_enable_assert;}

EXPORT_FUNC void __ASSERT(const char * _pfile, int _line, const char * _pfunc, const char * _pexpression) {
    XLoggerInfo info= {0};
    char assertlog[4096] = {'\0'};
    int offset = 0;

    offset += snprintf(assertlog, sizeof(assertlog), "[ASSERT(%s)]", _pexpression);

#ifdef ANDROID
    android_callstack(assertlog+offset, sizeof(assertlog)-offset);
#endif

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
#if defined(ANDROID) //&& (defined(DEBUG))
        raise(SIGTRAP);
        __assert2(_pfile, _line, _pfunc, _pexpression);
#endif

#if defined(__APPLE__) //&& (defined(DEBUG))
         __assert_rtn(_pfunc, _pfile, _line, _pexpression);
#endif
    }
}

void __ASSERTV2(const char * _pfile, int _line, const char * _pfunc, const char * _pexpression, const char * _format, va_list _list) {
    char assertlog[4096] = {'\0'};
    XLoggerInfo info= {kLevelFatal};
    int offset = 0;

    offset += snprintf(assertlog, sizeof(assertlog), "[ASSERT(%s)]", _pexpression);
    offset += vsnprintf(assertlog+offset, sizeof(assertlog)-offset, _format, _list);

#ifdef ANDROID
    android_callstack(assertlog+offset, sizeof(assertlog)-offset);
#endif

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
#if defined(ANDROID) //&& (defined(DEBUG))
        raise(SIGTRAP);
        __assert2(_pfile, _line, _pfunc, _pexpression);
#endif

#if defined(__APPLE__) //&& (defined(DEBUG))
         __assert_rtn(_pfunc, _pfile, _line, _pexpression);
#endif

#if defined(WIN32) //&& (defined(DEBUG))
         __assert(_pexpression, _pfile, _line);
#endif
    }
}

void __ASSERT2(const char * _pfile, int _line, const char * _pfunc, const char * _pexpression, const char * _format, ...) {
    va_list valist;
    va_start(valist, _format);
    __ASSERTV2(_pfile, _line, _pfunc,  _pexpression,  _format, valist);
    va_end(valist);
}

