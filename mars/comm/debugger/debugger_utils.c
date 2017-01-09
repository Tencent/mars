// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  debug.c
//  PublicComponent
//
//  Created by 刘粲 on 13-12-2.
//

#include <unistd.h>
#include <sys/sysctl.h>


int isDebuggerPerforming() {
    struct kinfo_proc infos_process;
    size_t size_info_proc = sizeof(infos_process);
    pid_t pid_process = getpid(); // pid of the current process
    //
    int mib[] = {CTL_KERN,        // Kernel infos
        KERN_PROC,       // Search in process table
        KERN_PROC_PID,   // the process with pid =
        pid_process};    // pid_process
    //
    //Retrieve infos for current process in infos_process
    int ret = sysctl(mib, 4, &infos_process, &size_info_proc, NULL, 0);
    if (ret) return 0;             // sysctl failed
    //
    struct extern_proc process = infos_process.kp_proc;
    int flags = process.p_flag;
    return flags & P_TRACED;       // value of the debug flag
}
