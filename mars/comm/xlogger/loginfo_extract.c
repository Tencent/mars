/*
 * loginfo_extract.c
 *
 *  Created on: 2016年10月13日
 *      Author: yanguoyue
 */

#include "loginfo_extract.h"

#include <string.h>
#include <stddef.h>


const char* ExtractFileName(const char* _path) {
    if (NULL == _path) return "";

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


void ExtractFunctionName(const char* _func, char* _func_ret, int _len) {
    if (NULL == _func)return;
    
    const char* start = _func;
    const char* end = NULL;
    const char* pos = _func;
    
    while ('\0' != *pos) {
        if (NULL == end && ' ' == *pos) {
            start = ++pos;
            continue;
        }
        
        if (':' == *pos && ':' == *(pos+1)) {
            pos += 2;
            start = pos;
            continue;
        }
        
        if ('(' == *pos) {
            end = pos;
        } else if (NULL != start && (':' == *pos || ']' == *pos)) {
            end = pos;
            break;
        }
        ++pos;
    }
    
    
    if (NULL == start || NULL == end || start + 1 >= end) {
        strncpy(_func_ret, _func, _len);
        _func_ret[_len - 1] = '\0';
        return;
    }
    
    ptrdiff_t len = end - start;
    --_len;
    len = _len < len ? _len : len;
    memcpy(_func_ret, start, len);
    _func_ret[len] = '\0';
}
