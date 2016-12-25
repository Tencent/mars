//
//  IPv6_only.hpp
//  comm
//
//  Created by rayye on 16/1/14.
//  Copyright © 2016年 Tencent. All rights reserved.
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