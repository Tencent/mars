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
//  IPv6_only.hpp
//  comm
//
//  Created by yerungui on 16/1/14.
//

#ifndef __ip_type__
#define __ip_type__

#ifdef __cplusplus
extern "C" {
#endif

enum TLocalIPStack {
    ELocalIPStack_None = 0,
    ELocalIPStack_IPv4 = 1,
    ELocalIPStack_IPv6 = 2,
    ELocalIPStack_Dual = 3,
};
    
const char* const TLocalIPStackStr[] = {
    "ELocalIPStack_None",
    "ELocalIPStack_IPv4",
    "ELocalIPStack_IPv6",
    "ELocalIPStack_Dual",
};

TLocalIPStack local_ipstack_detect();
    
#ifdef __cplusplus
}
#endif

#include <string>
TLocalIPStack local_ipstack_detect_log(std::string& _log);


#endif /* __ip_type__ */
