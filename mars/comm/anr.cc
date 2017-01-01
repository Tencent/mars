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
 * anr.cpp
 *
 *  Created on: 2014-7-18
 *      Author: yerungui
 */

#include "anr.h"

#include <vector>
#include <algorithm>

#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#include "comm/thread/thread.h"
#include "comm/thread/lock.h"
#include "comm/time_utils.h"
#include "comm/xlogger/xlogger.h"

#ifdef ANDROID
#include "android/fatal_assert.h"
#endif

#if !defined(ANDROID) && !defined(__APPLE__)
#define ANR_CHECK_DISABLE
#endif

#ifndef ANR_CHECK_DISABLE

boost::signals2::signal<void (bool _iOS_style)>& GetSignalCheckHit() {
	static boost::signals2::signal<void (bool _iOS_style)> SignalCheckHit;
	return SignalCheckHit;
}

namespace {

struct check_content {
    uintptr_t ptr;
    const char* file;
    const char* func;
    int line;
    int timeout;
    intmax_t tid;

    uint64_t start_time;
    uint64_t end_time;

    uint64_t start_tickcount;
    uint64_t used_cpu_time; //ms

    bool operator<(const check_content& _ref) const {
        return end_time > _ref.end_time;
    }
};

static std::vector<check_content> sg_check_heap;
static Mutex             sg_mutex;
static Condition          sg_cond;
static bool               sg_exit = false;


static void __unregister_anr_impl(uintptr_t _ptr) {
    for (std::vector<check_content>::iterator it = sg_check_heap.begin(); it != sg_check_heap.end(); ++it) {
        if (it->ptr == _ptr) {
            sg_check_heap.erase(it);
            std::make_heap(sg_check_heap.begin(), sg_check_heap.end());
            break;
        }
    }
}

static void __unregister_anr(uintptr_t _ptr) {
    ScopedLock lock(sg_mutex);
    __unregister_anr_impl(_ptr);
    sg_cond.notifyAll(lock);
}

static void __register_anr(uintptr_t _ptr, const char* _file, const char* _func, int _line, int _timeout) {
    ScopedLock lock(sg_mutex);
    __unregister_anr_impl(_ptr);

    if (0 >= _timeout) return;

    check_content ch = {_ptr, _file, _func, _line, _timeout, xlogger_tid(), clock_app_monotonic(), 0, gettickcount(), 0/*init cpu_time*/};
    ch.end_time = ch.start_time + ch.timeout;

    sg_check_heap.push_back(ch);
    std::push_heap(sg_check_heap.begin(), sg_check_heap.end());

    sg_cond.notifyAll(lock);
}


static const int64_t kEachRoundSleepTime = 15*1000;//ms
static const int64_t kTimeDeviation = 500;
static bool iOS_style = false;

static void __anr_checker_thread() {
    while (true) {
        ScopedLock lock(sg_mutex);

    	uint64_t round_tick_start = clock_app_monotonic();
    	uint64_t use_cpu_time_1 = (uint64_t)(((double)clock()/CLOCKS_PER_SEC)*1000); //ms

        if (sg_exit) return;

        int64_t wait_timeout = 0;
        bool is_wait_timeout = false;

        if (sg_check_heap.empty()) {
        	//wait_timeout = kEachRoundSleepTime;
            sg_cond.wait(lock);
            //is_wait_timeout = (ETIMEDOUT==ret);
        } else {
            wait_timeout = (sg_check_heap.front().end_time - clock_app_monotonic());
            if (wait_timeout<0) {
                xwarn2("@%p", (void*)sg_check_heap.front().ptr)(TSF"wait_timeout:%_, end_time:%_, used_cpu_time:%_, now:%_, anr_checker_size:%_", wait_timeout,
                      sg_check_heap.front().end_time, sg_check_heap.front().used_cpu_time, clock_app_monotonic(), sg_check_heap.size());
                wait_timeout = 0;
            }
            wait_timeout = std::min(wait_timeout, kEachRoundSleepTime);
            int ret = sg_cond.wait(lock, wait_timeout);
            //is_wait_timeout = true;
            is_wait_timeout = (wait_timeout>0 && ETIMEDOUT==ret);
        }

        int64_t round_tick_elapse = clock_app_monotonic()-round_tick_start;
        uint64_t use_cpu_time_2 = (uint64_t)(((double)clock()/CLOCKS_PER_SEC) * 1000); //ms
        if (is_wait_timeout && round_tick_elapse > (wait_timeout+kTimeDeviation)) {
            xwarn2("@%p", (void*)sg_check_heap.front().ptr)(TSF"now:%_, round_tick_start:%_, round_tick_elapse:%_, wait_timeout:%_, round cputime:%_, anr_checker_size:%_", clock_app_monotonic(), round_tick_start, round_tick_elapse, wait_timeout, use_cpu_time_2-use_cpu_time_1, sg_check_heap.size());
            iOS_style = true;
        }
        for (std::vector<check_content>::iterator it = sg_check_heap.begin(); it != sg_check_heap.end(); ++it) {
            it->used_cpu_time += (use_cpu_time_2-use_cpu_time_1);
        }
        bool check_hit = false;
        if (iOS_style) {
            if (!sg_check_heap.empty() && (uint64_t)sg_check_heap.front().timeout <= sg_check_heap.front().used_cpu_time) {
                check_hit = true;
                GetSignalCheckHit()(true);
                xassert2(sg_check_heap.front().end_time <= clock_app_monotonic(),
                         "end_time:%" PRIu64", now:%" PRIu64", anr_checker_size:%d, @%p", sg_check_heap.front().end_time, clock_app_monotonic(), (int)sg_check_heap.size(), (void*)sg_check_heap.front().ptr); //old logic is strict than new logic
            }
        } else {
            if (!sg_check_heap.empty()  && sg_check_heap.front().end_time <= clock_app_monotonic()) {
                check_hit = true;
                GetSignalCheckHit()(false);
            }
        }


        if (!sg_check_heap.empty() && check_hit) {
            check_content& front = sg_check_heap.front();
            __ASSERT2(front.file, front.line, front.func, "anr dead lock", "timeout:%d, tid:%" PRIu64 ", runing time:%" PRIu64 ", real time:%" PRIu64 ", used_cpu_time:%" PRIu64 ", iOS_style:%s, anr_checker_size:%d, @%p",
                    front.timeout, front.tid, clock_app_monotonic() - front.start_time, gettickcount() - front.start_tickcount, front.used_cpu_time, iOS_style?"true":"false", (int)sg_check_heap.size(), (void*)sg_check_heap.front().ptr);
#ifdef ANDROID
            __FATAL_ASSERT2(front.file, front.line, front.func, "anr dead lock", "timeout:%d, tid:%" PRIu64 ", runing time:%" PRIu64 ", real time:%" PRIu64 ", used_cpu_time:%" PRIu64 ", iOS_style:%s, anr_checker_size:%d, @%p",
                    front.timeout, front.tid, clock_app_monotonic() - front.start_time, gettickcount() - front.start_tickcount, front.used_cpu_time, iOS_style?"true":"false", (int)sg_check_heap.size(), (void*)sg_check_heap.front().ptr);
#endif
            std::pop_heap(sg_check_heap.begin(), sg_check_heap.end());
            sg_check_heap.pop_back();
        }
    }
}

static Thread              sg_thread(&__anr_checker_thread);
static class startup {
  public:
    startup() {
        sg_thread.start();
    }

    ~startup() {
        ScopedLock lock(sg_mutex);
        sg_exit = true;
        sg_cond.notifyAll(lock);
        lock.unlock();

        sg_thread.join();
    }
} __startup;
}  // namespace

#endif

scope_anr::scope_anr(const char* _file, const char* _func, int _line)
    : file_(_file), func_(_func), line_(_line)
{}

scope_anr::~scope_anr() {
#ifndef ANR_CHECK_DISABLE
    __unregister_anr(reinterpret_cast<uintptr_t>(this));
#endif
}


void scope_anr::anr(int _timeout) {
#ifndef ANR_CHECK_DISABLE
    __register_anr(reinterpret_cast<uintptr_t>(this), file_, func_, line_, _timeout);
#endif
}

