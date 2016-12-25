//
//  debug.c
//  PublicComponent
//
//  Created by 刘粲 on 13-12-2.
//  Copyright (c) 2013年 Tencent. All rights reserved.
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
