// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/********************************************************************
    created:    2009/11/16
    created:    16:11:2009   18:26
    filename:     marcotoolkit.h
    file base:    marcotoolkit
    file ext:    h
    author:        yerungui

*********************************************************************/
#ifndef COMM_MARCOTOOLKIT_H_
#define COMM_MARCOTOOLKIT_H_

#pragma once

/* MT_NULL */
#if !defined(MMT_NULL)
#define MT_NULL 0
#endif

/* MT_DIM helper */
#if !defined(MT_DIM)
#if !defined(__cplusplus)
#define MT_ARRAYCOUNT(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++" {
    template <typename _CountofType, size_t _SizeOfArray>
    char (*__countof_helper(_CountofType(&_Array)[_SizeOfArray]))[_SizeOfArray];
#define MT_ARRAYCOUNT(_Array) sizeof(*__countof_helper(_Array))
}
#endif
#endif

#ifdef __cplusplus
template<class T>
inline T MT_MAX(const T& a, const T& b) {
    if (a < b) return b;

    return a;
}

template<class T>
inline T MT_MIN(const T& a, const T& b) {
    if (b < a) return b;

    return a;
}

#else
#define MT_MAX(a, b)    (((a) > (b)) ? (a) : (b))
#define MT_MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif

/* Define MT_OFFSETOF macro */
#ifdef __cplusplus

#ifdef  _WIN64
#define MT_OFFSETOF(s, m)   (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
#else
#define MT_OFFSETOF(s, m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif

#else

#ifdef  _WIN64
#define MT_OFFSETOF(s, m)   (size_t)( (ptrdiff_t)&(((s *)0)->m) )
#else
#define MT_OFFSETOF(s, m)   (size_t)&(((s *)0)->m)
#endif

#endif    /* __cplusplus */


/* Define MT_DELETE macro */
#if !defined(MT_DELETE)
#if defined(__cplusplus)
#define MT_DELETE(x)\
    if (MT_NULL != x) {\
        delete x;\
        x = MT_NULL;\
    }
#endif
#endif

/* Define MT_DELETE_ARRAY macro */
#if !defined(MT_DELETE_ARRAY)
#if defined(__cplusplus)
#define MT_DELETE_ARRAY(x)\
    if (MT_NULL != x) {\
        delete[] x;\
        x = MT_NULL;\
    }
#endif
#endif

/* Define MT_FREE macro */
#if !defined(MT_FREE)
#define MT_FREE(x)\
    if (MT_NULL != x) {\
        free(x);\
        x = MT_NULL;\
    }
#endif

#endif
