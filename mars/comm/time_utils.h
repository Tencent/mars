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
