/*
 ============================================================================
 Name        : xloggerbase.c
 ============================================================================
 */

#include "comm/xlogger/xloggerbase.h"
#include <stdio.h>

#include "comm/compiler_util.h"

WEAK_FUNC  TLogLevel   __xlogger_Level_impl();
WEAK_FUNC  void        __xlogger_SetLevel_impl(TLogLevel _level);
WEAK_FUNC  int         __xlogger_IsEnabledFor_impl(TLogLevel _level);
WEAK_FUNC xlogger_appender_t __xlogger_SetAppender_impl(xlogger_appender_t _appender);
WEAK_FUNC void __xlogger_Write_impl(const XLoggerInfo* _info, const char* _log);
WEAK_FUNC void __xlogger_VPrint_impl(const XLoggerInfo* _info, const char* _format, va_list _list);

WEAK_FUNC void __xlogger_AssertP_impl(const XLoggerInfo* _info, const char* _expression, const char* _format, va_list _list);
WEAK_FUNC void __xlogger_Assert_impl(const XLoggerInfo* _info, const char* _expression, const char* _log);

#ifndef WIN32
WEAK_FUNC const char* xlogger_dump(const void* _dumpbuffer, size_t _len) { return "";}
#endif

TLogLevel   xlogger_Level() {
    if (NULL == &__xlogger_Level_impl)  return kLevelNone;
	return __xlogger_Level_impl();
}

void xlogger_SetLevel(TLogLevel _level){
    if (NULL != &__xlogger_SetLevel_impl)
        __xlogger_SetLevel_impl(_level);
}

int  xlogger_IsEnabledFor(TLogLevel _level) {
    if (NULL == &__xlogger_IsEnabledFor_impl) { return 0;}
	return __xlogger_IsEnabledFor_impl(_level);

}

xlogger_appender_t xlogger_SetAppender(xlogger_appender_t _appender) {
    if (NULL == &__xlogger_SetAppender_impl) { return NULL;}
    return __xlogger_SetAppender_impl(_appender);
}

void xlogger_Write(const XLoggerInfo* _info, const char* _log) {
	if (NULL != &__xlogger_Write_impl)
		__xlogger_Write_impl(_info, _log);
}

void xlogger_VPrint(const XLoggerInfo* _info, const char* _format, va_list _list) {
	if (NULL != &__xlogger_VPrint_impl)
		__xlogger_VPrint_impl(_info, _format, _list);
}

void xlogger_Print(const XLoggerInfo* _info, const char* _format, ...) {
	if (NULL == &__xlogger_VPrint_impl){ return; }
    
	va_list valist;
	va_start(valist, _format);
    __xlogger_VPrint_impl(_info, _format, valist);
	va_end(valist);
}


void xlogger_AssertP(const XLoggerInfo* _info, const char* _expression, const char* _format, ...) {
	if (NULL == &__xlogger_AssertP_impl)  { return; }
    
	va_list valist;
	va_start(valist, _format);
	__xlogger_AssertP_impl(_info, _expression, _format, valist);
	va_end(valist);
}

void xlogger_Assert(const XLoggerInfo* _info, const char* _expression, const char* _log) {
    if (NULL != &__xlogger_Assert_impl)
    	__xlogger_Assert_impl(_info, _expression, _log);
}


#ifndef USING_XLOG_WEAK_FUNC
static TLogLevel gs_level = kLevelNone;
static xlogger_appender_t gs_appender = NULL;

TLogLevel   __xlogger_Level_impl() {return gs_level;}
void        __xlogger_SetLevel_impl(TLogLevel _level){ gs_level = _level;}
int         __xlogger_IsEnabledFor_impl(TLogLevel _level) {return gs_level <= _level;}

xlogger_appender_t __xlogger_SetAppender_impl(xlogger_appender_t _appender)  {
    xlogger_appender_t old_appender = gs_appender;
    gs_appender = _appender;
    return old_appender;
}

void __xlogger_Write_impl(const XLoggerInfo* _info, const char* _log) {
    
    if (!gs_appender) return;
    
    if (_info && -1==_info->pid && -1==_info->tid && -1==_info->maintid)
    {
        XLoggerInfo* info = (XLoggerInfo*)_info;
        info->pid = xlogger_pid();
        info->tid = xlogger_tid();
        info->maintid = xlogger_maintid();
    }
    
    if (NULL == _log) {
        if (_info) {
            XLoggerInfo* info = (XLoggerInfo*)_info;
            info->level = kLevelFatal;
        }
        gs_appender(_info, "NULL == _log");
    } else {
        gs_appender(_info, _log);
    }
}

void __xlogger_VPrint_impl(const XLoggerInfo* _info, const char* _format, va_list _list) {
    if (NULL == _format) {
        XLoggerInfo* info = (XLoggerInfo*)_info;
        info->level = kLevelFatal;
        __xlogger_Write_impl(_info, "NULL == _format");
    } else {
        char temp[4096] = {'\0'};
        vsnprintf(temp, 4096, _format, _list);
        __xlogger_Write_impl(_info, temp);
    }
}

extern void __ASSERTV2(const char * _pfile, int _line, const char * _pfunc, const char * _pexpression, const char * _format, va_list _list);
void __xlogger_AssertP_impl(const XLoggerInfo* _info, const char* _expression, const char* _format, va_list _list) {
    __ASSERTV2(_info->filename, _info->line, _info->func_name, _expression, _format, _list);
}

extern void __ASSERT2(const char * _pfile, int _line, const char * _pfunc, const char * _pexpression, const char * _format, ...);
void __xlogger_Assert_impl(const XLoggerInfo* _info, const char* _expression, const char* _log) {
    __ASSERT2(_info->filename, _info->line, _info->func_name, _expression, _log);
    
}
#endif
