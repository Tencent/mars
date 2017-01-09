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
//  ifaddr.h
//  comm
//
//  Created by yerungui on 14-10-31.
//

#ifndef __comm__ifaddr__
#define __comm__ifaddr__

#include <string>
#include <vector>
#include <string.h>

struct ifaddrinfo_ipv4_t
{
    ifaddrinfo_ipv4_t()
    {
        ifa_ip = 0;
        memset(ip, 0, sizeof(ip));
    }
    
    std::string  ifa_name;
    uint32_t     ifa_ip;
    char         ip[16];
};

struct ifaddrinfo_ip_t
{
    ifaddrinfo_ip_t()
    {
        ifa_family = 0;
        memset(ifa_ip, 0, sizeof(ifa_ip));
        memset(ip, 0, sizeof(ip));
        ifa_flags = 0;
    }

    uint8_t      ifa_family;
    std::string  ifa_name;
    uint32_t     ifa_ip[4];
    char         ip[64];
    unsigned int ifa_flags;

};

bool getifaddrs_ipv4(std::vector<ifaddrinfo_ipv4_t>& _addrs);
bool getifaddrs_ipv4(ifaddrinfo_ipv4_t& _addr);

bool getifaddrs_ipv4_lan(ifaddrinfo_ipv4_t& _addr);
bool getifaddrs_ipv4_lan(std::vector<ifaddrinfo_ipv4_t>& _addrs);


#define	IFF_BROADCAST_FILTER	0x2
#define	IFF_POINTOPOINT_FILTER	0x10
bool getifaddrs_ipv4_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter);
bool getifaddrs_ipv6_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter);

#endif /* defined(__comm__ifaddr__) */
