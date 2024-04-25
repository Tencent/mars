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
#include "comm/strutil.h"
#include "comm/xlogger/xlogger.h"

#if defined(__linux__) && !defined(AI_DEFAULT)
	#define  AI_DEFAULT (AI_V4MAPPED | AI_ADDRCONFIG)
#endif

static const char kWellKnownNat64Prefix[] = {'6', '4', ':','f', 'f', '9', 'b', ':', ':', '\0'};

socket_address::socket_address(const char* _ip, uint16_t _port, bool _bind_cellular) {
    in6_addr addr6 = IN6ADDR_ANY_INIT;
    in_addr  addr4 = {0};
    
    if (socket_inet_pton(AF_INET, _ip, &addr4)) {
        sockaddr_in sock_addr = {0};
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr = addr4;
        sock_addr.sin_port = htons(_port);

        __init((sockaddr*)&sock_addr, _bind_cellular);
    } else if (socket_inet_pton(AF_INET6, _ip, &addr6)) {
        sockaddr_in6 sock_addr = {0};
        sock_addr.sin6_family = AF_INET6;
        sock_addr.sin6_addr = addr6;
        sock_addr.sin6_port = htons(_port);
        
        __init((sockaddr*)&sock_addr, _bind_cellular);
    } else {
    	sockaddr sock_addr = {0};
    	sock_addr.sa_family = AF_UNSPEC;
    	__init((sockaddr*)&sock_addr, _bind_cellular);
    }
}

socket_address::socket_address(const sockaddr_in& _addr) {
    __init((sockaddr*)&_addr);
}

socket_address::socket_address(const sockaddr_in6& _addr) {
    __init((sockaddr*)&_addr);
}

socket_address::socket_address(const sockaddr* _addr, bool _bind_cellular) {
    __init(_addr, _bind_cellular);
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

void  socket_address::__init(const sockaddr* _addr, bool _bind_cellular) {
    memset(&addr_, 0, sizeof(addr_));
    memset(ip_, 0, sizeof(ip_));
    memset(url_, 0, sizeof(url_));
    is_bind_cellular_network_ = _bind_cellular;

    if (AF_INET == _addr->sa_family) {
        memcpy(&addr_, _addr, sizeof(sockaddr_in));
        socket_inet_ntop(_asv4()->sin_family, &_asv4()->sin_addr, ip_, sizeof(ip_));
        snprintf(url_, sizeof(url_), "%s:%u", ip_, port());
    } else if (AF_INET6 == _addr->sa_family) {
        memcpy(&addr_, _addr, sizeof(sockaddr_in6));
        if (IN6_IS_ADDR_NAT64(&_asv6()->sin6_addr)) {
			strncpy(ip_, kWellKnownNat64Prefix, 9);
			sockaddr_in addr = { 0 };
			addr.sin_family = AF_INET;
#ifdef WIN32
            addr.sin_addr.s_addr = *((in_addr_t*)&(_asv6()->sin6_addr.u.Byte[12]));
#else
            addr.sin_addr.s_addr = _asv6()->sin6_addr.s6_addr32[3];
#endif
			socket_inet_ntop(_asv6()->sin6_family, &(_asv6()->sin6_addr), ip_+9, sizeof(ip_)-9);
		} else {
			socket_inet_ntop(_asv6()->sin6_family, &(_asv6()->sin6_addr), ip_, sizeof(ip_));
		}

		snprintf(url_, sizeof(url_), "[%s]:%u", ip_, port());
	} else {
        addr_.ss_family = AF_UNSPEC;
    }
}

bool socket_address::fix_current_nat64_addr() {
	xinfo_function(); //打印耗时
	bool ret = false;
	bool is_update = false;
    if (AF_INET6 == addr_.ss_family && 0!=strncasecmp("::FFFF:", ip_, 7)) {
		//更新addr_, ip_, url_
//		if (is_update) {
			in6_addr nat64_v6_addr;
#ifdef WIN32
            ret = ConvertV4toNat64V6(*(struct in_addr*)(&(_asv6()->sin6_addr.u.Byte[12])), nat64_v6_addr);
#else
            ret = ConvertV4toNat64V6(*(struct in_addr*)(&(_asv6()->sin6_addr.s6_addr32[3])), nat64_v6_addr);
#endif
			
			xdebug2(TSF"ret =%_, ip_=%_, nat64_v6_addr = %_", ret, ip_, strutil::Hex2Str((char*)&(nat64_v6_addr.s6_addr16), 16));
			if (ret) {
				memcpy ((char*)&(_asv6()->sin6_addr.s6_addr16), (char*)&(nat64_v6_addr.s6_addr16), 16);
				socket_inet_ntop(AF_INET6, &(_asv6()->sin6_addr), ip_, sizeof(ip_));
				//-----把ip_转为更易读的v6 ip形式---//
				if (0==strncasecmp(kWellKnownNat64Prefix, ip_, 9)) {
					sockaddr_in addr_v4 = { 0 };
					addr_v4.sin_family = AF_INET;
#ifdef WIN32
                    addr_v4.sin_addr.s_addr = *((in_addr_t*)&(_asv6()->sin6_addr.u.Byte[12]));
#else
                    addr_v4.sin_addr.s_addr = _asv6()->sin6_addr.s6_addr32[3];
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
	if (AF_INET6 == addr_.ss_family) {
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
    if (AF_INET == addr_.ss_family) {
        return sizeof(sockaddr_in);
    } else if (AF_INET6 == addr_.ss_family) {
        return sizeof(sockaddr_in6);
    }

    return 0;
}

const char* socket_address::url() const {
    return url_;
}

const char* socket_address::ip() const {
    if (AF_INET == addr_.ss_family) {
        return ip_;
    } else if (AF_INET6 == addr_.ss_family) {
        if (0 == strncasecmp("::FFFF:", ip_, 7))
            return ip_+7;
        else if (0 == strncasecmp(kWellKnownNat64Prefix, ip_, 9))
        	return ip_+9;
        else
            return ip_;
    }

    xerror2(TSF"invalid ip family:%_, ip:%_", addr_.ss_family, ip_);
    return "";
}

const char* socket_address::ipv6() const {
    return ip_;
}

uint16_t socket_address::port() const {
    if (AF_INET == addr_.ss_family) {
        return ntohs(_asv4()->sin_port);
    } else if (AF_INET6 == addr_.ss_family) {
        return ntohs(_asv6()->sin6_port);
    }

    return 0;
}

bool socket_address::valid() const {
    return (AF_INET == addr_.ss_family || AF_INET6 == addr_.ss_family);
}

bool socket_address::valid_server_address(bool _allowloopback, bool _ignore_port) const {
    if (AF_INET == addr_.ss_family) {
        uint32_t hostip = ntohl(_asv4()->sin_addr.s_addr);
        return  (_ignore_port ? true : 0 != _asv4()->sin_port)
        && hostip != INADDR_ANY
        && hostip != INADDR_BROADCAST
        && hostip != INADDR_NONE
        && (_allowloopback? true : hostip != INADDR_LOOPBACK);
    } else if (AF_INET6 == addr_.ss_family) {
        if (IN6_IS_ADDR_V4MAPPED(&_asv6()->sin6_addr)) {
            uint32_t hostip = ntohl((*(const uint32_t *)(const void *)(&_asv6()->sin6_addr.s6_addr[12])));
            return  (_ignore_port ? true :0 != _asv6()->sin6_port)
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

bool socket_address::valid_broadcast_address() const {
    if (AF_INET == addr_.ss_family) {
        return  0 != _asv4()->sin_port && INADDR_BROADCAST == ntohl(_asv4()->sin_addr.s_addr);
    } else if (AF_INET6 == addr_.ss_family) {
        return false;
    }
    return false;

}

bool socket_address::valid_loopback_ip() const {
    if (AF_INET == addr_.ss_family) {
        return INADDR_LOOPBACK == ntohl(_asv4()->sin_addr.s_addr);
    } else if (AF_INET6 == addr_.ss_family) {
        return false;
    }
    return false;
}

bool socket_address::valid_broadcast_ip() const {
    if (AF_INET == addr_.ss_family) {
        return INADDR_BROADCAST == ntohl(_asv4()->sin_addr.s_addr);
    } else if (AF_INET6 == addr_.ss_family) {
        return false;
    }
    return false;
}

bool socket_address::isv4mapped_address() const {
    if (AF_INET6 == addr_.ss_family) {
        return IN6_IS_ADDR_V4MAPPED(&(_asv6()->sin6_addr));
    }
    return false;
}

bool socket_address::isv6() const{
    return AF_INET6 == addr_.ss_family && !isv4mapped_address();
}
bool socket_address::isv4() const{
    return AF_INET == addr_.ss_family || isv4mapped_address();
}

socket_address& socket_address::v4tov4mapped_address() {
    if (AF_INET == addr_.ss_family) {
        sockaddr_in6 sock_addr6 = {0};
        sockaddr_in sock_addr4 = *_asv4();

        sock_addr6.sin6_family = AF_INET6;
        IN6_SET_ADDR_V4MAPPED(&sock_addr6.sin6_addr, &sock_addr4.sin_addr);
        sock_addr6.sin6_port = sock_addr4.sin_port;

        __init((sockaddr*)&sock_addr6);
    }
    return *this;
}

socket_address& socket_address::v4tonat64_address() {
    if (AF_INET == addr_.ss_family) {
        sockaddr_in6 sock_addr6 = {0};
        sockaddr_in sock_addr4 = *_asv4();

        sock_addr6.sin6_family = AF_INET6;
        IN6_SET_ADDR_NAT64(&sock_addr6.sin6_addr, &sock_addr4.sin_addr);
        sock_addr6.sin6_port = sock_addr4.sin_port;

        __init((sockaddr*)&sock_addr6);
    }
    address_fix();
    return *this;
}

socket_address& socket_address::v4tov6_address(TLocalIPStack stack) {
	if (stack == ELocalIPStack_IPv6)
		return v4tonat64_address();
	else if (stack == ELocalIPStack_IPv4) {
	    return *this;
	} else {
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
        } else {
            xerror2(TSF"invalid famiray %_", addr.ss_family);
        }
    }

    return socket_address("0.0.0.0", 0);
}

const sockaddr_in* socket_address::_asv4() const{
    return reinterpret_cast<const sockaddr_in*>(&addr_);
}
const sockaddr_in6* socket_address::_asv6() const{
    return reinterpret_cast<const sockaddr_in6*>(&addr_);
}

bool socket_address::is_bind_cellular_network() const{
    return is_bind_cellular_network_;
}


//
bool operator==(const socket_address& lhs, const socket_address& rhs){
    return lhs.is_ipport_equal(rhs);
}
