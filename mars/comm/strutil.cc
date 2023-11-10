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
// Support for Symbian
// yerungui
////////////////////////////////////////////////////////////////////////////////

#include "strutil.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <locale>

#include "comm/xlogger/xlogger.h"
#include "openssl/md5.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace strutil {

std::string& URLEncode(const std::string& _url, std::string& _encode_url) {
    std::string::const_iterator iter = _url.begin();

    char transferr[4] = {0};

    for (; iter != _url.end(); ++iter) {
        char ch = *iter;
        if ((('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z')) || (('0' <= ch) && (ch <= '9')) || ch == '.'
            || ch == '-' || ch == '_' || ch == '*') {
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

#define TRIMLEFT(T)                              \
    T& TrimLeft(T& str) {                        \
        T& t = str;                              \
        T::iterator i;                           \
        for (i = t.begin(); i != t.end(); ++i) { \
            if (!isspace((int)*i)) {             \
                break;                           \
            }                                    \
        }                                        \
        if (i == t.end()) {                      \
            t.clear();                           \
        } else {                                 \
            t.erase(t.begin(), i);               \
        }                                        \
        return t;                                \
    }

#define TRIMRIGHT(T)                     \
    T& TrimRight(T& str) {               \
        if (str.begin() == str.end()) {  \
            return str;                  \
        }                                \
                                         \
        T& t = str;                      \
        T::iterator i;                   \
        for (i = t.end() - 1;; --i) {    \
            if (!isspace((int)*i)) {     \
                t.erase(i + 1, t.end()); \
                break;                   \
            }                            \
            if (i == t.begin()) {        \
                t.clear();               \
                break;                   \
            }                            \
        }                                \
        return t;                        \
    }

#define TRIM(T)                                  \
    T& Trim(T& str) {                            \
        T& t = str;                              \
                                                 \
        T::iterator i;                           \
        for (i = t.begin(); i != t.end(); ++i) { \
            if (!isspace((int)*i)) {             \
                break;                           \
            }                                    \
        }                                        \
        if (i == t.end()) {                      \
            t.clear();                           \
            return t;                            \
        } else {                                 \
            t.erase(t.begin(), i);               \
        }                                        \
                                                 \
        for (i = t.end() - 1;; --i) {            \
            if (!isspace((int)*i)) {             \
                t.erase(i + 1, t.end());         \
                break;                           \
            }                                    \
            if (i == t.begin()) {                \
                t.clear();                       \
                break;                           \
            }                                    \
        }                                        \
                                                 \
        return t;                                \
    }

#define TOLOWER(T)                                                \
    T& ToLower(T& str) {                                          \
        T& t = str;                                               \
        std::transform(t.begin(), t.end(), t.begin(), ::tolower); \
        return t;                                                 \
    }

#define TOUPPER(T)                                                \
    T& ToUpper(T& str) {                                          \
        T& t = str;                                               \
        std::transform(t.begin(), t.end(), t.begin(), ::toupper); \
        return t;                                                 \
    }

#define STARTSWITH(T)                                \
    bool StartsWith(const T& str, const T& substr) { \
        return str.find(substr) == 0;                \
    }

#define ENDSWITH(T)                                                       \
    bool EndsWith(const T& str, const T& substr) {                        \
        size_t i = str.rfind(substr);                                     \
        return (i != T::npos) && (i == (str.length() - substr.length())); \
    }

/*#define EQUALSIGNORECASE(T) bool EqualsIgnoreCase(const T& str1, const T& str2)\
{\
    T str1temp = str1;\
    T str2temp = str2;\
    return ToLower(str1temp) == ToLower(str2temp);\
}*/

#define SPLITTOKEN(T)                                                                   \
    std::vector<T>& SplitToken(const T& str, const T& delimiters, std::vector<T>& ss) { \
        Tokenizer<T> tokenizer(str, delimiters);                                        \
        while (tokenizer.NextToken()) {                                                 \
            ss.push_back(tokenizer.GetToken());                                         \
        }                                                                               \
                                                                                        \
        return ss;                                                                      \
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
// EQUALSIGNORECASE(string)
// EQUALSIGNORECASE(wstring)
//
SPLITTOKEN(std::string)
SPLITTOKEN(std::wstring)

#ifdef WIN32
#include <Windows.h>
std::wstring String2WString(const std::string& _src, unsigned int _cp) {
    const int len = static_cast<int>(_src.length());
    std::wstring enc;
    const int req = MultiByteToWideChar(_cp, 0, _src.c_str(), len, NULL, 0);
    if (req > 0) {
        enc.resize(static_cast<size_t>(req));
        MultiByteToWideChar(_cp, 0, _src.c_str(), len, &enc[0], req);
    }
    return enc;
}

std::wstring UTF8String2Wstring(const std::string& _src) {
    return String2WString(_src, CP_UTF8);
}
#endif
std::string Hex2Str(const char* _str, unsigned int _len) {
    std::string outstr = "";
    static const char* HEX = "0123456789abcdef";
    const uint8_t* input = (const uint8_t*)_str;
    uint8_t t, a, b;
    for (unsigned int i = 0; i < _len; i++) {
        t = input[i];
        // byte a = t / 16;
        a = t >> 4;
        // byte b = t % 16;
        b = t & 0x0f;
        outstr.append(1, HEX[a]);
        outstr.append(1, HEX[b]);
    }
    return outstr;
}

std::string Str2Hex(const char* _str, unsigned int _len) {
    if (_len > 1024) {
        xassert2(false, TSF "string length %_ too long.", _len);
        return "";
    }
    char outbuffer[512 + 1];
    
    unsigned int outoffset = 0;
    const char* ptr = _str;
    unsigned int length = _len / 2;

    if (length > sizeof(outbuffer))
        length = sizeof(outbuffer);

    for (unsigned int i = 0; i < length; i++) {
        char tmp[4];

        memset(tmp, 0, sizeof(tmp));
        tmp[0] = ptr[i * 2];
        tmp[1] = ptr[i * 2 + 1];
        char* p = NULL;
        outbuffer[outoffset] = (char)strtol(tmp, &p, 16);
        outoffset++;
    }
    std::string ret;
    ret.assign(outbuffer, outoffset);
    return ret;
}

std::string ReplaceChar(const char* const input_str, char be_replaced, char replace_with) {
    std::string output_str(input_str);
    size_t len = output_str.size();
    xassert2(len < 16 * 1024, TSF "input_str:%_", input_str);
    for (size_t i = 0; i < len; ++i) {
        if (be_replaced == output_str[i]) {
            output_str[i] = replace_with;
        }
    }
    return output_str;
}
std::string GetFileNameFromPath(const char* _path) {
    if (NULL == _path)
        return "";

    const char* pos = strrchr(_path, '\\');

    if (NULL == pos) {
        pos = strrchr(_path, '/');
    }

    if (NULL == pos || '\0' == *(pos + 1)) {
        return _path;
    } else {
        return pos + 1;
    }
}

template <typename charT>
struct my_equal {
    my_equal(const std::locale& loc) : loc_(loc) {
    }
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }

 private:
    const std::locale& loc_;
};

// find substring (case insensitive)
size_t ci_find_substr(const std::string& str, const std::string& sub, size_t pos) {
    const std::locale& loc = std::locale();
    typename std::string::const_iterator it = std::search(str.begin() + pos,
                                                          str.end(),
                                                          sub.begin(),
                                                          sub.end(),
                                                          my_equal<typename std::string::value_type>(loc));

    if (it != str.end())
        return it - str.begin();
    else
        return std::string::npos;  // not found
}

std::string MD5DigestToBase16(const uint8_t digest[16]) {
    return DigestToBase16(&digest[0], 16);
}

std::string DigestToBase16(const uint8_t* digest, size_t length) {
    assert(length % 2 == 0);
    static char const zEncode[] = "0123456789abcdef";

    std::string ret;
    ret.resize(length * 2);

    for (size_t i = 0, j = 0; i < length; i++, j += 2) {
        uint8_t a = digest[i];
        ret[j] = zEncode[(a >> 4) & 0xf];
        ret[j + 1] = zEncode[a & 0xf];
    }
    return ret;
}

}  // namespace strutil
