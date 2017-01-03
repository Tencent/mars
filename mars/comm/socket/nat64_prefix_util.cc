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
 * nat64_prefix_util.cpp
 *
 *  Created on: 2016年6月22日
 *      Author: wutianqiang
 */


#include "nat64_prefix_util.h"
#include "local_ipstack.h"
#include "unix_socket.h"
#include "xlogger/xlogger.h"
#include "strutil.h"
#include "platform_comm.h"

static const uint8_t kWellKnownV4Addr1[4] = {192, 0, 0, 170};
static const uint8_t kWellKnownV4Addr2[4] = {192, 0, 0, 171};
static const uint8_t kOurDefineV4Addr[4] = {192, 0, 2, 1};

//insert 0 after first byte
static const uint8_t kWellKnownV4Addr1_index1[5] = {192, 0, 0, 0, 170};
static const uint8_t kWellKnownV4Addr2_index1[5] = {192, 0, 0, 0, 171};
static const uint8_t kOurDefineV4Addr_index1[5] = {192, 0, 0, 2, 1};

//insert 0 after second byte
static const uint8_t kWellKnownV4Addr1_index2[5] = {192, 0, 0, 0, 170};
static const uint8_t kWellKnownV4Addr2_index2[5] = {192, 0, 0, 0, 171};
static const uint8_t kOurDefineV4Addr_index2[5] = {192, 0, 0, 2, 1};

//insert 0 after third byte
static const uint8_t kWellKnownV4Addr1_index3[5] = {192, 0, 0, 0, 170};
static const uint8_t kWellKnownV4Addr2_index3[5] = {192, 0, 0, 0, 171};
static const uint8_t kOurDefineV4Addr_index3[5] = {192, 0, 2, 0, 1};

//static bool IsIPv4Addr(const std::string& _str) {
//	struct in_addr v4_addr= {0};
//	return socket_inet_pton(AF_INET, _str.c_str(), &v4_addr)==0; //1 for success, 0 for invalid ip, -1 for other error
//}
static size_t GetSuffixZeroCount(uint8_t* _buf, size_t _buf_len) {
	size_t zero_count = 0;
	for(size_t i=0; i<_buf_len; i++) {
		if ((uint8_t)0==_buf[_buf_len-1-i])
			zero_count++;
		else
			break;

	}
	return zero_count;
}
static bool IsNat64AddrValid(const struct in6_addr* _replaced_nat64_addr) {
	bool is_iOS_above_9_2 = false;
#ifdef __APPLE__
	 if (publiccomponent_GetSystemVersion() >= 9.2f) is_iOS_above_9_2 = true;
#endif
	size_t suffix_zero_count = GetSuffixZeroCount((uint8_t*)_replaced_nat64_addr, sizeof(struct in6_addr));
	if (0!=suffix_zero_count) {
		xwarn2(TSF"suffix_zero_count=%_, _replaced_nat64_addr=%_", suffix_zero_count,
				strutil::Hex2Str((char*)_replaced_nat64_addr, sizeof(struct in6_addr)));
	}
	bool is_valid = false;
	switch(suffix_zero_count) {
		case 3:
			//Pref64::/64
			if (is_iOS_above_9_2) {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+9, kOurDefineV4Addr, 4)) {
					is_valid = true;
				}
			} else {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+9, kWellKnownV4Addr1, 4)
					|| 0==memcmp(((uint8_t*)_replaced_nat64_addr)+9, kWellKnownV4Addr2, 4)) {
					is_valid = true;
				}
			}
			break;
		case 4:
			//Pref64::/56
			if (is_iOS_above_9_2) {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+7, kOurDefineV4Addr_index1, 5)) {
					is_valid = true;
				}
			} else {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+7, kWellKnownV4Addr1_index1, 5)
					|| 0==memcmp(((uint8_t*)_replaced_nat64_addr)+7, kWellKnownV4Addr2_index1, 5)) {
					is_valid = true;
				}
			}
			break;
		case 5:
			//Pref64::/48
			if (is_iOS_above_9_2) {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+6, kOurDefineV4Addr_index2, 5)) {
					is_valid = true;
				}
			} else {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+6, kWellKnownV4Addr1_index2, 5)
					|| 0==memcmp(((uint8_t*)_replaced_nat64_addr)+6, kWellKnownV4Addr2_index2, 5)) {
					is_valid = true;
				}
			}
			break;
		case 6:
			//Pref64::/40
			if (is_iOS_above_9_2) {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+5, kOurDefineV4Addr_index3, 5)) {
					is_valid = true;
				}
			} else {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+5, kWellKnownV4Addr1_index3, 5)
					|| 0==memcmp(((uint8_t*)_replaced_nat64_addr)+5, kWellKnownV4Addr2_index3, 5)) {
					is_valid = true;
				}
			}
			break;
		case 8: //7bytes suffix and 1 bytes u(RFC6052)
			//Pref64::/32
			if (is_iOS_above_9_2) {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+4, kOurDefineV4Addr, 4)) {
					is_valid = true;
				}
			} else {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+4, kWellKnownV4Addr1, 4)
					|| 0==memcmp(((uint8_t*)_replaced_nat64_addr)+4, kWellKnownV4Addr2, 4)) {
					is_valid = true;
				}
			}
			break;
		case 0:
			//Pref64::/96
			if (is_iOS_above_9_2) {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+12, kOurDefineV4Addr, 4)) {
					is_valid = true;
				}
			} else {
				if (0==memcmp(((uint8_t*)_replaced_nat64_addr)+12, kWellKnownV4Addr1, 4)
					|| 0==memcmp(((uint8_t*)_replaced_nat64_addr)+12, kWellKnownV4Addr2, 4)) {
					is_valid = true;
				}
			}
			break;
		default:
			xassert2(false, TSF"suffix_zero_count=%_", suffix_zero_count);
	}
	return is_valid;
}
static void ReplaceNat64WithV4IP(struct in6_addr* _replaced_nat64_addr, const struct in_addr* _v4_addr) {
	size_t suffix_zero_count = GetSuffixZeroCount((uint8_t*)_replaced_nat64_addr, sizeof(struct in6_addr));
	uint8_t zero = (uint8_t)0;
	switch(suffix_zero_count) {
		case 3:
			//Pref64::/64
			memcpy(((uint8_t*)_replaced_nat64_addr)+9, (uint8_t*)_v4_addr, 4);
			break;
		case 4:
			//Pref64::/56
			memcpy(((uint8_t*)_replaced_nat64_addr)+7, (uint8_t*)_v4_addr, 1);
			memcpy(((uint8_t*)_replaced_nat64_addr)+8, &zero, 1);
			memcpy(((uint8_t*)_replaced_nat64_addr)+9, ((uint8_t*)_v4_addr)+1, 3);

			break;
		case 5:
			//Pref64::/48
			memcpy(((uint8_t*)_replaced_nat64_addr)+6, (uint8_t*)_v4_addr, 2);
			memcpy(((uint8_t*)_replaced_nat64_addr)+8, &zero, 1);
			memcpy(((uint8_t*)_replaced_nat64_addr)+9, ((uint8_t*)_v4_addr)+2, 2);
			break;
		case 6:
			//Pref64::/40
			memcpy(((uint8_t*)_replaced_nat64_addr)+5, (uint8_t*)_v4_addr, 3);
			memcpy(((uint8_t*)_replaced_nat64_addr)+8, &zero, 1);
			memcpy(((uint8_t*)_replaced_nat64_addr)+9, ((uint8_t*)_v4_addr)+3, 1);
			break;
		case 8:
			//Pref64::/32
			memcpy(((uint8_t*)_replaced_nat64_addr)+4, (uint8_t*)_v4_addr, 4);
			break;
		case 0:
			//Pref64::/96
			memcpy(((uint8_t*)_replaced_nat64_addr)+12, (uint8_t*)_v4_addr, 4);
			break;
		default:
			memcpy(((uint8_t*)_replaced_nat64_addr)+12, (uint8_t*)_v4_addr, 4);
			xassert2(false, TSF"suffix_zero_count=%_", suffix_zero_count);
	}
}

bool ConvertV4toNat64V6(const struct in_addr& _v4_addr, struct in6_addr& _v6_addr) {
    xdebug_function();
    if (ELocalIPStack_IPv6 != local_ipstack_detect()) {
    	xwarn2(TSF"Current Network is not ELocalIPStack_IPv6, no need GetNetworkNat64Prefix.");
		return false;
    }
	struct addrinfo hints, *res=NULL, *res0=NULL;
	int error = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	char v4_ip[16] = {0};
	socket_inet_ntop(AF_INET, &_v4_addr, v4_ip, sizeof(v4_ip));
#ifdef __APPLE__
	if (publiccomponent_GetSystemVersion() >= 9.2f) {//higher than iOS9.2
		error = getaddrinfo(v4_ip, NULL, &hints, &res0);
	} else {//lower than iOS9.2 or other platform
#endif
		error = getaddrinfo("ipv4only.arpa", NULL, &hints, &res0);
#ifdef __APPLE__
	}
#endif

	bool ret = false;
    if (error==0) {
    	for (res = res0; res; res = res->ai_next) {
    		char ip_buf[64] = {0};

    		if (AF_INET6 == res->ai_family) {
#ifdef __APPLE__
				if (publiccomponent_GetSystemVersion() >= 9.2f) { //higher than iOS9.2
					//copy all 16 bytes
					memcpy ( (char*)&_v6_addr, (char*)&((((sockaddr_in6*)res->ai_addr)->sin6_addr).s6_addr32), 16);
					ret = true;
					break;
				} else { //lower than iOS9.2 or other platform
#endif

	    			if (IsNat64AddrValid((struct in6_addr*)&(((sockaddr_in6*)res->ai_addr)->sin6_addr))) {
						ReplaceNat64WithV4IP((struct in6_addr*)&(((sockaddr_in6*)res->ai_addr)->sin6_addr) , &_v4_addr);
						memcpy ( (char*)&_v6_addr, (char*)&((((sockaddr_in6*)res->ai_addr)->sin6_addr).s6_addr32), 16);
						const char* ip_str = socket_inet_ntop(AF_INET6, &_v6_addr, ip_buf, sizeof(ip_buf));
						xdebug2(TSF"AF_INET6 v4_ip=%_, nat64 ip_str = %_", v4_ip, ip_str);
		    			ret = true;
		    			break;
	    			} else {
	    				xerror2(TSF"Nat64 addr invalid, =%_",
	    						strutil::Hex2Str((char*)&(((sockaddr_in6*)res->ai_addr)->sin6_addr), 16));
	    				ret = false;
	    			}
#ifdef __APPLE__
				}
#endif

    		} else if (AF_INET == res->ai_family){
    			const char* ip_str = socket_inet_ntop(AF_INET, &(((sockaddr_in*)res->ai_addr)->sin_addr), ip_buf, sizeof(ip_buf));
    			xinfo2(TSF"AF_INET ip_str = %_", ip_str);
    			ret = false;
    		} else {
    			xerror2(TSF"invalid ai_family = %_", res->ai_family);
    			ret = false;
    		}

    	}
    } else {
    	xerror2(TSF"getaddrinfo error = %_", error);
    	ret = false;
    }

    freeaddrinfo(res0);
    return ret;

}

bool ConvertV4toNat64V6(const std::string& _v4_ip, std::string& _nat64_v6_ip) {
	struct in_addr v4_addr = {0};
	int pton_ret = socket_inet_pton(AF_INET, _v4_ip.c_str(), &v4_addr);
	if (0==pton_ret) {
    	xwarn2(TSF"param error. %_ is not v4 ip", _v4_ip.c_str());
    	return false;
    }

	struct in6_addr v6_addr = {{{0}}};
	if (ConvertV4toNat64V6(v4_addr, v6_addr)) {
		char v6_ip[64] = {0};
		socket_inet_ntop(AF_INET6, &v6_addr, v6_ip, sizeof(v6_ip));
		_nat64_v6_ip = std::string(v6_ip);
		return true;
	}
	return false;
}

///----------------------------------------------------------------------------
bool  GetNetworkNat64Prefix(struct in6_addr& _nat64_prefix_in6) {
    xdebug_function();
    if (ELocalIPStack_IPv6 != local_ipstack_detect()) {
    	xwarn2(TSF"Current Network is not ELocalIPStack_IPv6, no need GetNetworkNat64Prefix.");
		return false;
    }
	struct addrinfo hints, *res=NULL, *res0=NULL;
	int error = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	bool ret = false;
#ifdef __APPLE__
	if (publiccomponent_GetSystemVersion() >= 9.2f) {
		error = getaddrinfo("192.0.2.1", NULL, &hints, &res0);
	} else {
#endif
		error = getaddrinfo("ipv4only.arpa", NULL, &hints, &res0);
#ifdef __APPLE__
	}
#endif
    if (error==0) {
    	for (res = res0; res; res = res->ai_next) {
    		char ip_buf[64] = {0};

    		if (AF_INET6 == res->ai_family) {
    			memcpy ( (char*)&(_nat64_prefix_in6.s6_addr32), (char*)&((((sockaddr_in6*)res->ai_addr)->sin6_addr).s6_addr32), 12);

    			ret = true;
    			break;

    		} else if (AF_INET == res->ai_family){
    			const char* ip_str = socket_inet_ntop(AF_INET, &(((sockaddr_in*)res->ai_addr)->sin_addr), ip_buf, sizeof(ip_buf));
    			xinfo2(TSF"AF_INET ip_str = %_", ip_str);
    			ret = false;
    		} else {
    			xerror2(TSF"invalid ai_family = %_", res->ai_family);
    			ret = false;
    		}

    	}
    } else {
    	xerror2(TSF" getaddrinfo error = %_", error);
    	ret = false;
    }

    freeaddrinfo(res0);
    return ret;
}

bool  GetNetworkNat64Prefix(std::string& _nat64_prefix) {
	struct in6_addr nat64_prefix_in6;
	memset(&nat64_prefix_in6, 0, sizeof(nat64_prefix_in6));

	if (GetNetworkNat64Prefix(nat64_prefix_in6)) {
		char ip_buf[64] = {0};
		const char* prefix_str = socket_inet_ntop(AF_INET6, &nat64_prefix_in6, ip_buf, sizeof(ip_buf));
		_nat64_prefix = std::string(prefix_str);
		return true;
	}
	return false;
}
