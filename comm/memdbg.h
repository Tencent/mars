/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/********************************************************************
    created:    2012/9/13
    filename:     memdbg.h
    file ext:    h
    author:        Ray
*********************************************************************/

#ifndef COMM_MEMDBG_H_
#define COMM_MEMDBG_H_

/**
 *注意:
 *1.尽量不要把这头文件include到别外的头文件内
 *2.在.c或者.cpp文件内，应该把头文件放到其他文件之后include
 */

#ifdef DEBUG
#include <stddef.h>

#ifndef _MSC_VER
    #define __MEMDBG_FUNCTION__       __PRETTY_FUNCTION__
#else
    // Definitely, VC6 not support this feature!
    #if _MSC_VER > 1200
        #define __MEMDBG_FUNCTION__   __FUNCTION__
    #else
        #define __MEMDBG_FUNCTION__   "N/A"
        #warning "'__FUNCTION__' is not supported by this compiler"
    #endif
#endif

#if defined(__cplusplus)

void* operator new(size_t _size, const char* _filename, int _line, const char* _func);
void* operator new[](size_t _size, const char* _filename, int _line, const char* _func);

void operator delete(void* _p);
void operator delete[](void* _p);

void operator delete(void* _p, size_t _size);
void operator delete[](void* _p, size_t _size);

#endif

#ifdef  __cplusplus
extern "C" {
#endif

void* malloc_dbg(size_t _size, const char* _filename, int _line, const char* _func);
void* calloc_dbg(size_t _num, size_t _size, const char* _filename, int _line, const char* _func);
void* realloc_dbg(void* _oldpointer, size_t _newsize, const char* _filename, int _line, const char* _func);
void  free_dbg(void* _p, const char* _filename, int _line, const char* _func);

#ifdef  __cplusplus
}
#endif

#define   malloc(s)             malloc_dbg(s, __FILE__, __LINE__, __MEMDBG_FUNCTION__)
#define   calloc(c, s)          calloc_dbg(c, s, __FILE__, __LINE__, __MEMDBG_FUNCTION__)
#define   realloc(p, s)         realloc_dbg(p, s, __FILE__, __LINE__, __MEMDBG_FUNCTION__)
#define   free(p)               free_dbg(p, __FILE__, __LINE__, __MEMDBG_FUNCTION__)

#define  new new(__FILE__, __LINE__, __MEMDBG_FUNCTION__)
// #define  new(poject) new(poject, __FILE__, __LINE__, __MEMDBG_FUNCTION__)
#endif  //

#ifdef  __cplusplus
extern "C" {
#endif

void DumpMemoryLeaks(void (*)(const char*));

#ifdef  __cplusplus
}
#endif

#endif  // COMM_MEMDBG_H_
