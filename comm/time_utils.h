/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * utils.h
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#ifndef COMM_UTILS_H_
#define COMM_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
    
    uint64_t gettickcount();  // ms
    int64_t gettickspan(uint64_t _old_tick);    // ms
    uint64_t timeMs();
    
    uint64_t clock_app_monotonic();  // ms

#ifdef __cplusplus
}
#endif


#endif /* COMM_UTILS_H_ */
