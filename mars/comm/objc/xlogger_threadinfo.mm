// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under
// the License.
//
//  xloggr_threadinfo.m
//  MicroMessenger
//
//  Created by yerungui on 13-3-13.
//

#import <Foundation/NSThread.h>

#include <pthread.h>
#include <unistd.h>

extern "C" {
static pthread_t s_mainpthread = pthread_self();

intmax_t xlogger_pid() {
    static intmax_t pid = getpid();
    return pid;
}

intmax_t xlogger_tid() {
    uint64_t tid = 0;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<intmax_t>(tid);
}

intmax_t xlogger_maintid() {
    static intmax_t s_maintid = 0;
    if (s_maintid == 0) {
        uint64_t tid = 0;
        pthread_threadid_np(s_mainpthread, &tid);
        s_maintid = static_cast<intmax_t>(tid);
    }
    return s_maintid;
}
}

void comm_export_symbols_5() {}
