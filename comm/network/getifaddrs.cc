//
//  ifaddr.cpp
//  comm
//
//  Created by yerungui on 14-10-31.
//  Copyright (c) 2014年 Tencent. All rights reserved.
//

#include "comm/network/getifaddrs.h"

#if !UWP
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
	return false;
}

bool getifaddrs_ipv4_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter) {
	return false;
}

bool getifaddrs_ipv6_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter) {
	return false;
}
#endif
