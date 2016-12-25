//
//  in6_macros.h
//  comm
//
//  Created by yerungui on 15/12/15.
//  Copyright (c) 2015年 Tencent. All rights reserved.
//

#ifndef comm_in6_v4mapped_macros_h
#define comm_in6_v4mapped_macros_h

#if !(defined(_WIN32) || defined(__WIN32__) || defined(WIN32))

#include <netinet/in.h>

#ifndef __APPLE__
#define IN6ADDR_V4MAPPED_INIT {{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}}}
#endif

#ifdef __APPLE__
#define	s6_addr16   __u6_addr.__u6_addr16
#define	s6_addr32   __u6_addr.__u6_addr32
#endif

#define	IN6ADDR_NAT64_INIT {{{ 0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}

const in6_addr in6addr_v4mapped_init  = IN6ADDR_V4MAPPED_INIT;
const in6_addr in6addr_nat64_init		= IN6ADDR_NAT64_INIT;

inline void IN6_SET_ADDR_V4MAPPED(in6_addr* a6, const in_addr* a4) {
    *a6 = in6addr_v4mapped_init;
    a6->s6_addr32[3] = a4->s_addr;
}

inline void IN6_SET_ADDR_NAT64(in6_addr* a6, const in_addr* a4) {
    *a6 = in6addr_nat64_init;
    a6->s6_addr32[3] = a4->s_addr;
}

inline bool IN6_IS_ADDR_NAT64(const in6_addr* a6) {
    return a6->s6_addr32[0] == htonl(0x0064ff9b);
}

#else
#include <mstcpip.h>
//TODO impl
#define	IN6ADDR_NAT64_INIT {{{ 0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
const in6_addr in6addr_nat64_init = IN6ADDR_NAT64_INIT;

inline void IN6_SET_ADDR_NAT64(in6_addr* a6, const in_addr* a4) {
	*a6 = in6addr_nat64_init;
	a6->s6_bytes[12] = ((CONST UCHAR *) a4)[0];
	a6->s6_bytes[13] = ((CONST UCHAR *) a4)[1];
	a6->s6_bytes[14] = ((CONST UCHAR *) a4)[2];
	a6->s6_bytes[15] = ((CONST UCHAR *) a4)[3];
}

inline bool IN6_IS_ADDR_NAT64(in6_addr* a6) {
	return a6->s6_words[0] == htons(0x0064) && a6->s6_words[1] == htons(0x0064);
}
#endif

#endif
