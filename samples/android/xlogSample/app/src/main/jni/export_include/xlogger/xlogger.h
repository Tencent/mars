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
 ============================================================================
 Name		: xlogger.h
 ============================================================================
 */

#ifndef XLOGGER_H_
#define XLOGGER_H_

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/cdefs.h>
#include <stdio.h>


#include "xloggerbase.h"
#include "preprocessor.h"

#ifdef XLOGGER_DISABLE
#define  xlogger_IsEnabledFor(_level)	(false)
#define  xlogger_AssertP(...) 		 	((void)0)
#define  xlogger_Assert(...)			((void)0)
#define  xlogger_VPrint(...)			((void)0)
#define  xlogger_Print(...)				((void)0)
#define  xlogger_Write(...)				((void)0)
#endif

#ifdef __cplusplus
#include <string>

template <bool x> struct XLOGGER_STATIC_ASSERTION_FAILURE;
template <> struct XLOGGER_STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template<int x> struct xlogger_static_assert_test{};


#define XLOGGER_STATIC_ASSERT( ... ) typedef ::xlogger_static_assert_test<\
										sizeof(::XLOGGER_STATIC_ASSERTION_FAILURE< ((__VA_ARGS__) == 0 ? false : true) >)>\
										PP_CAT(boost_static_assert_typedef_, __LINE__)


template<unsigned char base, typename T>
char* xlogger_itoa(T value, char* result, bool upper_case=true)
{
	XLOGGER_STATIC_ASSERT(2<=base && base <= 36);

	char* ptr_right = result, *ptr_left = result;
	T tmp_value = value;
	const char* num_mapping;

	if (upper_case)
		num_mapping = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	else
		num_mapping = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

	do
	{
		T quotient = tmp_value/base;
		*(ptr_right++) =  num_mapping[35 + tmp_value - quotient*base];
		tmp_value = quotient;
	} while (tmp_value);


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
	if (value < 0) *(ptr_right++) = '-';
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

	*(ptr_right--) = '\0';

	while(ptr_left < ptr_right)
	{
		char tmp_char = *ptr_right;
		*(ptr_right--)= *ptr_left;
		*(ptr_left++) = tmp_char;
	}
	return result;
}

class TVariant
{
public:
    TVariant(bool aBool):m_value(NULL) { if (aBool) m_value = "true"; else m_value = "false"; m_valuecache[0] = 0;}
    TVariant(char aChar):m_value(NULL) { m_valuecache[0] = aChar; m_valuecache[1] = '\0'; m_value = m_valuecache;}
    TVariant(signed char aSChar):m_value(NULL) { m_valuecache[0] = (char)aSChar; m_valuecache[1] = '\0'; m_value = m_valuecache; }
    TVariant(unsigned char aUChar):m_value(NULL) { xlogger_itoa<10>(aUChar, m_valuecache); m_value = m_valuecache; }
    TVariant(short aShort):m_value(NULL) { xlogger_itoa<10>(aShort, m_valuecache); m_value = m_valuecache; }
    TVariant(unsigned short aUShort):m_value(NULL) { xlogger_itoa<10>(aUShort, m_valuecache); m_value = m_valuecache; }
    TVariant(int aInt):m_value(NULL) { xlogger_itoa<10>(aInt, m_valuecache); m_value = m_valuecache; }
    TVariant(unsigned int aUInt):m_value(NULL) { xlogger_itoa<10>(aUInt, m_valuecache); m_value = m_valuecache;}
    TVariant(long aLong):m_value(NULL) { xlogger_itoa<10>(aLong, m_valuecache); m_value = m_valuecache; }
    TVariant(unsigned long aULong):m_value(NULL) { xlogger_itoa<10>(aULong, m_valuecache); m_value = m_valuecache; }
    TVariant(long long aLongLong):m_value(NULL) { xlogger_itoa<10>(aLongLong, m_valuecache); m_value = m_valuecache; }
    TVariant(unsigned long long aULongLong):m_value(NULL) { xlogger_itoa<10>(aULongLong, m_valuecache); m_value = m_valuecache; }
    TVariant(float aFloat):m_value(NULL) { snprintf(m_valuecache, sizeof(m_valuecache), "%E", aFloat); m_value = m_valuecache; }
    TVariant(double aDouble):m_value(NULL) { snprintf(m_valuecache, sizeof(m_valuecache), "%E", aDouble); m_value = m_valuecache; }
    TVariant(long double aLongDouble):m_value(NULL) { snprintf(m_valuecache, sizeof(m_valuecache), "%LE", aLongDouble); m_value = m_valuecache;}
    TVariant(const void* aVoidPtr):m_value(NULL) { m_valuecache[0] = '0';  m_valuecache[1] = 'x'; xlogger_itoa<16>((uintptr_t)aVoidPtr, m_valuecache+2); m_value = m_valuecache;}
    TVariant(const char* aCharPtr):m_value(NULL) { m_value = (const char*)aCharPtr;  m_valuecache[0] = '\0';}
    TVariant(char* _ptr):m_value(NULL) { m_value = (const char*)_ptr;  m_valuecache[0] = '\0';}
    TVariant(const unsigned char* aUCharPtr):m_value(NULL) { m_value = (const char*)aUCharPtr;  m_valuecache[0] = '\0';}
    TVariant(const std::string& aValue):m_value(NULL) { m_value = aValue.c_str();  m_valuecache[0] = '\0';}
    ~TVariant() {}
    const char* ToString() const { return m_value; }

private:
    TVariant(const TVariant&);
    TVariant& operator=(const TVariant&);
    
private:
    const char* m_value;
    char m_valuecache[65];
};



const struct TypeSafeFormat {TypeSafeFormat(){}} __tsf__;
const struct XLoggerTag {XLoggerTag(){}} __xlogger_tag__;
const struct XLoggerInfoNull {XLoggerInfoNull(){}} __xlogger_info_null__;


class XMessage
{
public:
	XMessage(): m_message() { m_message.reserve(512); }
	~XMessage() {}

public:
    const std::string& Message() const { return m_message;}
    std::string& Message() { return m_message;}

    const std::string& String() const { return m_message;}
    std::string& String() { return m_message;}

#ifdef __GNUC__
	__attribute__((__format__ (printf, 2, 0)))
#endif
	XMessage&  WriteNoFormat(const char* _log) { m_message+= _log; return *this;}
#ifdef __GNUC__
	__attribute__((__format__ (printf, 3, 0)))
#endif
	XMessage&  WriteNoFormat(const TypeSafeFormat&, const char* _log) { m_message+= _log; return *this;}

	XMessage& operator<<(const TVariant& _value);
	XMessage& operator>>(const TVariant& _value);

    XMessage& operator()() {return *this;}
    void operator+=(const TVariant& _value) { m_message+= _value.ToString();}
#ifdef __GNUC__
	__attribute__((__format__ (printf, 2, 3)))
#endif
	XMessage& operator()(const char* _format, ...);

#ifdef __GNUC__
    __attribute__((__format__ (printf, 2, 0)))
#endif
	XMessage& VPrintf(const char* _format, va_list _list);

#define XLOGGER_FORMAT_ARGS(n) PP_ENUM_TRAILING_PARAMS(n, const TVariant& a)
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(0));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(1));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(2));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(3));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(4));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(5));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(6));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(7));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(8));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(9));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(10));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(11));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(12));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(13));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(14));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(15));
	XMessage&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(16));
#undef XLOGGER_FORMAT_ARGS

private:
	void DoTypeSafeFormat(const char* _format, const TVariant** _args);

private:
//    XMessage(const XMessage&);
//    XMessage& operator=(const XMessage&);

private:
    std::string m_message;
};

class XLogger
{
public:
	XLogger(TLogLevel _level, const char* _tag, const char* _file, const char* _func, int _line, bool (*_hook)(XLoggerInfo& _info, std::string& _log))
    :m_info(), m_message(), m_isassert(false), m_exp(NULL),m_hook(_hook), m_isinfonull(false)
    {
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

        m_message.reserve(512);
    }
	~XLogger()
	{
        if (!m_isassert && m_message.empty()) return;

        gettimeofday(&m_info.timeval, NULL);
        if (m_hook && !m_hook(m_info, m_message)) return;
        
        if (m_isassert)
            xlogger_Assert(m_isinfonull?NULL:&m_info, m_exp, m_message.c_str());
        else
            xlogger_Write(m_isinfonull?NULL:&m_info, m_message.c_str());
	}

public:
	XLogger& Assert(const char* _exp)
	{
	    m_isassert = true;
        m_exp = _exp;
	    return *this;
	}
    
    bool Empty() const { return !m_isassert && m_message.empty();}
    const std::string& Message() const { return m_message;}

#ifdef __GNUC__
    __attribute__((__format__ (printf, 2, 0)))
#endif
	XLogger&  WriteNoFormat(const char* _log) { m_message+= _log; return *this;}
#ifdef __GNUC__
     __attribute__((__format__ (printf, 3, 0)))
#endif
	XLogger&  WriteNoFormat(const TypeSafeFormat&, const char* _log) { m_message+= _log; return *this;}

	XLogger& operator<<(const TVariant& _value);
	XLogger& operator>>(const TVariant& _value);

    void operator>>(XLogger& _xlogger)
    {
        if (_xlogger.m_info.level < m_info.level)
        {
        	_xlogger.m_info.level = m_info.level;
        	_xlogger.m_isassert = m_isassert;
        	_xlogger.m_exp = m_exp;
        }

        m_isassert = false;
        m_exp = NULL;

        _xlogger.m_message += m_message;
        m_message.clear();
    }

    void operator<<(XLogger& _xlogger)
    {
    	_xlogger.operator>>(*this);
    }

	XLogger& operator()() {return *this;}
	XLogger& operator()(const XLoggerInfoNull&) { m_isinfonull = true; return *this;}
	XLogger& operator()(const XLoggerTag&, const char* _tag) { m_info.tag = _tag; return *this;}
#ifdef __GNUC__
	__attribute__((__format__ (printf, 2, 3)))
#endif
	XLogger& operator()(const char* _format, ...);

#ifdef __GNUC__
	 __attribute__((__format__ (printf, 2, 0)))
#endif
	XLogger& VPrintf(const char* _format, va_list _list);

#define XLOGGER_FORMAT_ARGS(n) PP_ENUM_TRAILING_PARAMS(n, const TVariant& a)
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(0));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(1));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(2));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(3));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(4));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(5));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(6));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(7));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(8));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(9));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(10));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(11));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(12));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(13));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(14));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(15));
	XLogger&  operator()(const TypeSafeFormat&, const char*_format XLOGGER_FORMAT_ARGS(16));
#undef XLOGGER_FORMAT_ARGS

private:
	void DoTypeSafeFormat(const char* _format, const TVariant** _args);
    
private:
    XLogger(const XLogger&);
    XLogger& operator=(const XLogger&);
    
private:
	XLoggerInfo m_info;
    std::string m_message;
    bool m_isassert;
    const char* m_exp;
    bool (*m_hook)(XLoggerInfo& _info, std::string& _log);
	bool m_isinfonull;
};


class XScopeTracer
{
public:
    XScopeTracer(TLogLevel _level, const char* _tag, const char* _name, const char* _file, const char* _func, int _line, const char* _log)
    :m_enable(xlogger_IsEnabledFor(_level)), m_info(), m_tv()
    {
    	m_info.level = _level;

        if (m_enable)
        {
		    m_info.tag = _tag;
		    m_info.filename = _file;
		    m_info.func_name = _func;
		    m_info.line = _line;
	        gettimeofday(&m_info.timeval, NULL);
	        m_info.pid = -1;
	        m_info.tid = -1;
	        m_info.maintid = -1;

	        strncpy(m_name, _name, sizeof(m_name));
	        m_name[sizeof(m_name)-1] = '\0';

            m_tv = m_info.timeval;
            char strout[1024] = {'\0'};
            snprintf(strout, sizeof(strout), "-> %s %s", m_name, NULL!=_log? _log:"");
            xlogger_Write(&m_info, strout);
        }
    }

    ~XScopeTracer()
    {
        if (m_enable)
        {
            timeval tv;
            gettimeofday(&tv, NULL);
            m_info.timeval = tv;
            long timeSpan = (tv.tv_sec - m_tv.tv_sec) * 1000 + (tv.tv_usec - m_tv.tv_usec) / 1000;
            char strout[1024] = {'\0'};
            snprintf(strout, sizeof(strout), "<- %s +%ld, %s", m_name, timeSpan, m_exitmsg.c_str());
            xlogger_Write(&m_info, strout);
        }
    }
    
    void Exit(const std::string& _exitmsg)
    {
        m_exitmsg += _exitmsg;
    }
    
private:
    XScopeTracer(const XScopeTracer&);
    XScopeTracer& operator=(const XScopeTracer&);

private:
    bool m_enable;
    XLoggerInfo m_info;
	char m_name[128];
	timeval m_tv;
    
    std::string m_exitmsg;
};

///////////////////////////XMessage////////////////////
inline XMessage& XMessage::operator<<(const TVariant& _value)
{
	if (NULL != _value.ToString()) {
		m_message += _value.ToString();
	} else {
        assert(false);
	}
    return *this;
}

inline XMessage& XMessage::operator>>(const TVariant& _value)
{
	if (NULL != _value.ToString()) {
		m_message.insert(0,  _value.ToString());
	} else {
        assert(false);
	}
    return *this;
}

inline XMessage& XMessage::VPrintf(const char* _format, va_list _list)
{
    if (_format == NULL)
    {
        assert(false);
        return *this;
    }

	char temp[4096] = {'\0'};
	vsnprintf(temp, 4096, _format, _list);
	m_message += temp;
	return *this;
}

inline XMessage& XMessage::operator()(const char* _format, ...)
{
    if (_format == NULL)
    {
        assert(false);
        return *this;
    }

	va_list valist;
	va_start(valist, _format);
	VPrintf(_format, valist);
	va_end(valist);
    return *this;
}

#define XLOGGER_FORMAT_ARGS(n) PP_ENUM_TRAILING_PARAMS(n, const TVariant& a)
#define XLOGGER_VARIANT_ARGS(n) PP_ENUM_PARAMS(n, &a)
#define XLOGGER_VARIANT_ARGS_NULL(n) PP_ENUM(n, NULL)
#define XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(n, m) \
		inline XMessage& XMessage::operator()(const TypeSafeFormat&, const char* _format XLOGGER_FORMAT_ARGS(n)) { \
		if (_format != NULL) { \
			const TVariant* args[16] = { XLOGGER_VARIANT_ARGS(n) PP_COMMA_IF(PP_AND(n, m)) XLOGGER_VARIANT_ARGS_NULL(m) }; \
			DoTypeSafeFormat(_format, args); \
		} \
        return *this;\
	}

XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(0, 16)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(1, 15)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(2, 14)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(3, 13)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(4, 12)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(5, 11)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(6, 10)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(7, 9)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(8, 8)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(9, 7)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(10, 6)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(11, 5)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(12, 4)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(13, 3)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(14, 2)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(15, 1)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(16, 0)

#undef XLOGGER_FORMAT_ARGS
#undef XLOGGER_VARIANT_ARGS
#undef XLOGGER_VARIANT_ARGS_NULL
#undef XLOGGER_TYPESAFE_FORMAT_IMPLEMENT

inline void XMessage::DoTypeSafeFormat(const char* _format, const TVariant** _args) {

	const char* current = _format;
    int count = 0;
	while ('\0' != *current)
	{
       if ('%' != *current)
       {
           m_message += *current;
			++current;
			continue;
       }

		char nextch = *(current+1);
		if (('0' <=nextch  && nextch <= '9') || nextch == '_')
		{
			int argIndex = count;
            if (nextch != '_') argIndex = nextch - '0';

			if (_args[argIndex] != NULL)
			{
				if (NULL != _args[argIndex]->ToString())
				{
					m_message += _args[argIndex]->ToString();
				} else {
					m_message += "(null)";
			        assert(false);
				}
			} else {
		        assert(false);
			}
            count++;
			current += 2;
		}
		else if (nextch == '%') {
		    m_message += '%';
			current += 2;
		} else {
			++current;
	        assert(false);
		}
	}
}

///////////////////////////XLogger////////////////////
inline XLogger& XLogger::operator<<(const TVariant& _value)
{
	if (NULL != _value.ToString()) {
		m_message += _value.ToString();
	} else {
        m_info.level = kLevelFatal;
        m_message += "{!!! XLogger& XLogger::operator<<(const TVariant& _value): _value.ToString() == NULL !!!}";
        assert(false);
	}
    return *this;
}

inline XLogger& XLogger::operator>>(const TVariant& _value)
{
	if (NULL != _value.ToString()) {
		m_message.insert(0,  _value.ToString());
	} else {
        m_info.level = kLevelFatal;
		m_message.insert(0,  "{!!! XLogger& XLogger::operator>>(const TVariant& _value): _value.ToString() == NULL !!!}");
        assert(false);
	}
    return *this;
}

inline XLogger& XLogger::VPrintf(const char* _format, va_list _list)
{
    if (_format == NULL)
    {
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

inline XLogger& XLogger::operator()(const char* _format, ...)
{
    if (_format == NULL)
    {
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

#define XLOGGER_FORMAT_ARGS(n) PP_ENUM_TRAILING_PARAMS(n, const TVariant& a)
#define XLOGGER_VARIANT_ARGS(n) PP_ENUM_PARAMS(n, &a)
#define XLOGGER_VARIANT_ARGS_NULL(n) PP_ENUM(n, NULL)
#define XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(n, m) \
		inline XLogger& XLogger::operator()(const TypeSafeFormat&, const char* _format XLOGGER_FORMAT_ARGS(n)) { \
		if (_format != NULL) { \
			const TVariant* args[16] = { XLOGGER_VARIANT_ARGS(n) PP_COMMA_IF(PP_AND(n, m)) XLOGGER_VARIANT_ARGS_NULL(m) }; \
			DoTypeSafeFormat(_format, args); \
		} \
        return *this;\
	}

XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(0, 16)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(1, 15)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(2, 14)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(3, 13)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(4, 12)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(5, 11)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(6, 10)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(7, 9)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(8, 8)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(9, 7)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(10, 6)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(11, 5)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(12, 4)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(13, 3)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(14, 2)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(15, 1)
XLOGGER_TYPESAFE_FORMAT_IMPLEMENT(16, 0)


#undef XLOGGER_FORMAT_ARGS
#undef XLOGGER_VARIANT_ARGS
#undef XLOGGER_VARIANT_ARGS_NULL
#undef XLOGGER_TYPESAFE_FORMAT_IMPLEMENT

inline void XLogger::DoTypeSafeFormat(const char* _format, const TVariant** _args) {

	const char* current = _format;
    int count = 0;
	while ('\0' != *current)
	{
       if ('%' != *current)
       {
           m_message += *current;
			++current;
			continue;
       }

		char nextch = *(current+1);
		if (('0' <=nextch  && nextch <= '9') || nextch == '_')
		{

			int argIndex = count;
            if (nextch != '_') argIndex = nextch - '0';

			if (_args[argIndex] != NULL)
			{
				if (NULL != _args[argIndex]->ToString())
				{
					m_message += _args[argIndex]->ToString();
				} else {
			        m_info.level = kLevelFatal;
			        m_message += "{!!! void XLogger::DoTypeSafeFormat: _args[";
			        m_message += TVariant(argIndex).ToString();
			        m_message += "]->ToString() == NULL !!!}";
			        assert(false);
				}
			} else {
		        m_info.level = kLevelFatal;
		        m_message += "{!!! void XLogger::DoTypeSafeFormat: _args[";
		        m_message += TVariant(argIndex).ToString();
		        m_message += "] == NULL !!!}";
		        assert(false);
			}
            count++;
			current += 2;
		}
		else if (nextch == '%') {
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

#endif //cpp


#define __CONCAT_IMPL__(x, y)       x##y
#define __CONCAT__(x, y)            __CONCAT_IMPL__(x, y)
#define __ANONYMOUS_VARIABLE__(x)   __CONCAT__(x, __LINE__)

#define __XFILE__                   (__FILE__)

#ifndef _MSC_VER
    //#define __XFUNCTION__       __PRETTY_FUNCTION__
	#define __XFUNCTION__       __FUNCTION__
#else
    // Definitely, VC6 not support this feature!
    #if _MSC_VER > 1200
        #define __XFUNCTION__   __FUNCSIG__
    #else
        #define __XFUNCTION__   "N/A"
        #warning " is not supported by this compiler"
    #endif
#endif

//xlogger define

#ifndef XLOGGER_TAG
#define XLOGGER_TAG ""
#endif

/* tips: this code replace or change the tag in source file
static const char* __my_xlogger_tag = "prefix_"XLOGGER_TAG"_suffix";
#undef XLOGGER_TAG
#define XLOGGER_TAG __my_xlogger_tag
*/

#define xdump xlogger_dump
#define XLOGGER_ROUTER_OUTPUT(op1,op,...) PP_IF(PP_NUM_PARAMS(__VA_ARGS__),PP_IF(PP_DEC(PP_NUM_PARAMS(__VA_ARGS__)),op,op1), )

#if !defined(__cplusplus)

#ifdef __GNUC__
__attribute__((__format__ (printf, 2, 3)))
#endif
__inline void  __xlogger_c_write(const XLoggerInfo* _info, const char* _log, ...) { xlogger_Write(_info, _log); }

#define xlogger2(level, tag, file, func, line, ...)      if ((!xlogger_IsEnabledFor(level)));\
															  else { XLoggerInfo info= {level, tag, file, func, line,\
																	 {0, 0}, -1, -1, -1};\ gettimeofday(&info.m_tv, NULL);\
																	 XLOGGER_ROUTER_OUTPUT(__xlogger_c_write(&info, __VA_ARGS__),xlogger_Print(&info, __VA_ARGS__), __VA_ARGS__);}

#define xlogger2_if(exp, level, tag, file, func, line, ...)    if ((!(exp) || !xlogger_IsEnabledFor(level)));
																	else { XLoggerInfo info= {level, tag, file, func, line,\
																		   {0, 0}, -1, -1, -1}; gettimeofday(&info.timeval, NULL);\
																		   XLOGGER_ROUTER_OUTPUT(__xlogger_c_write(&info, __VA_ARGS__),xlogger_Print(&info, __VA_ARGS__), __VA_ARGS__);

#define __xlogger_c_impl(level,  ...) 			xlogger2(level, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, __VA_ARGS__)
#define __xlogger_c_impl_if(level, exp, ...) 	xlogger2_if(exp, level, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, __VA_ARGS__)

#define xverbose2(...)             __xlogger_c_impl(kLevelVerbose, __VA_ARGS__)
#define xdebug2(...)               __xlogger_c_impl(kLevelDebug, __VA_ARGS__)
#define xinfo2(...)                __xlogger_c_impl(kLevelInfo, __VA_ARGS__)
#define xwarn2(...)                __xlogger_c_impl(kLevelWarn, __VA_ARGS__)
#define xerror2(...)               __xlogger_c_impl(kLevelError, __VA_ARGS__)
#define xfatal2(...)               __xlogger_c_impl(kLevelFatal, __VA_ARGS__)

#define xverbose2_if(exp, ...)     __xlogger_c_impl_if(kLevelVerbose, exp, __VA_ARGS__)
#define xdebug2_if(exp, ...)       __xlogger_c_impl_if(kLevelDebug, exp, __VA_ARGS__)
#define xinfo2_if(exp, ...)        __xlogger_c_impl_if(kLevelInfo, exp, __VA_ARGS__)
#define xwarn2_if(exp, ...)        __xlogger_c_impl_if(kLevelWarn, exp,  __VA_ARGS__)
#define xerror2_if(exp, ...)       __xlogger_c_impl_if(kLevelError, exp, __VA_ARGS__)
#define xfatal2_if(exp, ...)       __xlogger_c_impl_if(kLevelFatal, exp, __VA_ARGS__)

#define xassert2(exp, ...)    if (((exp) || !xlogger_IsEnabledFor(kLevelFatal)));else {\
                                    XLoggerInfo info= {kLevelFatal, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__,\
                                    {0, 0}, -1, -1, -1};\
                                    gettimeofday(&info.m_tv, NULL);\
                                    xlogger_AssertP(&info, #exp, __VA_ARGS__);}
//"##__VA_ARGS__" remove "," if NULL
#else

#ifndef XLOGGER_HOOK
#define XLOGGER_HOOK NULL
#endif

#define xlogger(level, tag, file, func, line, ...)     if ((!xlogger_IsEnabledFor(level)));\
													   else XLogger(level, tag, file, func, line, XLOGGER_HOOK)\
													   	     XLOGGER_ROUTER_OUTPUT(.WriteNoFormat(TSF __VA_ARGS__),(TSF __VA_ARGS__), __VA_ARGS__)

#define xlogger2(level, tag, file, func, line, ...)     if ((!xlogger_IsEnabledFor(level)));\
									 	 	 	   	    else XLogger(level, tag, file, func, line, XLOGGER_HOOK)\
															 XLOGGER_ROUTER_OUTPUT(.WriteNoFormat(__VA_ARGS__),(__VA_ARGS__), __VA_ARGS__)

#define xlogger2_if(exp, level, tag, file, func, line, ...)     if ((!(exp) || !xlogger_IsEnabledFor(level)));\
																else XLogger(level, tag, file, func, line, XLOGGER_HOOK)\
																 	 XLOGGER_ROUTER_OUTPUT(.WriteNoFormat(__VA_ARGS__),(__VA_ARGS__), __VA_ARGS__)

#define __xlogger_cpp_impl2(level, ...)     		 xlogger2(level, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, __VA_ARGS__)
#define __xlogger_cpp_impl_if(level, exp, ...)     xlogger2_if(exp, level, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, __VA_ARGS__)

#define xverbose2(...)             __xlogger_cpp_impl2(kLevelVerbose, __VA_ARGS__)
#define xdebug2(...)               __xlogger_cpp_impl2(kLevelDebug, __VA_ARGS__)
#define xinfo2(...)                __xlogger_cpp_impl2(kLevelInfo, __VA_ARGS__)
#define xwarn2(...)                __xlogger_cpp_impl2(kLevelWarn, __VA_ARGS__)
#define xerror2(...)               __xlogger_cpp_impl2(kLevelError, __VA_ARGS__)
#define xfatal2(...)               __xlogger_cpp_impl2(kLevelFatal, __VA_ARGS__)
#define xlog2(level, ...)          __xlogger_cpp_impl2(level, __VA_ARGS__)

#define xverbose2_if(exp, ...)     __xlogger_cpp_impl_if(kLevelVerbose, exp,  __VA_ARGS__)
#define xdebug2_if(exp, ...)       __xlogger_cpp_impl_if(kLevelDebug, exp,  __VA_ARGS__)
#define xinfo2_if(exp, ...)        __xlogger_cpp_impl_if(kLevelInfo, exp,  __VA_ARGS__)
#define xwarn2_if(exp, ...)        __xlogger_cpp_impl_if(kLevelWarn, exp,  __VA_ARGS__)
#define xerror2_if(exp, ...)       __xlogger_cpp_impl_if(kLevelError, exp,  __VA_ARGS__)
#define xfatal2_if(exp, ...)       __xlogger_cpp_impl_ifkLevelFatal, exp, __VA_ARGS__)
#define xlog2_if(level, ...)	   __xlogger_cpp_impl_if(level, __VA_ARGS__)

#define xgroup2_define(group)      XLogger group(kLevelAll, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, XLOGGER_HOOK)
#define xgroup2(...)               XLogger(kLevelAll, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, XLOGGER_HOOK)(__VA_ARGS__)
#define xgroup2_if(exp, ...)       if ((!(exp))); else XLogger(kLevelAll, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, XLOGGER_HOOK)(__VA_ARGS__)

#define xassert2(exp, ...)    if (((exp) || !xlogger_IsEnabledFor(kLevelFatal)));\
							 else XLogger(kLevelFatal, XLOGGER_TAG, __XFILE__, __XFUNCTION__, __LINE__, XLOGGER_HOOK).Assert(#exp)\
							 	  XLOGGER_ROUTER_OUTPUT(.WriteNoFormat(__VA_ARGS__),(__VA_ARGS__), __VA_ARGS__)

#define xmessage2_define(name, ...)   	XMessage name; name XLOGGER_ROUTER_OUTPUT(.WriteNoFormat(__VA_ARGS__),(__VA_ARGS__), __VA_ARGS__)
#define xmessage2(...)   				XMessage() XLOGGER_ROUTER_OUTPUT(.WriteNoFormat(__VA_ARGS__),(__VA_ARGS__), __VA_ARGS__)


#define XLOGGER_SCOPE_MESSAGE(...)      PP_IF(PP_NUM_PARAMS(__VA_ARGS__), xmessage2(__VA_ARGS__).String().c_str(), NULL)
#define __xscope_impl(level, name, ...)   XScopeTracer __ANONYMOUS_VARIABLE__(_tracer_)(level, XLOGGER_TAG, name, __XFILE__, __XFUNCTION__, __LINE__, XLOGGER_SCOPE_MESSAGE(__VA_ARGS__))

#define xverbose_scope(name, ...)       __xscope_impl(kLevelVerbose, name, __VA_ARGS__)
#define xdebug_scope(name, ...)         __xscope_impl(kLevelDebug, name, __VA_ARGS__)
#define xinfo_scope(name, ...)          __xscope_impl(kLevelInfo, name, __VA_ARGS__)

#define __xfunction_scope_impl(level, name, ...)    XScopeTracer ____xloger_anonymous_function_scope_20151022____(level, XLOGGER_TAG, name, __XFILE__, __XFUNCTION__, __LINE__, XLOGGER_SCOPE_MESSAGE(__VA_ARGS__))

#define xverbose_function(...)          __xfunction_scope_impl(kLevelVerbose, __FUNCTION__, __VA_ARGS__)
#define xdebug_function(...)            __xfunction_scope_impl(kLevelDebug, __FUNCTION__, __VA_ARGS__)
#define xinfo_function(...)             __xfunction_scope_impl(kLevelInfo, __FUNCTION__, __VA_ARGS__)
#define xexitmsg_function(...)             ____xloger_anonymous_function_scope_20151022____.Exit(xmessage2(__VA_ARGS__).String())
#define xexitmsg_function_if(exp, ...)     if((!exp)); else ____xloger_anonymous_function_scope_20151022____.Exit(xmessage2(__VA_ARGS__).String())


#define TSF __tsf__,
#define XTAG __xlogger_tag__,
#define XNULL __xlogger_info_null__
#define XENDL "\n"
#define XTHIS "@%p, ", this

#endif
#endif /* XLOGGER_H_ */
