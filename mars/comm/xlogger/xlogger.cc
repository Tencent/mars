// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 ============================================================================
 Name		: xlogger.h
 ============================================================================
 */

#include "xlogger.h"

XLogger::XLogger(TLogLevel _level,
                 const char* _tag,
                 const char* _file,
                 const char* _func,
                 int _line,
                 bool _trace,
                 bool (*_hook)(XLoggerInfo& _info, std::string& _log))
: m_info(), m_message(), m_isassert(false), m_exp(NULL), m_hook(_hook), m_isinfonull(false) {
    m_info.level = _level;
    m_info.tag = _tag;
    m_info.filename = _file;
    m_info.func_name = _func;
    m_info.line = _line;
    m_info.timeval.tv_sec = 0;
    m_info.timeval.tv_usec = 0;
    m_info.pid = -1;
    m_info.tid = -1;
    m_info.maintid = -1;
    m_info.traceLog = _trace ? 1 : 0;

    m_message.reserve(512);
}

XLogger::~XLogger() {
    if (!m_isassert && m_message.empty())
        return;

    gettimeofday(&m_info.timeval, NULL);
    if (m_hook && !m_hook(m_info, m_message))
        return;

    xlogger_filter_t filter = xlogger_GetFilter();
    if (filter && filter(&m_info, m_message.c_str()) <= 0)
        return;

    if (m_isassert)
        xlogger_Assert(m_isinfonull ? NULL : &m_info, m_exp, m_message.c_str());
    else
        xlogger_Write(m_isinfonull ? NULL : &m_info, m_message.c_str());
}

void XLogger::operator>>(XLogger& _xlogger) {
    if (_xlogger.m_info.level < m_info.level) {
        _xlogger.m_info.level = m_info.level;
        _xlogger.m_isassert = m_isassert;
        _xlogger.m_exp = m_exp;
    }

    m_isassert = false;
    m_exp = NULL;

    _xlogger.m_message += m_message;
    m_message.clear();
}

void XLogger::operator<<(XLogger& _xlogger) {
    _xlogger.operator>>(*this);
}

XLogger& XLogger::Assert(const char* _exp) {
    m_isassert = true;
    m_exp = _exp;
    return *this;
}

XLogger& XLogger::operator<<(const string_cast& _value) {
    if (NULL != _value.str()) {
        m_message += _value.str();
    } else {
        m_info.level = kLevelFatal;
        m_message += "{!!! XLogger& XLogger::operator<<(const string_cast& _value): _value.str() == NULL !!!}";
        assert(false);
    }
    return *this;
}

XLogger& XLogger::operator>>(const string_cast& _value) {
    if (NULL != _value.str()) {
        m_message.insert(0, _value.str());
    } else {
        m_info.level = kLevelFatal;
        m_message.insert(0, "{!!! XLogger& XLogger::operator>>(const string_cast& _value): _value.str() == NULL !!!}");
        assert(false);
    }
    return *this;
}

XLogger& XLogger::VPrintf(const char* _format, va_list _list) {
    if (_format == NULL) {
        m_info.level = kLevelFatal;
        m_message += "{!!! XLogger& XLogger::operator()(const char* _format, va_list _list): _format == NULL !!!}";
        assert(false);
        return *this;
    }

    char temp[4096] = {'\0'};
    vsnprintf(temp, 4096, _format, _list);
    m_message += temp;
    return *this;
}

XLogger& XLogger::operator()(const char* _format, ...) {
    if (_format == NULL) {
        m_info.level = kLevelFatal;
        m_message += "{!!! XLogger& XLogger::operator()(const char* _format, ...): _format == NULL !!!}";
        assert(false);
        return *this;
    }

    va_list valist;
    va_start(valist, _format);
    VPrintf(_format, valist);
    va_end(valist);
    return *this;
}

void XLogger::DoTypeSafeFormat(const char* _format, const string_cast** _args) {
    const char* current = _format;
    int count = 0;
    while ('\0' != *current) {
        if ('%' != *current) {
            m_message += *current;
            ++current;
            continue;
        }

        char nextch = *(current + 1);
        if (('0' <= nextch && nextch <= '9') || nextch == '_') {
            int argIndex = count;
            if (nextch != '_')
                argIndex = nextch - '0';

            if (_args[argIndex] != NULL) {
                if (NULL != _args[argIndex]->str()) {
                    m_message += _args[argIndex]->str();
                } else {
                    m_info.level = kLevelFatal;
                    m_message += "{!!! void XLogger::DoTypeSafeFormat: _args[";
                    m_message += string_cast(argIndex).str();
                    m_message += "]->str() == NULL !!!}";
                    assert(false);
                }
            } else {
                m_info.level = kLevelFatal;
                m_message += "{!!! void XLogger::DoTypeSafeFormat: _args[";
                m_message += string_cast(argIndex).str();
                m_message += "] == NULL !!!}";
                assert(false);
            }
            count++;
            current += 2;
        } else if (nextch == '%') {
            m_message += '%';
            current += 2;
        } else {
            ++current;
            m_info.level = kLevelFatal;
            m_message += "{!!! void XLogger::DoTypeSafeFormat: %";
            m_message += nextch;
            m_message += " not fit mode !!!}";
            assert(false);
        }
    }
}

XScopeTracer::XScopeTracer(TLogLevel _level,
                           const char* _tag,
                           const char* _name,
                           const char* _file,
                           const char* _func,
                           int _line,
                           const char* _log)
: m_enable(xlogger_IsEnabledFor(_level)), m_info(), m_tv() {
    m_info.level = _level;

    if (m_enable) {
        m_info.tag = _tag;
        m_info.filename = _file;
        m_info.func_name = _func;
        m_info.line = _line;
        gettimeofday(&m_info.timeval, NULL);
        m_info.pid = -1;
        m_info.tid = -1;
        m_info.maintid = -1;

        strncpy(m_name, _name, sizeof(m_name));
        m_name[sizeof(m_name) - 1] = '\0';

        m_tv = m_info.timeval;
        char strout[1024] = {'\0'};
        snprintf(strout, sizeof(strout), "-> %s %s", m_name, NULL != _log ? _log : "");
        xlogger_filter_t filter = xlogger_GetFilter();
        if (NULL == filter || filter(&m_info, strout) > 0) {
            xlogger_Write(&m_info, strout);
        }
    }
}

XScopeTracer::~XScopeTracer() {
    if (m_enable) {
        timeval tv;
        gettimeofday(&tv, NULL);
        m_info.timeval = tv;
        long timeSpan = (tv.tv_sec - m_tv.tv_sec) * 1000 + (tv.tv_usec - m_tv.tv_usec) / 1000;
        char strout[1024] = {'\0'};
        snprintf(strout, sizeof(strout), "<- %s +%ld, %s", m_name, timeSpan, m_exitmsg.c_str());
        xlogger_filter_t filter = xlogger_GetFilter();
        if (NULL == filter || filter(&m_info, strout) > 0) {
            xlogger_Write(&m_info, strout);
        }
    }
}

///////////////////////////XMessage////////////////////
XMessage& XMessage::operator<<(const string_cast& _value) {
    if (NULL != _value.str()) {
        m_message += _value.str();
    } else {
        assert(false);
    }
    return *this;
}

XMessage& XMessage::operator>>(const string_cast& _value) {
    if (NULL != _value.str()) {
        m_message.insert(0, _value.str());
    } else {
        assert(false);
    }
    return *this;
}

XMessage& XMessage::VPrintf(const char* _format, va_list _list) {
    if (_format == NULL) {
        assert(false);
        return *this;
    }

    char temp[4096] = {'\0'};
    vsnprintf(temp, 4096, _format, _list);
    m_message += temp;
    return *this;
}

XMessage& XMessage::operator()(const char* _format, ...) {
    if (_format == NULL) {
        assert(false);
        return *this;
    }

    va_list valist;
    va_start(valist, _format);
    VPrintf(_format, valist);
    va_end(valist);
    return *this;
}

void XMessage::DoTypeSafeFormat(const char* _format, const string_cast** _args) {
    const char* current = _format;
    int count = 0;
    while ('\0' != *current) {
        if ('%' != *current) {
            m_message += *current;
            ++current;
            continue;
        }

        char nextch = *(current + 1);
        if (('0' <= nextch && nextch <= '9') || nextch == '_') {
            int argIndex = count;
            if (nextch != '_')
                argIndex = nextch - '0';

            if (_args[argIndex] != NULL) {
                if (NULL != _args[argIndex]->str()) {
                    m_message += _args[argIndex]->str();
                } else {
                    m_message += "(null)";
                    assert(false);
                }
            } else {
                assert(false);
            }
            count++;
            current += 2;
        } else if (nextch == '%') {
            m_message += '%';
            current += 2;
        } else {
            ++current;
            assert(false);
        }
    }
}
