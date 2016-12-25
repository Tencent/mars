/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * bootrun.h
 *
 *  Created on: 2014-7-9
 *      Author: yerungui
 */

#ifndef COMM_BOOTRUN_H_
#define COMM_BOOTRUN_H_

#include <stdlib.h>
#include "comm/compiler_util.h"

#ifdef __cplusplus
extern "C" {
#endif

__inline int boot_run_atstartup(void (*func)(void)) { func(); return 0;}
__inline int boot_run_atexit(void (*func)(void)) { return atexit(func);}

#ifdef __cplusplus
}
#endif

#define BOOT_RUN_STARTUP(func) VARIABLE_IS_NOT_USED static int __anonymous_run_variable_startup_##func = boot_run_atstartup(func)
#define BOOT_RUN_EXIT(func) VARIABLE_IS_NOT_USED static int __anonymous_run_variable_exit_##func = boot_run_atexit(func)

#endif /* COMM_BOOTRUN_H_ */
