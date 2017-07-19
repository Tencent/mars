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
 * socket_address.cpp
 *
 *  Created on: 2014-12-1
 *      Author: yerungui
 */

#include "comm/socket/socket_address.h"

#include <stdio.h>
#include <string.h>

#include "comm/socket/ipv6_address_utils.h"
#include "comm/socket/nat64_prefix_util.h"
#include "comm/socket/local_ipstack.h"
#include "comm/strutil.h"
#include "comm/xlogger/xlogger.h"

#if defined(__linux__) && !defined(AI_DEFAULT)
	#define  AI_DEFAULT (AI_V4MAPPED | AI_ADDRCONFIG)
#endif

static const char kWellKnownNat64Prefix[] = {'6', '4', ':','f', 'f', '9', 'b', ':', ':', '\0'};

socket_address::socket_address(const char* _url) {
    char ip_s[40] = {0};
    uint16_t port_u = 0;

    if (0 < sscanf(_url, "%15[0-9.]:%8hu", ip_s, &port_u)) {
        sockaddr_in sock_addr = {0};
        sock_addr.sin_family = AF_INET;
        socket_inet_pton(AF_INET, ip_s, &sock_addr.sin_addr);
        sock_addr.sin_port = htons(port_u);

        __init((sockaddr*)&sock_addr);
    } else if (0 < sscanf(_url, "[%40[0-9a-fA-F:.]]:%8hu", ip_s, &port_u) || 0 < sscanf(_url, "%40[0-9a-fA-F:.]", ip_s)) {
        sockaddr_in6 sock_addr = {0};
        sock_addr.sin6_family = AF_INET6;
        socket_inet_pton(AF_INET6, ip_s, &sock_addr.sin6_addr);
        sock_addr.sin6_port = htons(port_u);

        __init((sockaddr*)&sock_addr);
    } else {
    	sockaddr sock_addr = {0};
    	sock_addr.sa_family = AF_UNSPEC;
    	__init((sockaddr*)&sock_addr);
    }
}

socket_address::socket_address(const char* _ip, uint16_t _port) {
    in6_addr addr6 = IN6ADDR_ANY_INIT;
    in_addr  addr4 = {0};
    
    if (socket_inet_pton(AF_INET, _ip, &addr4)) {
        sockaddr_in sock_addr = {0};
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr = addr4;
        sock_addr.sin_port = htons(_port);

        __init((sockaddr*)&sock_addr);
    } else if (socket_inet_pton(AF_INET6, _ip, &addr6)) {
        sockaddr_in6 sock_addr = {0};
        sock_addr.sin6_family = AF_INET6;
        sock_addr.sin6_addr = addr6;
        sock_addr.sin6_port = htons(_port);
        
        __init((sockaddr*)&sock_addr);
    } else {
    	sockaddr sock_addr = {0};
    	sock_addr.sa_family = AF_UNSPEC;
    	__init((sockaddr*)&sock_addr);
    }
}

socket_address::socket_address(const sockaddr_in& _addr) {
    __init((sockaddr*)&_addr);
}

socket_address::socket_address(const sockaddr_in6& _addr) {
    __init((sockaddr*)&_addr);
}

socket_address::socket_address(const sockaddr* _addr) {
    __init(_addr);
}

socket_address::socket_address(const struct in_addr& _in_addr) {
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_addr = _in_addr;
	__init((sockaddr*)&addr);
}

socket_address::socket_address(const struct in6_addr& _in6_addr) {
	sockaddr_in6 addr6 = {0};
	addr6.sin6_family = AF_INET6;
	addr6.sin6_addr = _in6_addr;
	__init((sockaddr*)&addr6);
}

void  socket_address::__init(const sockaddr* _addr) {
    memset(&addr_, 0, sizeof(addr_));
    memset(ip_, 0, sizeof(ip_));
    memset(url_, 0, sizeof(url_));

    if (AF_INET == _addr->sa_family) {
        (sockaddr_in&)addr_ = *(sockaddr_in*)_addr;
        sockaddr_in& addr = (sockaddr_in&)addr_;

        socket_inet_ntop(addr.sin_family, &(addr.sin_addr), ip_, sizeof(ip_));
        snprintf(url_, sizeof(url_), "%s:%u", ip_, port());
    } else if (AF_INET6 == _addr->sa_family) {
        (sockaddr_in6&)addr_ = *(sockaddr_in6*)_addr;
        sockaddr_in6& addr = (sockaddr_in6&)addr_;

		if (IN6_IS_ADDR_NAT64(&addr_.in6.sin6_addr)) {
			strncpy(ip_, kWellKnownNat64Prefix, 9);
			sockaddr_in addr = { 0 };
			addr.sin_family = AF_INET;
#ifdef WIN32
            addr.sin_addr.s_addr = *((in_addr_t*)&(addr_.in6.sin6_addr.u.Byte[12]));
#else
			addr.sin_addr.s_addr = addr_.in6.sin6_addr.s6_addr32[3];
#endif
			socket_inet_ntop(addr.sin_family, &(addr.sin_addr), ip_+9, sizeof(ip_)-9);
		} else {
			socket_inet_ntop(addr.sin6_family, &(addr.sin6_addr), ip_, sizeof(ip_));
		}

		snprintf(url_, sizeof(url_), "[%s]:%u", ip_, port());
	} else {
    	addr_.sa.sa_family = AF_UNSPEC;
    }
}

bool socket_address::fix_current_nat64_addr() {
	xinfo_function(); //打印耗时
	bool ret = false;
	bool is_update = false;
	if (AF_INET6 == addr_.sa.sa_family && 0!=strncasecmp("::FFFF:", ip_, 7)) {
		//更新addr_, ip_, url_
//		if (is_update) {
			in6_addr nat64_v6_addr;
#ifdef WIN32
            ret = ConvertV4toNat64V6(*(struct in_addr*)(&(addr_.in6.sin6_addr.u.Byte[12])), nat64_v6_addr);
#else
            ret = ConvertV4toNat64V6(*(struct in_addr*)(&(addr_.in6.sin6_addr.s6_addr32[3])), nat64_v6_addr);
#endif
			
			xdebug2(TSF"ret =%_, ip_=%_, nat64_v6_addr = %_", ret, ip_, strutil::Hex2Str((char*)&(nat64_v6_addr.s6_addr16), 16));
			if (ret) {
				memcpy ((char*)&(addr_.in6.sin6_addr.s6_addr16), (char*)&(nat64_v6_addr.s6_addr16), 16);
				socket_inet_ntop(AF_INET6, &(addr_.in6.sin6_addr), ip_, sizeof(ip_));
				//-----把ip_转为更易读的v6 ip形式---//
				if (0==strncasecmp(kWellKnownNat64Prefix, ip_, 9)) {
					sockaddr_in addr_v4 = { 0 };
					addr_v4.sin_family = AF_INET;
#ifdef WIN32
                    addr_v4.sin_addr.s_addr = *((in_addr_t*)&(addr_.in6.sin6_addr.u.Byte[12]));
#else
                    addr_v4.sin_addr.s_addr = addr_.in6.sin6_addr.s6_addr32[3];
#endif
					socket_inet_ntop(addr_v4.sin_family, &(addr_v4.sin_addr), ip_+9, sizeof(ip_)-9);
				}
				//-----------------------------//
				snprintf(url_, sizeof(url_), "[%s]:%u", ip_, port());
				xdebug2(TSF"after fix url_=%_", url_);
			} else {
				xerror2(TSF"ConvertV4toNat64V6() ret=%_, ipstack=%_", ret, TLocalIPStackStr[local_ipstack_detect()]);
			}
//		} else {
//			xdebug2(TSF"no update nat64_prefix");
//		}

	}
	xdebug2(TSF"is_update =%_, ret=%_", is_update, ret);
	return ret;
}
const sockaddr& socket_address::address_fix() {
	if (AF_INET6 == addr_.sa.sa_family) {
		xdebug2(TSF"before fix current ipv6 = %_", ipv6());
		fix_current_nat64_addr();
		xdebug2(TSF"after fix current ipv6 = %_", ipv6());
	}
    return (sockaddr&)addr_;
}
const sockaddr& socket_address::address() const {
	//if (ELocalIPStack_IPv6==local_ipstack_detect())
	//	return address_fix();
    return (sockaddr&)addr_;
}
socklen_t socket_address::address_length() const {
    if (AF_INET == addr_.sa.sa_family) {
        return sizeof(sockaddr_in);
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return sizeof(sockaddr_in6);
    }

    return 0;
}

const char* socket_address::url() const {
    return url_;
}

const char* socket_address::ip() const {
    if (AF_INET == addr_.sa.sa_family) {
        return ip_;
    } else if (AF_INET6 == addr_.sa.sa_family) {
        if (0 == strncasecmp("::FFFF:", ip_, 7))
            return ip_+7;
        else if (0 == strncasecmp(kWellKnownNat64Prefix, ip_, 9))
        	return ip_+9;
        else
            return ip_;
    }

    return "";
}

const char* socket_address::ipv6() const {
    return ip_;
}

uint16_t socket_address::port() const {
    if (AF_INET == addr_.sa.sa_family) {
        return ntohs(addr_.in.sin_port);
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return ntohs(addr_.in6.sin6_port);
    }

    return 0;
}

bool socket_address::valid() const {
    if (AF_INET == addr_.sa.sa_family) {
    	return true;
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return true;
    }

    return false;
}

bool socket_address::valid_server_address(bool _allowloopback) const {
    if (AF_INET == addr_.sa.sa_family) {
        const sockaddr_in& sock_addr = addr_.in;

        uint32_t hostip = ntohl(sock_addr.sin_addr.s_addr);
        return  0 != sock_addr.sin_port
        && hostip != INADDR_ANY
        && hostip != INADDR_BROADCAST
        && hostip != INADDR_NONE
        && (_allowloopback? true : hostip != INADDR_LOOPBACK);
    } else if (AF_INET6 == addr_.sa.sa_family) {
        const sockaddr_in6& sock_addr6 = addr_.in6;
        if (IN6_IS_ADDR_V4MAPPED(&(sock_addr6.sin6_addr))) {
            uint32_t hostip = ntohl((*(const uint32_t *)(const void *)(&sock_addr6.sin6_addr.s6_addr[12])));
            return  0 != sock_addr6.sin6_port
            && hostip != INADDR_ANY
            && hostip != INADDR_BROADCAST
            && hostip != INADDR_NONE
            && (_allowloopback? true : hostip != INADDR_LOOPBACK);
        } else {
            //TODO
            return true;
        }
    }
    
    return false;
}

bool socket_address::valid_bind_address() const {
    if (AF_INET == addr_.sa.sa_family) {
        const sockaddr_in& sock_addr = addr_.in;

        uint32_t hostip = ntohl(sock_addr.sin_addr.s_addr);
        return  hostip != INADDR_BROADCAST;
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return false;
    }
     return false;
}

bool socket_address::valid_broadcast_address() const {
    if (AF_INET == addr_.sa.sa_family) {
        const sockaddr_in& sock_addr = addr_.in;
        return  0 != sock_addr.sin_port && INADDR_BROADCAST == ntohl(sock_addr.sin_addr.s_addr);
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return false;
    }
    return false;

}

bool socket_address::valid_loopback_ip() const {
    if (AF_INET == addr_.sa.sa_family) {
        const sockaddr_in& sock_addr = addr_.in;
        return INADDR_LOOPBACK == ntohl(sock_addr.sin_addr.s_addr);
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return false;
    }
    return false;
}

bool socket_address::valid_broadcast_ip() const {
    if (AF_INET == addr_.sa.sa_family) {
        const sockaddr_in& sock_addr = addr_.in;
        return INADDR_BROADCAST == ntohl(sock_addr.sin_addr.s_addr);
    } else if (AF_INET6 == addr_.sa.sa_family) {
        return false;
    }
    return false;
}

bool socket_address::isv4mapped_address() const {
    if (AF_INET6 == addr_.sa.sa_family) {
        const sockaddr_in6& sock_addr = addr_.in6;
        return IN6_IS_ADDR_V4MAPPED(&sock_addr.sin6_addr);
    }
    return false;
}

socket_address& socket_address::v4tov4mapped_address() {
    if (AF_INET == addr_.sa.sa_family) {
        sockaddr_in6 sock_addr6 = {0};
        sockaddr_in sock_addr4 = addr_.in;

        sock_addr6.sin6_family = AF_INET6;
        IN6_SET_ADDR_V4MAPPED(&sock_addr6.sin6_addr, &sock_addr4.sin_addr);
        sock_addr6.sin6_port = sock_addr4.sin_port;

        __init((sockaddr*)&sock_addr6);
    }
    return *this;
}

socket_address& socket_address::v4tonat64_address() {
    if (AF_INET == addr_.sa.sa_family) {
        sockaddr_in6 sock_addr6 = {0};
        sockaddr_in sock_addr4 = addr_.in;

        sock_addr6.sin6_family = AF_INET6;
        IN6_SET_ADDR_NAT64(&sock_addr6.sin6_addr, &sock_addr4.sin_addr);
        sock_addr6.sin6_port = sock_addr4.sin_port;

        __init((sockaddr*)&sock_addr6);
    }
    address_fix();
    return *this;
}

socket_address& socket_address::v4tov6_address(bool _nat64) {
	if (_nat64)
		return v4tonat64_address();
	else
	{
#ifdef WIN32
		return *this;
#else
		return v4tov4mapped_address();
#endif
	}

}

socket_address socket_address::getsockname(SOCKET _sock) {
    struct sockaddr_storage addr = {0};
    socklen_t addr_len = sizeof(addr);
    
    if (0 == ::getsockname(_sock, (sockaddr*)&addr, &addr_len)) {
        if (AF_INET == addr.ss_family) {
            return socket_address((const sockaddr_in&)addr);
        } else if (AF_INET6 == addr.ss_family) {
            return socket_address((const sockaddr_in6&)addr);
        }
    }

    return socket_address("0.0.0.0", 0);
}

socket_address socket_address::getpeername(SOCKET _sock) {
    struct sockaddr_storage addr = {0};
    socklen_t addr_len = sizeof(addr);
    
    if (0 == ::getpeername(_sock, (sockaddr*)&addr, &addr_len)) {
        if (AF_INET == addr.ss_family) {
            return socket_address((const sockaddr_in&)addr);
        } else if (AF_INET6 == addr.ss_family) {
            return socket_address((const sockaddr_in6&)addr);
        }
    }

    return socket_address("0.0.0.0", 0);
}
