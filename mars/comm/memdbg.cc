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
    created:    2012/9/13
    filename:   memdbg.cc
    file ext:   cc
    author:     Ray
*********************************************************************/

#include "comm/assert/__assert.h"

#ifdef DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>

#ifdef ANDROID
#include <unistd.h>
#include "comm/android/callstack.h"
#endif
#include "mars/comm/xlogger/xlogger.h"

enum TMemoryType {
    ECType = 0,
    ECPPType = 1,
    ECPPArrayType = 2,
};

const char* gs_typename[] = {
    "ECType",
    "ECPPType",
    "ECPPArrayType"
};

struct CrtMem {
    const char*                _crtFileName;
    int                        _crtLine;
    const char*                _crtFuncName;
    void*                    _crtMemAddr;
    size_t                  _crtMemLen;
    TMemoryType                _crtMemoryType;
#ifdef ANDROID
    android::CallStack      _crtCallStack;
#endif
};

static std::vector<CrtMem> gs_CrtMemRoot;

static void __LogAlloc(void* _retp, size_t _size, const char* _filename, int _line, const char* _func, TMemoryType _type) {
    ASSERT(_retp);

    CrtMem crtMemCell;
    crtMemCell._crtFileName   = _filename;
    crtMemCell._crtLine       = _line;
    crtMemCell._crtFuncName   = _func;
    crtMemCell._crtMemAddr    = _retp;
    crtMemCell._crtMemLen     = _size;
    crtMemCell._crtMemoryType = _type;
#ifdef ANDROID
    crtMemCell._crtCallStack.update();
#endif

    gs_CrtMemRoot.push_back(crtMemCell);
}

static void __DeleteLogAlloc(void* _p, const char* _filename, int _line, const char* _func, TMemoryType _type) {
    for (std::vector<CrtMem>::iterator it = gs_CrtMemRoot.begin(); it != gs_CrtMemRoot.end(); ++it) {
        if (_p == it->_crtMemAddr) {
            if (_type != it->_crtMemoryType) {
                XMessage strstream;
                strstream << "\n[" << it->_crtFileName << ", " << it->_crtLine << ", " << it->_crtFuncName << "]"
                          << "alloc type is: " << gs_typename[it->_crtMemoryType] << "\n"
                          << "[" << _filename << ", " << _line << ", " << _func << "]"
                          << "dealloc type is: " << gs_typename[_type] << "\n";
#ifdef ANDROID
                android::CallStack  callstack;
                callstack.update();
                std::string str_stack = callstack.Format("dealloc type error stack dump", "memdbg");
                strstream << str_stack;
#endif
                __ASSERT(__FILE__, __LINE__, __FUNCTION__, strstream.String().c_str());
            }

            gs_CrtMemRoot.erase(it);
            break;
        }
    }
}

extern "C" {
    void* malloc_dbg(size_t _size, const char* _filename, int _line, const char* _func) {
        void* ret = malloc(_size);
        ASSERT(ret);

        if (ret)
            __LogAlloc(ret, _size, _filename, _line, _func, ECType);

        return ret;
    }

    void* calloc_dbg(size_t _num, size_t _size, const char* _filename, int _line, const char* _func) {
        void* ret = calloc(_num, _size);
        __LogAlloc(ret, _num * _size, _filename, _line, _func, ECType);
        return ret;
    }

    void* realloc_dbg(void* _oldpointer, size_t _newsize, const char* _filename, int _line, const char* _func) {
        if (NULL != _oldpointer)
            __DeleteLogAlloc(_oldpointer, _filename, _line, _func, ECType);

        void* ret = realloc(_oldpointer, _newsize);
        ASSERT(ret);

        if (ret)
            __LogAlloc(ret, _newsize, _filename, _line, _func, ECType);

        return ret;
    }

    void  free_dbg(void* _p, const char* _filename, int _line, const char* _func) {
        if (!_p)
            return;

        __DeleteLogAlloc(_p, _filename, _line, _func, ECType);
        free(_p);
    }

}  // "C"

void* operator new(size_t _size, const char* _filename, int _line, const char* _func) {
    void* ret = malloc(_size);
    ASSERT(ret);

    if (ret)
        __LogAlloc(ret, _size, _filename, _line, _func, ECPPType);

    return ret;
}

void* operator new[](size_t _size, const char* _filename, int _line, const char* _func) {
    void* ret = malloc(_size);
    ASSERT(ret);

    if (ret)
        __LogAlloc(ret, _size, _filename, _line, _func, ECPPArrayType);

    return ret;
}

void operator delete(void* _p) throw() {
    if (!_p)
        return;

    __DeleteLogAlloc(_p, __FILE__, __LINE__, __FUNCTION__, ECPPType);
    free(_p);
}

void operator delete[](void* _p) throw() {
    if (!_p)
        return;

    __DeleteLogAlloc(_p, __FILE__, __LINE__, __FUNCTION__, ECPPArrayType);
    free(_p);
}

void operator delete(void* _p, size_t /*_size*/) {
    if (!_p)
        return;

    __DeleteLogAlloc(_p, __FILE__, __LINE__, __FUNCTION__, ECPPType);
    free(_p);
}

void operator delete[](void* _p, size_t /*_size*/) {
    if (!_p)
        return;

    __DeleteLogAlloc(_p, __FILE__, __LINE__, __FUNCTION__, ECPPArrayType);
    free(_p);
}

#endif

extern "C" void DumpMemoryLeaks(void (* _pfunoutput)(const char*)) {
    ASSERT(_pfunoutput);
#ifdef DEBUG

    if (gs_CrtMemRoot.empty()) {
        _pfunoutput("No memory leaks detected!\n");
        return;
    }

    XMessage strstream;
    strstream << "Detected memory leaks!\n";
    strstream << "<--------------------------------Dumping objects-------------------------------->\n";

    for (std::vector<CrtMem>::iterator it = gs_CrtMemRoot.begin(); it != gs_CrtMemRoot.end(); ++it) {
        strstream << "[" << it->_crtFileName << ", " << it->_crtLine << ", " << it->_crtFuncName << "]"
                  << ": block at " << it->_crtMemAddr << ", type= " << gs_typename[it->_crtMemoryType] << ", " << it->_crtMemLen
                  << " bytes long\n Data <";

        size_t length = it->_crtMemLen < 100 ? it->_crtMemLen : 50;
        char buf[12];

        for (unsigned int i = 0; i < length; ++i) {
            snprintf(buf, sizeof(buf), "%c", *(((char*)it->_crtMemAddr) + i));
            strstream << buf;
        }

        strstream << ">\n";
#ifdef ANDROID
        strstream << it->_crtCallStack.Format("memory leaks stack dump", "memdbg");
#endif
        strstream << "<--------------------------------Separator---------------------------------->\n";
    }

    strstream << "<--------------------------------Dump end----------------------------------->\n";
    _pfunoutput(strstream.String().c_str());
#else
    _pfunoutput("Notice memdbg isn't running, because \"NO DEBUG\" was defined");
#endif
}
