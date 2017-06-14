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
 Author		: yerungui
 Created on	: 2016-04-14
 */

#ifndef STRING_CAST_H_
#define STRING_CAST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <strings.h>
#else
#include "projdef.h"
#endif
#include <inttypes.h>
#include <string.h>

#include <limits>
#include <string>

#include "strutil.h"

template<typename T>
char* string_cast_itoa(const T& value, char* result, uint8_t base = 10, bool upper_case=true) {
    
    if(!(2<=base && base <= 36)) {
        strcpy(result, "itoa err");
        return result;
    }
    
    char* ptr_right = result, *ptr_left = result;
    T tmp_value = value;
    const char* num_mapping;
    
    if (upper_case)
        num_mapping = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    else
        num_mapping = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";
    
    do {
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
    
    while(ptr_left < ptr_right) {
        char tmp_char = *ptr_right;
        *(ptr_right--)= *ptr_left;
        *(ptr_left++) = tmp_char;
    }
    return result;
}


#define string_cast_hex(value) string_cast(value, 16)
#define string_cast_oct(value) string_cast(value, 8)

class string_cast {
public:
    string_cast(char _value):value_(NULL) {  value_cache_[0] = _value; value_cache_[1] = '\0'; value_ = value_cache_;}
    
    string_cast(int _value, uint8_t base=10):value_(NULL) { string_cast_itoa(_value, value_cache_, base); value_ = value_cache_;}
    string_cast(long _value, uint8_t base=10):value_(NULL) { string_cast_itoa(_value, value_cache_, base); value_ = value_cache_;}
    string_cast(long long _value, uint8_t base=10):value_(NULL) { string_cast_itoa(_value, value_cache_, base); value_ = value_cache_;}
    
    string_cast(unsigned int _value, uint8_t base=10):value_(NULL) { string_cast_itoa(_value, value_cache_, base); value_ = value_cache_;}
    string_cast(unsigned long _value, uint8_t base=10):value_(NULL) { string_cast_itoa(_value, value_cache_, base); value_ = value_cache_;}
    string_cast(unsigned long long _value, uint8_t base=10):value_(NULL) { string_cast_itoa(_value, value_cache_, base); value_ = value_cache_;}

    string_cast(float _value):value_(NULL) { snprintf(value_cache_, sizeof(value_cache_), "%E", _value); value_ = value_cache_; }
    string_cast(double _value):value_(NULL) { snprintf(value_cache_, sizeof(value_cache_), "%E", _value); value_ = value_cache_; }
    string_cast(long double _value):value_(NULL) { snprintf(value_cache_, sizeof(value_cache_), "%LE", _value); value_ = value_cache_;}
    
    string_cast(bool _value):value_(NULL) { if (_value) value_ = "true"; else value_ = "false"; value_cache_[0] = '\0';}
    string_cast(const void* _value):value_(NULL) { value_cache_[0] = '0';  value_cache_[1] = 'x'; string_cast_itoa((uintptr_t)_value, value_cache_+2, 16); value_ = value_cache_;}
    
    string_cast(const char* _value):value_(NULL) { value_ = (const char*)_value;  value_cache_[0] = '\0';}
    string_cast(const std::string& _value):value_(NULL) { value_ = _value.c_str();  value_cache_[0] = '\0';}
    
    const char* str() const { return value_;}
    operator const char* () const { return value_;}
    
private:
    string_cast(const string_cast&);
    string_cast& operator=(const string_cast&);
    
private:
    const char* value_;
    char value_cache_[65];
};

namespace detail {
    
template <typename T, int base=0>
class __signed_number_cast {
public:
    __signed_number_cast(const char* _str):value_(0), vaild_(false) {
        
        if (_str == NULL)
            return;
        
        char *end = NULL;
        
        vaild_ = true;
        value_ = strtoimax(_str, &end, base);
        
        if (_str == end) {
            vaild_ = false;
            return;
        }
        
        if (value_ <( std::numeric_limits<T>::min)()) {
            value_ =( std::numeric_limits<T>::min)();
            vaild_ = false;
            return;
        }
        if ((std::numeric_limits<T>::max)() < value_) {
            value_ = (std::numeric_limits<T>::max)();
            vaild_ = false;
            return;
        }
    }
    
    operator T () const { return static_cast<T>(value_); }
    bool valid() const { return vaild_;}
    
private:
    intmax_t value_;
    bool     vaild_;
};
    
template <typename T, int base=0>
class __unsigned_number_cast {
public:
    __unsigned_number_cast(const char* _str):value_(0), vaild_(false) {
        
        if (_str == NULL)
            return;
        
        char *end = NULL;
        
        vaild_ = true;
        value_ = strtoumax(_str, &end, base);
        
        if (_str == end) {
            vaild_ = false;
            return;
        }
        if (value_ <( std::numeric_limits<T>::min)()) {
            value_ =( std::numeric_limits<T>::min)();
            vaild_ = false;
            return;
        }
        if ((std::numeric_limits<T>::max)() < value_) {
            value_ = (std::numeric_limits<T>::max)();
            vaild_ = false;
            return;
        }
    }
    
    operator T () const { return static_cast<T>(value_); }
    bool valid() const { return vaild_;}
    
private:
    uintmax_t value_;
    bool      vaild_;
};
        
template <typename T>
class __float_number_cast {
public:
    __float_number_cast(const char* _str):value_(0), vaild_(false) {
        
        if (_str == NULL)
            return;
            
        char *end = NULL;
        
        vaild_ = true;
        value_ = strtod(_str, &end);
        
        if (_str == end) {
            vaild_ = false;
        }
    }
    
    operator T() const { return static_cast<T>(value_); }
    bool valid() const { return vaild_;}
    
private:
    double      value_;
    bool        vaild_;
};
}

template <typename T> class number_cast {};

template <> class number_cast<int8_t > : public detail::__signed_number_cast<int8_t >
{ public: number_cast(const char* _str):__signed_number_cast(_str){}; };
template <> class number_cast<int16_t> : public detail::__signed_number_cast<int16_t>
{ public: number_cast(const char* _str):__signed_number_cast(_str){}; };
template <> class number_cast<int32_t> : public detail::__signed_number_cast<int32_t>
{ public: number_cast(const char* _str):__signed_number_cast(_str){}; };
template <> class number_cast<long>    : public detail::__signed_number_cast<long>
{ public: number_cast(const char* _str):__signed_number_cast(_str){}; };
template <> class number_cast<long long>  : public detail::__signed_number_cast<long long>
{ public: number_cast(const char* _str):__signed_number_cast(_str){}; };

template <> class number_cast<uint8_t > : public detail::__unsigned_number_cast<uint8_t >
{ public: number_cast(const char* _str) :__unsigned_number_cast(_str){}; };
template <> class number_cast<uint16_t> : public detail::__unsigned_number_cast<uint16_t>
{ public: number_cast(const char* _str) :__unsigned_number_cast(_str){}; };
template <> class number_cast<uint32_t> : public detail::__unsigned_number_cast<uint32_t>
{ public: number_cast(const char* _str) :__unsigned_number_cast(_str){}; };
template <> class number_cast<unsigned long> : public detail::__unsigned_number_cast<unsigned long>
{ public: number_cast(const char* _str) :__unsigned_number_cast(_str){}; };
template <> class number_cast<unsigned long long> : public detail::__unsigned_number_cast<unsigned long long>
{ public: number_cast(const char* _str) :__unsigned_number_cast(_str){}; };
    
template <> class number_cast<float> : public detail::__float_number_cast<float>
{ public: number_cast(const char* _str):__float_number_cast(_str){}; };
template <> class number_cast<double> : public detail::__float_number_cast<double>
{ public: number_cast(const char* _str):__float_number_cast(_str){}; };
        
template <>
class number_cast<const char*> {
public:
    number_cast(const char* _str):value_(NULL), vaild_(false) {
        
        if (_str == NULL)
            return;
        
        value_ = _str;
        vaild_ = true;
    }
    
    operator const char*() const { return value_; }
    bool valid() const { return vaild_;}
    
private:
    const char* value_;
    bool vaild_;
};
        
template <>
class number_cast<bool> {
public:
    number_cast(const char* _str):value_(false), vaild_(false) {
        
        if (_str == NULL)
            return;
        
        std::vector<std::string> vec_split;
        strutil::SplitToken(_str, strutil::default_delimiters<std::string>::value(), vec_split);
        
        if (vec_split.empty()) { return; }
        if (vec_split[0] == "1" || 0 == strcasecmp("true",  vec_split[0].c_str())) { vaild_ = true; value_ = true; }
        if (vec_split[0] == "0" || 0 == strcasecmp("false", vec_split[0].c_str())) { vaild_ = true; value_ = false;}
    }
    
    operator bool() const { return value_; }
    bool valid() const { return vaild_;}
    
private:
    bool value_;
    bool vaild_;
};
        
#endif /* STRING_CAST_H_ */
