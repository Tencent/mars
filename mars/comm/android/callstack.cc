/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/********************************************************************
    modify:    2012/9/17
    author:     Ray
*********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include "callstack.h"
#include "comm/thread/lock.h"

// #define HAVE_DLADDR 1

#if HAVE_DLADDR
#include <dlfcn.h>
#endif

#define HAVE_CXXABI 0

#if HAVE_CXXABI
#include <cxxabi.h>
#endif


// #include <utils/Log.h>
// #include <utils/Errors.h>
// #include <utils/threads.h>

#include <unwind.h>

typedef struct {
    size_t count;
    size_t ignore;
    const void** addrs;
} stack_crawl_state_t;

static _Unwind_Reason_Code trace_function(_Unwind_Context* context, void* arg) {
    stack_crawl_state_t* state = (stack_crawl_state_t*)arg;

    if (state->count) {
        void* ip = NULL;// = (void*)_Unwind_GetIP(context);

#if defined(__aarch64__)

		//_Unwind_GetGR crash
		return _URC_NO_REASON;
#elif !defined(__arm__)
      // ip = (void*)_Unwind_GetGR(context, 15);
		return _URC_NO_REASON;
#else
        _Unwind_VRS_Get(context, _UVRSC_CORE, 15, _UVRSD_UINT32, &ip);
#endif

        if (ip) {
            if (state->ignore) {
                state->ignore--;
            } else {
                state->addrs[0] = ip;
                state->addrs++;
                state->count--;
				return _URC_NO_REASON;
            }
        } else {
			return _URC_END_OF_STACK;
		}


    } 

	if (state->count <= 0) {
		return _URC_END_OF_STACK;
	}
	
	return _URC_NO_REASON;


}

#ifdef __GABIXX_UNWIND_H__
extern "C"
{
    typedef _Unwind_Reason_Code(*_Unwind_Trace_Fn)(_Unwind_Context* context, void* arg);
    extern _Unwind_Reason_Code _Unwind_Backtrace(_Unwind_Trace_Fn, void*);
}
#endif

static int backtrace(const void** addrs, size_t ignore, size_t size) {
    stack_crawl_state_t state;
    state.count = size;
    state.ignore = ignore;
    state.addrs = addrs;
    _Unwind_Backtrace(trace_function, (void*)&state);
    return size - state.count;
}


void android_callstack(char* _out, unsigned int _len) {
    android::CallStack cs;
    cs.update(2);
    std::string strstack = cs.Format();
    strncpy(_out, strstack.c_str(), _len);
}

/*****************************************************************************/
namespace android {

/*****************************************************************************/

static
const char* lookup_symbol(const void* addr, void** offset, char* name, size_t bufSize) {
#if HAVE_DLADDR
    Dl_info info;

    if (dladdr(addr, &info)) {
        *offset = info.dli_saddr;
        return info.dli_sname;
    }

#endif
    return NULL;
}

static
int32_t linux_gcc_demangler(const char* mangled_name, char* unmangled_name, size_t buffersize) {
    size_t out_len = 0;
#if HAVE_CXXABI
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangled_name, 0, &out_len, &status);

    if (status == 0) {
        // OK
        if (out_len < buffersize)
          memcpy(unmangled_name, demangled, out_len);
        else
          out_len = 0;

        free(demangled);
    } else {
        out_len = 0;
    }

#endif
    return out_len;
}

/*****************************************************************************/

class MapInfo {
    struct mapinfo {
        struct mapinfo* next;
        uint64_t start;
        uint64_t end;
        char name[];
    };

    const char* map_to_name(uint64_t pc, const char* def, uint64_t* start) {
        mapinfo* mi = getMapInfoList();

        while (mi) {
            if ((pc >= mi->start) && (pc < mi->end)) {
                if (start)
                    *start = mi->start;

                return mi->name;
            }

            mi = mi->next;
        }

        if (start)
            *start = 0;

        return def;
    }

    mapinfo* parse_maps_line(char* line) {
        mapinfo* mi;
        int len = strlen(line);

        if (len < 1) return 0;

        line[--len] = 0;

        if (len < 50) return 0;

        if (line[20] != 'x') return 0;

        mi = (mapinfo*)malloc(sizeof(mapinfo) + (len - 47));

        if (mi == 0) return 0;

        mi->start = strtoull(line, 0, 16);
        mi->end = strtoull(line + 9, 0, 16);
        mi->next = 0;
        strcpy(mi->name, line + 49);  // **CPPLINT SKIP**
        return mi;
    }

    mapinfo* getMapInfoList() {
        ScopedLock _l(mLock);
        if (milist == 0) {
            char data[1024];
            FILE* fp;
            snprintf(data, sizeof(data), "/proc/%d/maps", getpid());
            fp = fopen(data, "r");

            if (fp) {
                while (fgets(data, 1024, fp)) {
                    mapinfo* mi = parse_maps_line(data);

                    if (mi) {
                        mi->next = milist;
                        milist = mi;
                    }
                }

                fclose(fp);
            }
        }

        return milist;
    }
    mapinfo*    milist;
    Mutex       mLock;
    static MapInfo sMapInfo;

  public:
    MapInfo()
        : milist(0) {
    }

    ~MapInfo() {
        while (milist) {
            mapinfo* next = milist->next;
            free(milist);
            milist = next;
        }
    }

    static const char* mapAddressToName(const void* pc, const char* def,
                                        void const** start) {
        uint64_t s;
        char const* name = sMapInfo.map_to_name(uint64_t(uintptr_t(pc)), def, &s);

        if (start) {
            *start = (void*)s;
        }

        return name;
    }
};

/*****************************************************************************/

MapInfo MapInfo::sMapInfo;

/*****************************************************************************/

CallStack::CallStack()
    : count_(0)
    , tid_(0) {
    stack_[0] = 0;
}

CallStack::CallStack(const CallStack& rhs)
    : count_(rhs.count_)
    , tid_(rhs.tid_) {
    if (count_) {
        memcpy(stack_, rhs.stack_, count_ * sizeof(void*));
    }
}

CallStack::~CallStack() {
}

CallStack& CallStack::operator = (const CallStack& rhs) {
    count_ = rhs.count_;
    tid_ = rhs.tid_;

    if (count_) {
        memcpy(stack_, rhs.stack_, count_ * sizeof(void*));
    }

    return *this;
}

bool CallStack::operator == (const CallStack& rhs) const {
    if (count_ != rhs.count_)
        return false;

    return !count_ || (memcmp(stack_, rhs.stack_, count_ * sizeof(void*)) == 0);
}

bool CallStack::operator != (const CallStack& rhs) const {
    return !operator == (rhs);
}

bool CallStack::operator < (const CallStack& rhs) const {
    if (count_ != rhs.count_)
        return count_ < rhs.count_;

    return memcmp(stack_, rhs.stack_, count_ * sizeof(void*)) < 0;
}

bool CallStack::operator >= (const CallStack& rhs) const {
    return !operator < (rhs);
}

bool CallStack::operator > (const CallStack& rhs) const {
    if (count_ != rhs.count_)
        return count_ > rhs.count_;

    return memcmp(stack_, rhs.stack_, count_ * sizeof(void*)) > 0;
}

bool CallStack::operator <= (const CallStack& rhs) const {
    return !operator > (rhs);
}

const void* CallStack::operator [](int index) const {
    if (index >= int(count_))
        return 0;

    return stack_[index];
}


void CallStack::clear() {
    count_ = 0;
    tid_ = 0;
}

void CallStack::update(int32_t ignoreDepth, int32_t maxDepth) {
    tid_ = gettid();

    if (maxDepth > kMaxDepth)
        maxDepth = kMaxDepth;

    count_ = backtrace(stack_, ignoreDepth, maxDepth);
}

// Return the stack frame name on the designated level
std::string CallStack::toStringSingleLevel(const char* prefix, int32_t level) const {
    std::string res;
    char namebuf[1024];
    char tmp[256];
    char tmp1[32];
    char tmp2[32];
    void* offs;

    const void* ip = stack_[level];

    if (!ip) return res;

    if (prefix) res.append(prefix);

    snprintf(tmp1, 32, "#%02d  ", level);
    res.append(tmp1);

    const char* name = lookup_symbol(ip, &offs, namebuf, sizeof(namebuf));

    if (name) {
        if (linux_gcc_demangler(name, tmp, 256) != 0)
            name = tmp;

        snprintf(tmp1, 32, "0x%p: <", ip);
        snprintf(tmp2, 32, ">+0x%p", offs);
        res.append(tmp1);
        res.append(name);
        res.append(tmp2);
    } else {
        void const* start = 0;
        name = MapInfo::mapAddressToName(ip, "<unknown>", &start);
        snprintf(tmp, 256, "pc %08lx  %s",
                 long(uintptr_t(ip) - uintptr_t(start)), name);
        res.append(tmp);
    }

    res.append("\n");

    return res;
}

// Dump a stack trace to the log
// void CallStack::dump(const char* prefix) const
//{
//    /*
//     * Sending a single long log may be truncated since the stack levels can
//     * get very deep. So we request function names of each frame individually.
//     */
//    for (int i=0; i<int(mCount); i++) {
//        LOGD("%s", toStringSingleLevel(prefix, i).string());
//    }


// Return a string (possibly very long) containing the complete stack trace
std::string CallStack::toString(const char* prefix) const {
    std::string res;

    for (int i = 0; i < int(count_); i++) {
        res.append(toStringSingleLevel(prefix, i));
    }

    return res;
}

std::string CallStack::Format(const char*  _str_build_fingerprint, const char* _str_process_name) const {
    _str_build_fingerprint = _str_build_fingerprint ? _str_build_fingerprint : "";
    _str_process_name = _str_process_name ? _str_process_name : "";

    std::stringstream strstream;
    // 符合NDK-STACK符号寻找的格式
    strstream << "\n*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n";
    strstream << "Build fingerprint: " << _str_build_fingerprint << "\n";
    strstream << "pid: " << getpid() << ", tid: " << TID() << "  >>> " << _str_process_name << " <<<\n";
    strstream << toString();
    return strstream.str();
}

/*****************************************************************************/

};  // namespace android
