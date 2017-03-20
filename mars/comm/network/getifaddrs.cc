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
//  ifaddr.cpp
//  comm
//
//  Created by yerungui on 14-10-31.
//

#include "comm/network/getifaddrs.h"

#if (!UWP && !WIN32)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>

#ifdef ANDROID
#include "comm/jni/ifaddrs.h"
#else
#include <ifaddrs.h>
#endif

bool getifaddrs_ipv4(std::vector<ifaddrinfo_ipv4_t>& _addrs) {
    struct ifaddrs* ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        sockaddr_in* sa = (struct sockaddr_in*) ifa->ifa_addr;

        if (NULL == sa) continue;

        if (ifa->ifa_addr->sa_family == AF_INET && 0 == (ifa->ifa_flags & IFF_LOOPBACK)) {
            ifaddrinfo_ipv4_t addr;
            addr.ifa_name = ifa->ifa_name;
            addr.ifa_ip = sa->sin_addr.s_addr;
            inet_ntop(sa->sin_family,  &(sa->sin_addr), addr.ip, sizeof(addr.ip));

            _addrs.push_back(addr);
        }
    }

    freeifaddrs(ifap);
    return !_addrs.empty();
}

bool getifaddrs_ipv4(ifaddrinfo_ipv4_t& _addr) {
    struct ifaddrs* ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        sockaddr_in* sa = (struct sockaddr_in*) ifa->ifa_addr;

        if (NULL == sa) continue;

        if (ifa->ifa_addr->sa_family == AF_INET && 0 == (ifa->ifa_flags & IFF_LOOPBACK)) {
            _addr.ifa_name = ifa->ifa_name;
            _addr.ifa_ip = sa->sin_addr.s_addr;
            inet_ntop(sa->sin_family,  &(sa->sin_addr), _addr.ip, sizeof(_addr.ip));

            freeifaddrs(ifap);
            return true;
        }
    }

    freeifaddrs(ifap);
    return false;
}

bool getifaddrs_ipv4_lan(ifaddrinfo_ipv4_t& _addr) {
    struct ifaddrs* ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        sockaddr_in* sa = (struct sockaddr_in*) ifa->ifa_addr;

        if (NULL == sa) continue;

        if (ifa->ifa_addr->sa_family == AF_INET
                && !(ifa->ifa_flags & IFF_LOOPBACK)
                && !(ifa->ifa_flags & IFF_POINTOPOINT)
                && (ifa->ifa_flags & IFF_BROADCAST)
                && (ifa->ifa_flags & IFF_RUNNING)) {
            _addr.ifa_name = ifa->ifa_name;
            _addr.ifa_ip = sa->sin_addr.s_addr;
            inet_ntop(sa->sin_family,  &(sa->sin_addr), _addr.ip, sizeof(_addr.ip));

            freeifaddrs(ifap);
            return true;
        }
    }

    freeifaddrs(ifap);
    return false;
}

bool getifaddrs_ipv4_lan(std::vector<ifaddrinfo_ipv4_t>& _addrs) {
    struct ifaddrs* ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        sockaddr_in* sa = (struct sockaddr_in*) ifa->ifa_addr;

        if (NULL == sa) continue;

        if (ifa->ifa_addr->sa_family == AF_INET
                && !(ifa->ifa_flags & IFF_LOOPBACK)
                && !(ifa->ifa_flags & IFF_POINTOPOINT)
                && (ifa->ifa_flags & IFF_BROADCAST)
                && (ifa->ifa_flags & IFF_RUNNING)) {
            ifaddrinfo_ipv4_t addr;

            addr.ifa_name = ifa->ifa_name;
            addr.ifa_ip = sa->sin_addr.s_addr;
            inet_ntop(sa->sin_family,  &(sa->sin_addr), addr.ip, sizeof(addr.ip));

            _addrs.push_back(addr);
        }
    }

    freeifaddrs(ifap);
    return !_addrs.empty();
}

bool getifaddrs_ipv4_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter) {
    struct ifaddrs* ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        sockaddr_in* sa = (struct sockaddr_in*) ifa->ifa_addr;

        if (NULL == sa) continue;

        if (ifa->ifa_addr->sa_family == AF_INET
            && 0 == (ifa->ifa_flags & IFF_LOOPBACK)
            && (ifa->ifa_flags & IFF_RUNNING)
            && _flags_filter == (_flags_filter & ifa->ifa_flags)) {

            ifaddrinfo_ip_t addr;
            addr.ifa_family = AF_INET;
            addr.ifa_name = ifa->ifa_name;
            addr.ifa_ip[0] = sa->sin_addr.s_addr;
            addr.ifa_flags = ifa->ifa_flags;
            inet_ntop(sa->sin_family,  &(sa->sin_addr), addr.ip, sizeof(addr.ip));

            _addrs.push_back(addr);
        }
    }

    freeifaddrs(ifap);
    return !_addrs.empty();
}

bool getifaddrs_ipv6_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter) {
    struct ifaddrs* ifap, *ifa;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        sockaddr_in6* sa = (struct sockaddr_in6*) ifa->ifa_addr;

        if (NULL == sa) continue;

        if (ifa->ifa_addr->sa_family == AF_INET6
            && 0 == (ifa->ifa_flags & IFF_LOOPBACK)
            && (ifa->ifa_flags & IFF_RUNNING)
            && _flags_filter == (_flags_filter & ifa->ifa_flags)) {

            ifaddrinfo_ip_t addr;
            addr.ifa_family = AF_INET6;
            addr.ifa_name = ifa->ifa_name;
            memcpy(addr.ifa_ip, &sa->sin6_addr, sizeof(addr.ifa_ip));
            addr.ifa_flags = ifa->ifa_flags;
            inet_ntop(sa->sin6_family,  &(sa->sin6_addr), addr.ip, sizeof(addr.ip));

            _addrs.push_back(addr);
        }
    }

    freeifaddrs(ifap);
    return !_addrs.empty();
}
#else
#ifdef WIN32
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include "../socket/unix_socket.h"
#pragma comment(lib, "wsock32.lib")
#endif // WIN32

bool getifaddrs_ipv4(std::vector<ifaddrinfo_ipv4_t>& _addrs){
	return false;
}

bool getifaddrs_ipv4(ifaddrinfo_ipv4_t& _addr) {
	return false;
}

bool getifaddrs_ipv4_lan(ifaddrinfo_ipv4_t& _addr) {
	return false;
}

bool getifaddrs_ipv4_lan(std::vector<ifaddrinfo_ipv4_t>& _addrs) {
	ULONG outBufLen = 0;
	GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen);

	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
	GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST, NULL, pAddresses, &outBufLen);

	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	LPSOCKADDR addr = NULL;
	pCurrAddresses = pAddresses;

	ifaddrinfo_ipv4_t addr_t;
	while (pCurrAddresses)
	{
		if (pCurrAddresses->OperStatus != IfOperStatusUp)
		{
			pCurrAddresses = pCurrAddresses->Next;
			continue;
		}
		pUnicast = pCurrAddresses->FirstUnicastAddress;

		while (pUnicast)
		{
			addr = pUnicast->Address.lpSockaddr;
			if (addr->sa_family == AF_INET && pCurrAddresses->IfType != MIB_IF_TYPE_LOOPBACK)
			{
				sockaddr_in  *sa_in = (sockaddr_in *)addr;
				char* strIP = ::inet_ntoa((sa_in->sin_addr));
				addr_t.ifa_name = strIP;
				addr_t.ifa_ip = sa_in->sin_addr.S_un.S_addr;
				socket_inet_ntop(sa_in->sin_family, &(sa_in->sin_addr), addr_t.ip, sizeof(addr_t.ip));
				if (pCurrAddresses->IfType == IF_TYPE_IEEE80211)
				{
					_addrs.insert(_addrs.begin(), addr_t);
				}
				else
				{
					_addrs.push_back(addr_t);
				}
			}
			pUnicast = pUnicast->Next;
		}
		pCurrAddresses = pCurrAddresses->Next;
	}
	free(pAddresses);
	return !_addrs.empty();
}

bool getifaddrs_ipv4_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter) {
	return false;
}

bool getifaddrs_ipv6_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter) {
	return false;
}
#endif
