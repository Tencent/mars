// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// @(#) strutil.cc
// Utilities for std::string
// defined in namespace strutil
// by James Fancy
//Support for Symbian
// yerungui
////////////////////////////////////////////////////////////////////////////////

#include "strutil.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "comm/xlogger/xlogger.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace strutil
{


    
std::string& URLEncode(const std::string& _url, std::string& _encode_url) {
    std::string::const_iterator iter = _url.begin();
    
    char transferr[4] = {0};
    
    for (; iter != _url.end(); ++iter) {
          char ch = *iter;
        if ((('A'<=ch) && (ch<='Z')) ||
            (('a'<=ch) && (ch<='z')) ||
            (('0'<=ch) && (ch<='9')) ||
            ch == '.' || ch == '-' || ch == '_' || ch == '*') {
            _encode_url += ch;
        } else if (ch == ' ') {
            _encode_url += '+';
        } else {
            snprintf(transferr, sizeof(transferr), "%%%02X", (unsigned char)ch);
            _encode_url.append(transferr);
        }
    }
    
    return _encode_url;
}



#define TRIMLEFT(T) T& TrimLeft(T& str)\
{\
    T& t = str;\
    T::iterator i;\
    for (i = t.begin(); i != t.end(); ++i)\
    {\
        if (!isspace((int)*i))\
        {\
            break;\
        }\
    }\
    if (i == t.end())\
    {\
        t.clear();\
    }\
    else\
    {\
        t.erase(t.begin(), i);\
    }\
    return t;\
}

#define TRIMRIGHT(T) T& TrimRight(T& str)\
{\
    if (str.begin() == str.end())\
    {\
        return str;\
    }\
\
    T& t = str;\
    T::iterator i;\
    for (i = t.end() - 1;; --i)\
    {\
        if (!isspace((int)*i))\
        {\
            t.erase(i + 1, t.end());\
            break;\
        }\
        if (i == t.begin())\
        {\
            t.clear();\
            break;\
        }\
    }\
    return t;\
}

#define TRIM(T) T& Trim(T& str)\
{\
    T& t = str;\
\
    T::iterator i;\
    for (i = t.begin(); i != t.end(); ++i)\
    {\
        if (!isspace((int)*i))\
        {\
            break;\
        }\
    }\
    if (i == t.end())\
    {\
        t.clear();\
        return t;\
    }\
    else\
    {\
        t.erase(t.begin(), i);\
    }\
\
    for (i = t.end() - 1;; --i)\
    {\
        if (!isspace((int)*i))\
        {\
            t.erase(i + 1, t.end());\
            break;\
        }\
        if (i == t.begin())\
        {\
            t.clear();\
            break;\
        }\
    }\
\
    return t;\
}

#define TOLOWER(T) T& ToLower(T& str)\
{\
    T& t = str;\
	std::transform(t.begin(), t.end(), t.begin(), ::tolower);\
    return t;\
}

#define TOUPPER(T) T& ToUpper(T& str)\
{\
    T& t = str;\
	std::transform(t.begin(), t.end(), t.begin(), ::toupper);\
    return t;\
}

#define STARTSWITH(T) bool StartsWith(const T& str, const T& substr)\
{\
    return str.find(substr) == 0;\
}

#define ENDSWITH(T) bool EndsWith(const T& str, const T& substr)\
{\
    size_t i = str.rfind(substr);\
    return (i != T::npos) && (i == (str.length() - substr.length()));\
}

/*#define EQUALSIGNORECASE(T) bool EqualsIgnoreCase(const T& str1, const T& str2)\
{\
    T str1temp = str1;\
    T str2temp = str2;\
    return ToLower(str1temp) == ToLower(str2temp);\
}*/

#define SPLITTOKEN(T) std::vector<T>& SplitToken(const T& str, const T& delimiters, std::vector<T>& ss)\
{\
    Tokenizer<T> tokenizer(str, delimiters);\
    while (tokenizer.NextToken())\
    {\
        ss.push_back(tokenizer.GetToken());\
    }\
\
    return ss;\
}
//
TRIMLEFT(std::string)
TRIMLEFT(std::wstring)

TRIMRIGHT(std::string)
TRIMRIGHT(std::wstring)

TRIM(std::string)
TRIM(std::wstring)

TOLOWER(std::string)
TOLOWER(std::wstring)

TOUPPER(std::string)
TOUPPER(std::wstring)

STARTSWITH(std::string)
STARTSWITH(std::wstring)

ENDSWITH(std::string)
ENDSWITH(std::wstring)
//
//EQUALSIGNORECASE(string)
//EQUALSIGNORECASE(wstring)
//
SPLITTOKEN(std::string)
SPLITTOKEN(std::wstring)

std::string Hex2Str(const char* _str, unsigned int _len) {
    std::string outstr="";
    for(unsigned int i = 0; i< _len;i++) {
        char tmp[8];
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp,sizeof(tmp)-1,"%02x",(unsigned char)_str[i]);
        std::string tmpstr = tmp;
        outstr = outstr+tmpstr;

    }
    return outstr;
}

std::string Str2Hex(const char* _str, unsigned int _len) {
    char outbuffer[64];
    
    unsigned int outoffset = 0;
    const char * ptr = _str;
    unsigned int  length = _len/2;
    
    if (length > sizeof(outbuffer))
        length = sizeof(outbuffer);
    
    for(unsigned int i = 0; i< length;i++) {
        char tmp[4];
        
        memset(tmp,0,sizeof(tmp));
        tmp[0] = ptr[i*2];
        tmp[1] = ptr[i*2+1];
        char *p = NULL;
        outbuffer[outoffset] = (char)strtol(tmp,&p,16);
        outoffset++;
    }
    std::string ret ;
    ret.assign(outbuffer,outoffset);
    return ret;
}
    

std::string ReplaceChar(const char* const input_str, char be_replaced, char replace_with) {
	std::string output_str(input_str);
	size_t len = output_str.size();
	xassert2(len<16);
	for(size_t i=0; i<len; ++i) {
		if (be_replaced == output_str[i]) {
			output_str[i] = replace_with;
		}
	}
	return output_str;
}
}
