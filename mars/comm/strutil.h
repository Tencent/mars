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
// @(#) strutil.h
// Utilities for string
// defined in namespace strutil
// Support for Symbian
// yerungui
////////////////////////////////////////////////////////////////////////////////

#ifndef COMM_STRUTIL_H_
#define COMM_STRUTIL_H_

#include <sstream>
#include <string>
#include <vector>

// declaration
namespace strutil {
/////////////////////// string /////////////////////////////
std::string& URLEncode(const std::string& url, std::string& encodeUrl);

std::string& TrimLeft(std::string& str);
std::string& TrimRight(std::string& str);
std::string& Trim(std::string& str);

std::string& ToLower(std::string& str);
std::string& ToUpper(std::string& str);

bool StartsWith(const std::string& str, const std::string& substr);
bool EndsWith(const std::string& str, const std::string& substr);

std::vector<std::string>& SplitToken(const std::string& str,
                                     const std::string& delimiters,
                                     std::vector<std::string>& ss);

// T1 is iterator, T2 is string or wstring
template <typename T1, typename T2>
bool MergeToken(const T1& begin, const T1& end, const T2& delimiter, T2& result);
/////////////////////// wstring /////////////////////////////
std::wstring& TrimLeft(std::wstring& str);
std::wstring& TrimRight(std::wstring& str);
std::wstring& Trim(std::wstring& str);

bool StartsWith(const std::wstring& str, const std::wstring& substr);
bool EndsWith(const std::wstring& str, const std::wstring& substr);

std::wstring& ToLower(std::wstring& str);
std::wstring& ToUpper(std::wstring& str);

#ifdef WIN32
std::wstring String2WString(const std::string& _src, unsigned int _cp);
std::wstring UTF8String2Wstring(const std::string& _src);
#endif
std::vector<std::wstring>& SplitToken(const std::wstring& str,
                                      const std::wstring& delimiters,
                                      std::vector<std::wstring>& ss);

// Tokenizer class
template <class T>
struct default_delimiters {};
template <>
struct default_delimiters<std::string> {
    static const char* value() {
        return " \t\n\r;:,.?";
    }
};
template <>
struct default_delimiters<std::wstring> {
    static const wchar_t* value() {
        return L" \t\n\r;:,.?";
    }
};

template <class T>
class Tokenizer {
 public:
    Tokenizer(const T& str, const T& delimiters = default_delimiters<T>::value())
    : offset_(0), string_(str), delimiters_(delimiters) {
    }

    void Reset() {
        offset_ = 0;
    }
    const T GetToken() const {
        return token_;
    }
    bool NextToken() {
        return NextToken(delimiters_);
    }
    bool NextToken(const T& delimiters) {
        // find the start charater of the next token.
        typename T::size_type i = string_.find_first_not_of(delimiters, offset_);

        if (i == T::npos) {
            offset_ = string_.length();
            return false;
        }

        // find the end of the token.
        typename T::size_type j = string_.find_first_of(delimiters, i);

        if (j == T::npos) {
            token_ = string_.substr(i, string_.length() - i);
            offset_ = string_.length();
            return true;
        }

        // to intercept the token and save current position
        token_ = string_.substr(i, j - i);
        offset_ = j;
        return true;
    }

 private:
    Tokenizer(const Tokenizer&);
    Tokenizer& operator=(const Tokenizer&);

 protected:
    typename T::size_type offset_;

    const T string_;
    T token_;
    T delimiters_;
};

template <typename T1, typename T2>
bool MergeToken(const T1& begin, const T1& end, const T2& delimiter, T2& result) {
    if (begin == end) {
        return false;
    }

    if (delimiter.empty()) {
        return false;
    }

    result.clear();

    for (T1 iter = begin; iter != end; ++iter) {
        result += *iter;

        if (iter + 1 != end) {
            result += delimiter;
        }
    }

    return true;
}

std::string Hex2Str(const char* _str, unsigned int _len);
std::string Str2Hex(const char* _str, unsigned int _len);
std::string Hex2Str(const std::string& hex);
std::string Str2Hex(const std::string& str);

std::string ReplaceChar(const char* const input_str, char be_replaced = '@', char replace_with = '.');

std::string GetFileNameFromPath(const char* _path);

// find substring (case insensitive)
size_t ci_find_substr(const std::string& str, const std::string& sub, size_t pos);
std::string BufferMD5(const void* buffer, size_t size);
std::string MD5DigestToBase16(const uint8_t digest[16]);
std::string DigestToBase16(const uint8_t* digest, size_t length);

std::string CStr2StringSafe(const char* a);
bool CStrNullOrEmpty(const char* a);
bool CStrCmpSafe(const char* a, const char* b);
int32_t CStr2Int32Safe(const char* a, int32_t default_num);

void to_lower(std::string& str);
void to_upper(std::string& str);

#if __cplusplus > 201103L
template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
std::string to_hex_string(const T& v) {
    std::stringstream hex_stream;
    hex_stream << std::hex << v;
    return hex_stream.str();
}

template <typename T, std::enable_if_t<std::is_enum<T>::value, bool> = true>
std::string to_hex_string(const T& v) {
    std::stringstream hex_stream;
    hex_stream << std::hex << static_cast<int>(v);
    return hex_stream.str();
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
std::string to_oct_string(const T& v) {
    std::stringstream hex_stream;
    hex_stream << std::oct << v;
    return hex_stream.str();
}

template <typename T, std::enable_if_t<std::is_enum<T>::value, bool> = true>
std::string to_oct_string(const T& v) {
    std::stringstream hex_stream;
    hex_stream << std::oct << static_cast<int>(v);
    return hex_stream.str();
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
std::string to_string(const T& v) {
    return std::to_string(v);
}

template <typename T, std::enable_if_t<std::is_enum<T>::value, bool> = true>
std::string to_string(const T& v) {
    return std::to_string(static_cast<int>(v));
}

// std::string
template <typename T, std::enable_if_t<std::is_same<T, std::string>::value, bool> = true>
std::string to_string(const T& v) {
    return v;
}

// char*
template <typename T, std::enable_if_t<std::is_same<std::remove_cv_t<T>, char*>::value, bool> = true>
std::string to_string(const T& v) {
    return std::string(v);
}

// literal strings
template <typename T, std::size_t N, std::enable_if_t<std::is_same<T, char>::value, bool> = true>
std::string to_string(T const (&v)[N]) {
    return std::string(v);
}

template <typename T1, typename T2>
std::string to_string(const std::pair<T1, T2>& v) {
    return "(" + to_string(v.first) + ":" + to_string(v.second) + ")";
}

template <typename T>
std::string to_string(T* v) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << (uint64_t)(v);
    return ss.str();
}

template <class T>
std::string join_to_string(const T& stl,
                           const std::string& seperator = ",",
                           const std::string& prefix = "{",
                           const std::string& postfix = "}") {
    if (stl.empty()) {
        return {};
    }
    std::string rtn = prefix;
    for (const auto& it : stl) {
        rtn.append(strutil::to_string(it)).append(seperator);
    }
    return rtn.append(postfix);
}
#endif
}  // namespace strutil

#endif  // COMM_STRUTIL_H_
