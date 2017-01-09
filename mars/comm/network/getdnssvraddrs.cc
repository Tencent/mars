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
//  getdnssvraddrs.cpp
//  comm
//
//  Created by yerungui on 16/1/19.
//

#include "comm/network/getdnssvraddrs.h"

#ifdef ANDROID

#include <sys/system_properties.h>
#include <string>

void getdnssvraddrs(std::vector<socket_address>& dnsServers) {
    char buf1[PROP_VALUE_MAX];
    char buf2[PROP_VALUE_MAX];
    __system_property_get("net.dns1", buf1);
    __system_property_get("net.dns2", buf2);
    dnsServers.push_back(socket_address(buf1, 0));  // 主DNS
    dnsServers.push_back(socket_address(buf2, 0));  // 备DNS
}

#elif defined __APPLE__
#include <TargetConditionals.h>
#include <resolv.h>
#define RESOLV_CONFIG_PATH ("/etc/resolv.conf")

#define NAME_SVR ("nameserver")
#define NAME_SVR_LEN (10)

void getdnssvraddrs(std::vector<socket_address>& _dnssvraddrs) {
    struct __res_state stat = {0};
    res_ninit(&stat);
    union res_sockaddr_union addrs[MAXNS] = {0};
    int count = res_getservers(const_cast<res_state>(&stat), addrs, MAXNS);
    for (int i = 0; i < count; ++i) {
        if (AF_INET == addrs[i].sin.sin_family) {
            _dnssvraddrs.push_back(socket_address(addrs[i].sin));
        } else if (AF_INET6 == addrs[i].sin.sin_family) {
            _dnssvraddrs.push_back(socket_address(addrs[i].sin6));
        }
            
    }
    
    res_ndestroy(&stat);
}

#elif defined _WIN32
#include <stdio.h>
#include <windows.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")

void getdnssvraddrs(std::vector<socket_address>& _dnssvraddrs) {
    FIXED_INFO fi;
    ULONG ulOutBufLen = sizeof(fi);
    
    if (::GetNetworkParams(&fi, &ulOutBufLen) != ERROR_SUCCESS) {
        return;
    }
    
    IP_ADDR_STRING* pIPAddr = fi.DnsServerList.Next;
    
    while (pIPAddr != NULL) {
		_dnssvraddrs.push_back(socket_address(pIPAddr->IpAddress.String) );
        pIPAddr = pIPAddr->Next;
    }
    
    return;
}
#else
void getdnssvraddrs(std::vector<socket_address>& _dnssvraddrs) {
}
#endif
