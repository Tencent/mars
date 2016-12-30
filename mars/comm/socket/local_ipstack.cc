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
//  IPv6_only.cpp
//  comm
//
//  Created by yerungui on 16/1/14.
//

#include "local_ipstack.h"
#include <vector>
#include "xlogger/xlogger.h"
#if (defined(__APPLE__) || defined(ANDROID))
#include <strings.h>
#include "socket/unix_socket.h"
#include "network/getifaddrs.h"
#if defined(__APPLE__)
#include "network/getgateway.h"
#include "network/getdnssvraddrs.h"
#include "platform_comm.h"
#endif


typedef union sockaddr_union {
    struct sockaddr     generic;
    struct sockaddr_in  in;
    struct sockaddr_in6 in6;
} sockaddr_union;

/*
 * Connect a UDP socket to a given unicast address. This will cause no network
 * traffic, but will fail fast if the system has no or limited reachability to
 * the destination (e.g., no IPv4 address, no IPv6 default route, ...).
 */
static const unsigned int kMaxLoopCount = 10;
static int
_test_connect(int pf, struct sockaddr *addr, size_t addrlen, struct sockaddr* local_addr, socklen_t local_addr_len) {
    int s = socket(pf, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
        return 0;
    int ret;
    unsigned int loop_count = 0;
    do {
        ret = connect(s, addr, addrlen);
    } while (ret < 0 && errno == EINTR && loop_count++<kMaxLoopCount);
    if (loop_count>=kMaxLoopCount) {
    	xerror2(TSF"connect error. loop_count = %_", loop_count);
    }
    int success = (ret == 0);
    if (success) {
        memset(local_addr, 0, sizeof(struct sockaddr_storage));
        getsockname(s, local_addr, &local_addr_len);
    }
    loop_count = 0;
    do {
        ret = close(s);
    } while (ret < 0 && errno == EINTR && loop_count++<kMaxLoopCount);
    if (loop_count>=kMaxLoopCount) {
    	xerror2(TSF"close error. loop_count = %_", loop_count);
    }
    return success;
}

/*
 * The following functions determine whether IPv4 or IPv6 connectivity is
 * available in order to implement AI_ADDRCONFIG.
 *
 * Strictly speaking, AI_ADDRCONFIG should not look at whether connectivity is
 * available, but whether addresses of the specified family are "configured
 * on the local system". However, bionic doesn't currently support getifaddrs,
 * so checking for connectivity is the next best thing.
 */
static int
_have_ipv6(struct sockaddr* local_addr, socklen_t local_addr_len) {
#ifdef __APPLE__
    static const struct sockaddr_in6 sin6_test = {
        .sin6_len = sizeof(sockaddr_in6),
        .sin6_family = AF_INET6,
        .sin6_port = 80,
        .sin6_flowinfo = 0,
        .sin6_scope_id = 0,
        .sin6_addr.s6_addr = {  // 2000::
            0x20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    sockaddr_union addr = { .in6 = sin6_test };
#else
	 static  struct sockaddr_in6 sin6_test;
	    sin6_test.sin6_family = AF_INET6;
	    sin6_test.sin6_port = 80;
		sin6_test.sin6_flowinfo = 0;
		sin6_test.sin6_scope_id = 0;
		bzero(sin6_test.sin6_addr.s6_addr, sizeof(sin6_test.sin6_addr.s6_addr));
		sin6_test.sin6_addr.s6_addr[0] = 0x20;
	    sockaddr_union addr = { in6:sin6_test };
#endif
    return _test_connect(PF_INET6, &addr.generic, sizeof(addr.in6), local_addr, local_addr_len);
}

static int
_have_ipv4(struct sockaddr* local_addr, socklen_t local_addr_len) {
#ifdef __APPLE__
    static const struct sockaddr_in sin_test = {
        .sin_len = sizeof(sockaddr_in),
        .sin_family = AF_INET,
        .sin_port = 80,
        .sin_addr.s_addr = htonl(0x08080808L),  // 8.8.8.8
    };
    sockaddr_union addr = { .in = sin_test };
#else
    static struct sockaddr_in sin_test = {
        sin_family:AF_INET,
        sin_port:80,
    };
    sin_test.sin_addr.s_addr = htonl(0x08080808L); // 8.8.8.8
    sockaddr_union addr = { in:sin_test };
#endif
    return _test_connect(PF_INET, &addr.generic, sizeof(addr.in), local_addr, local_addr_len);
}

bool two_addrs_on_one_interface(sockaddr* first_addr, sockaddr* second_addr) {
    char ip1[64] = {0};
    char ip2[64] = {0};
    std::string ip1_ifname, ip2_ifname;
    if (AF_INET == first_addr->sa_family) {
        socket_inet_ntop(AF_INET, &(((sockaddr_in*)first_addr)->sin_addr), ip1, sizeof(ip1));
    } else if (AF_INET6 == first_addr->sa_family) {
        socket_inet_ntop(AF_INET6, &(((sockaddr_in6*)first_addr)->sin6_addr), ip1, sizeof(ip1));
    }
    if (AF_INET == second_addr->sa_family) {
        socket_inet_ntop(AF_INET, &(((sockaddr_in*)second_addr)->sin_addr), ip2, sizeof(ip2));
    } else if (AF_INET6 == second_addr->sa_family) {
        socket_inet_ntop(AF_INET6, &(((sockaddr_in6*)second_addr)->sin6_addr), ip2, sizeof(ip2));
    }
    
    std::vector<ifaddrinfo_ip_t> v4_addrs, v6_addrs;
    if (getifaddrs_ipv4_filter(v4_addrs, 0)) {
        for (size_t i=0; i<v4_addrs.size(); ++i) {
            if (!ip1_ifname.empty() && !ip2_ifname.empty()) break;
            if (0==strncmp(ip1, v4_addrs[i].ip, sizeof(ip1))) {
                ip1_ifname = v4_addrs[i].ifa_name;
            }
            if (0==strncmp(ip2, v4_addrs[i].ip, sizeof(ip2))) {
                ip2_ifname = v4_addrs[i].ifa_name;
            }
        }
    }
    if (getifaddrs_ipv6_filter(v6_addrs, 0)) {
        for (size_t i=0; i<v6_addrs.size(); ++i) {
            if (!ip1_ifname.empty() && !ip2_ifname.empty()) break;
            if (0==strncmp(ip1, v6_addrs[i].ip, sizeof(ip1))) {
                ip1_ifname = v6_addrs[i].ifa_name;
            }
            if (0==strncmp(ip2, v6_addrs[i].ip, sizeof(ip2))) {
                ip2_ifname = v6_addrs[i].ifa_name;
            }
        }
    }
    if (!ip1_ifname.empty() && !ip2_ifname.empty() && 0==ip1_ifname.compare(ip2_ifname)) return true;
    
    
    return false;
}

TLocalIPStack __local_ipstack_detect(std::string& _log) {
    XMessage detail(_log);
    detail("local_ipstack_detect ");
#ifdef __APPLE__
    in6_addr addr6_gateway = {0};
    if (0 != getdefaultgateway6(&addr6_gateway)){
        detail("getdefaultgateway6 fail");
        return ELocalIPStack_IPv4;
    }
    
    if (IN6_IS_ADDR_UNSPECIFIED(&addr6_gateway)) {
        detail("getdefaultgateway6 IN6_IS_ADDR_UNSPECIFIED");
        return ELocalIPStack_IPv4;
    }
    
    in_addr addr_gateway = {0};
    if (0 != getdefaultgateway(&addr_gateway)) {
        detail("getdefaultgateway fail");
        return ELocalIPStack_IPv6;
    }
    if (INADDR_NONE == addr_gateway.s_addr || INADDR_ANY == addr_gateway.s_addr ) {
        detail("getdefaultgateway INADDR_NONE or INADDR_ANY %d", addr_gateway.s_addr);
        return ELocalIPStack_IPv6;
    }
#endif
    sockaddr_storage v4_addr = {0};
    sockaddr_storage v6_addr = {0};
    int have_ipv4 = _have_ipv4((sockaddr*)&v4_addr, sizeof(v4_addr));
    int have_ipv6 = _have_ipv6((sockaddr*)&v6_addr, sizeof(v6_addr));
    int local_stack = 0;
    if (have_ipv4) { local_stack |= ELocalIPStack_IPv4; }
    if (have_ipv6) { local_stack |= ELocalIPStack_IPv6; }
    
    detail("have_ipv4:%d have_ipv6:%d \n", have_ipv4, have_ipv6);

#ifdef __APPLE__
    if (ELocalIPStack_Dual != local_stack) {
    	detail("ELocalIPStack_Dual != local_stack");
        return (TLocalIPStack)local_stack;
    }
//    if (two_addrs_on_one_interface((sockaddr*)&v4_addr, (sockaddr*)&v6_addr)) {
//    	detail("two_addrs_on_one_interface: true.v4_addr=%s, v6_addr=%_", socket_address(v4_addr).ip(), ocket_address(v6_addr).ipv6());
//        return ELocalIPStack_Dual;
//    }
    if (publiccomponent_GetSystemVersion() < 9.0f) {
		int dns_ip_stack = 0;
		std::vector<socket_address> dnssvraddrs;
		getdnssvraddrs(dnssvraddrs);

		for (size_t i = 0; i < dnssvraddrs.size(); ++i) {
			if (AF_INET == dnssvraddrs[i].address().sa_family) { dns_ip_stack |= ELocalIPStack_IPv4; }
			if (AF_INET6 == dnssvraddrs[i].address().sa_family) { dns_ip_stack |= ELocalIPStack_IPv6; }
		}

		detail("local_stack:%d dns_ip_stack:%d", local_stack, dns_ip_stack);
		return (TLocalIPStack)(ELocalIPStack_None==dns_ip_stack? local_stack:dns_ip_stack);
    } else {
    	xassert2(ELocalIPStack_Dual == local_stack);
    	return (TLocalIPStack)local_stack;
    }
#else
    return (TLocalIPStack)local_stack;
#endif
}

TLocalIPStack local_ipstack_detect() {
#ifdef ANDROID
	return ELocalIPStack_IPv4;
#endif
    std::string log;
    return __local_ipstack_detect(log);
}

static void __local_info(std::string& _log);

TLocalIPStack local_ipstack_detect_log(std::string& _log) {
    __local_info(_log);
   return __local_ipstack_detect(_log);
}

#include "network/getifaddrs.h"
#include "network/getgateway.h"
#include "network/getdnssvraddrs.h"

static void __local_info(std::string& _log) {
    XMessage detail_net_info;
    in6_addr addr6_gateway;
    memset(&addr6_gateway, 0, sizeof(addr6_gateway));
    if (0 == getdefaultgateway6(&addr6_gateway)) {
        detail_net_info << "defaultgateway6:" << socket_address(addr6_gateway).ipv6() << "\n";
    } else {
        detail_net_info << "defaultgateway6:failed \n";
    }
    in_addr addr_gateway;
    memset(&addr_gateway, 0, sizeof(addr_gateway));
    if (0 == getdefaultgateway(&addr_gateway)) {
        detail_net_info << "defaultgateway:" << socket_address(addr_gateway).ip() << "\n";
    } else {
        detail_net_info << "defaultgateway: failed \n";
    }
    
    std::vector<socket_address> dnssvraddrs;
    getdnssvraddrs(dnssvraddrs);
    if (!dnssvraddrs.empty()) {
        for (size_t i = 0; i < dnssvraddrs.size(); ++i) {
            if (AF_INET == dnssvraddrs[i].address().sa_family) {
                detail_net_info << "dns server" << i << ":AF_INET, " << dnssvraddrs[i].ip() << "\n";
            }
            if (AF_INET6 == dnssvraddrs[i].address().sa_family) {
                detail_net_info << "dns server" << i << ":AF_INET6, " << dnssvraddrs[i].ipv6() << "\n";
            }
        }
    } else {
        detail_net_info << "dns server: empty \n";
    }
    
    std::vector<ifaddrinfo_ip_t> v4_addrs;
    if (getifaddrs_ipv4_filter(v4_addrs, 0)) {
        for (size_t i = 0; i < v4_addrs.size(); ++i) {
            detail_net_info << "interface name:"<<v4_addrs[i].ifa_name << ", " << (v4_addrs[i].ifa_family==AF_INET?"AF_INET":"XX_INET")
            << ", ip:" << v4_addrs[i].ip << "\n";
        }
    } else {
        detail_net_info << "getifaddrs_ipv4_filter:false \n";
    }
    std::vector<ifaddrinfo_ip_t> v6_addrs;
    if (getifaddrs_ipv6_filter(v6_addrs, 0)) {
        for (size_t i = 0; i < v6_addrs.size(); ++i) {
            detail_net_info << "interface name:"<<v6_addrs[i].ifa_name << ", " << (v6_addrs[i].ifa_family==AF_INET6?"AF_INET6":"XX_INET")
	    		    	<< ", ip:" << v6_addrs[i].ip << "\n";
        }
    } else {
        detail_net_info << "getifaddrs_ipv6_filter:false \n";
    }
    
    sockaddr_storage v4_addr = {0};
    sockaddr_storage v6_addr = {0};
    int have_ipv4 = _have_ipv4((sockaddr*)&v4_addr, sizeof(v4_addr));
    int have_ipv6 = _have_ipv6((sockaddr*)&v6_addr, sizeof(v6_addr));
    detail_net_info("have_ipv4:%d have_ipv6:%d", have_ipv4, have_ipv6);
    
    _log += detail_net_info.Message();
}

#else
#include <string>
TLocalIPStack local_ipstack_detect() {
    return ELocalIPStack_IPv4;
}
TLocalIPStack local_ipstack_detect_log(std::string& _log) {
	_log = "no implement";
   return local_ipstack_detect();
}

#endif //__APPLE__

//TIPNetworkType IPNetworkTypeDetect_Gateway() {
//    in6_addr addr6_gateway = {0};
//    if (0 != getdefaultgateway6(&addr6_gateway))
//        return EIPv4;
//    
//    if (IN6_IS_ADDR_UNSPECIFIED(&addr6_gateway))
//        return EIPv4;
//    
//    in_addr addr_gateway = {0};
//    if (0 != getdefaultgateway(&addr_gateway))
//        return EIPv6;
//    
//    if (INADDR_NONE == addr_gateway.s_addr || INADDR_ANY == addr_gateway.s_addr )
//        return EIPv6;
//    
//    return EIPv46;
//}
