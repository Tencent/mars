//
//  routequery.cpp
//  sdt
//
//  Created by perryzhou on 23/05/2018.
//  Copyright © 2018 Tencent. All rights reserved.
//

#include "local_routetable.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "comm/xlogger/xlogger.h"

#if defined(__ANDROID__)
std::string get_local_route_table(){
    const char* cmd = "ip route list table all";
    xinfo2(TSF"popen cmd=%_", cmd);
    
    std::string result;
    FILE* pp = popen(cmd, "r");
    if (!pp) {
        xerror2(TSF"popen error:%_", strerror(errno));
        return result;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), pp) != NULL) {
        result.append(line, strlen(line));
    }
    pclose(pp);
    
    return result;
}
#elif defined(__APPLE__)

// code modified from https://opensource.apple.com/source/network_cmds/network_cmds-457/netstat.tproj/route.c
// output like `netstat -nlr`

#include <stdint.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <sys/sysctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <time.h>
#include <comm/objc/route.h>

/* alignment constraint for routing socket */
#define ROUNDUP(a) \
((a) > 0 ? (1 + (((a) - 1) | (sizeof(uint32_t) - 1))) : sizeof(uint32_t))
#define ADVANCE(x, n) (x += ROUNDUP((n)->sa_len))

/*
 * Definitions for showing gateway flags.
 */
struct bits {
    uint32_t    b_mask;
    char    b_val;
} bits[] = {
    { RTF_UP,    'U' },
    { RTF_GATEWAY,    'G' },
    { RTF_HOST,    'H' },
    { RTF_REJECT,    'R' },
    { RTF_DYNAMIC,    'D' },
    { RTF_MODIFIED,    'M' },
    { RTF_MULTICAST,'m' },
    { RTF_DONE,    'd' }, /* Completed -- for routing messages only */
    { RTF_CLONING,    'C' },
    { RTF_XRESOLVE,    'X' },
    { RTF_LLINFO,    'L' },
    { RTF_STATIC,    'S' },
    { RTF_PROTO1,    '1' },
    { RTF_PROTO2,    '2' },
    { RTF_WASCLONED,'W' },
    { RTF_PRCLONING,'c' },
    { RTF_PROTO3,    '3' },
    { RTF_BLACKHOLE,'B' },
    { RTF_BROADCAST,'b' },
    { RTF_IFSCOPE,    'I' },
    { RTF_IFREF,    'i' },
    { RTF_PROXY,    'Y' },
    { RTF_ROUTER,    'r' },
    { 0 }
};

typedef union {
    uint32_t dummy;        /* Helps align structure. */
    struct    sockaddr u_sa;
    u_short    u_data[128];
} sa_u;

static std::string np_rtentry __P((struct rt_msghdr2 *));
static std::string p_sockaddr __P((struct sockaddr *, struct sockaddr *, int, int));
static std::string p_flags __P((int, const char *));
static uint32_t forgemask __P((uint32_t));
static void domask __P((char *, uint32_t, uint32_t));

static std::string p_flags(int f, const char *format)
{
    char line[256];
    char name[33], *flags;
    struct bits *p = bits;
    
    for (flags = name; p->b_mask; p++)
        if (p->b_mask & f)
            *flags++ = p->b_val;
    *flags = '\0';
    snprintf(line, sizeof(line), format, name);
    return line;
}

std::string routename(uint32_t in)
{
    char line[MAXHOSTNAMELEN];
#define C(x)    ((x) & 0xff)
    in = ntohl(in);
    snprintf(line, sizeof(line), "%u.%u.%u.%u",
             C(in >> 24), C(in >> 16), C(in >> 8), C(in));
    return (line);
}

static uint32_t
forgemask(uint32_t a)
{
    uint32_t m;
    
    if (IN_CLASSA(a))
        m = IN_CLASSA_NET;
    else if (IN_CLASSB(a))
        m = IN_CLASSB_NET;
    else
        m = IN_CLASSC_NET;
    return (m);
}

static void
domask(char *dst, uint32_t addr, uint32_t mask)
{
    int b, i;
    
    if (!mask || (forgemask(addr) == mask)) {
        *dst = '\0';
        return;
    }
    i = 0;
    for (b = 0; b < 32; b++)
        if (mask & (1 << b)) {
            int bb;
            
            i = b;
            for (bb = b+1; bb < 32; bb++)
                if (!(mask & (1 << bb))) {
                    i = -1;    /* noncontig */
                    break;
                }
            break;
        }
    if (i == -1)
        snprintf(dst, sizeof(dst), "&0x%x", mask);
    else
        snprintf(dst, sizeof(dst), "/%d", 32-i);
}

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
std::string netname(uint32_t in, uint32_t mask)
{
    char line[MAXHOSTNAMELEN];
    uint32_t omask, dmask;
    uint32_t i;
    
    i = ntohl(in);
    dmask = forgemask(i);
    omask = mask;
    
    switch (dmask) {
        case IN_CLASSA_NET:
            if ((i & IN_CLASSA_HOST) == 0) {
                snprintf(line, sizeof(line), "%u", C(i >> 24));
                break;
            }
            /* FALLTHROUGH */
        case IN_CLASSB_NET:
            if ((i & IN_CLASSB_HOST) == 0) {
                snprintf(line, sizeof(line), "%u.%u",
                         C(i >> 24), C(i >> 16));
                break;
            }
            /* FALLTHROUGH */
        case IN_CLASSC_NET:
            if ((i & IN_CLASSC_HOST) == 0) {
                snprintf(line, sizeof(line), "%u.%u.%u",
                         C(i >> 24), C(i >> 16), C(i >> 8));
                break;
            }
            /* FALLTHROUGH */
        default:
            snprintf(line, sizeof(line), "%u.%u.%u.%u",
                     C(i >> 24), C(i >> 16), C(i >> 8), C(i));
            break;
    }
    domask(line+strlen(line), i, omask);
    return (line);
}


std::string netname6(struct sockaddr_in6 *sa6, struct sockaddr *sam)
{
    char line[MAXHOSTNAMELEN];
    u_char *lim;
    int masklen, illegal = 0, flag = NI_WITHSCOPEID;
    struct in6_addr *mask = sam ? &((struct sockaddr_in6 *)sam)->sin6_addr : 0;
    
    if (sam && sam->sa_len == 0) {
        masklen = 0;
    } else if (mask) {
        u_char *p = (u_char *)mask;
        for (masklen = 0, lim = p + 16; p < lim; p++) {
            switch (*p) {
                case 0xff:
                    masklen += 8;
                    break;
                case 0xfe:
                    masklen += 7;
                    break;
                case 0xfc:
                    masklen += 6;
                    break;
                case 0xf8:
                    masklen += 5;
                    break;
                case 0xf0:
                    masklen += 4;
                    break;
                case 0xe0:
                    masklen += 3;
                    break;
                case 0xc0:
                    masklen += 2;
                    break;
                case 0x80:
                    masklen += 1;
                    break;
                case 0x00:
                    break;
                default:
                    illegal ++;
                    break;
            }
        }
    } else {
        masklen = 128;
    }
    if (masklen == 0 && IN6_IS_ADDR_UNSPECIFIED(&sa6->sin6_addr))
        return ("default");
    
    flag |= NI_NUMERICHOST;
    getnameinfo((struct sockaddr *)sa6, sa6->sin6_len, line, sizeof(line),
                NULL, 0, flag);
    
    snprintf(&line[strlen(line)], sizeof(line) - strlen(line), "/%d", masklen);
    
    return line;
}

std::string routename6(struct sockaddr_in6 *sa6)
{
    char line[MAXHOSTNAMELEN];
    int flag = NI_WITHSCOPEID;
    /* use local variable for safety */
    struct sockaddr_in6 sa6_local = {sizeof(sa6_local), AF_INET6, };
    
    sa6_local.sin6_addr = sa6->sin6_addr;
    sa6_local.sin6_scope_id = sa6->sin6_scope_id;
    
    flag |= NI_NUMERICHOST;
    
    getnameinfo((struct sockaddr *)&sa6_local, sa6_local.sin6_len,
                line, sizeof(line), NULL, 0, flag);
    
    return line;
}

void
upHex(char *p0)
{
    char *p = p0;
    
    for (; *p; p++)
        switch (*p) {
                
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                *p += ('A' - 'a');
                break;
        }
}

/*
 * Print address family header before a section of the routing table.
 */
std::string pr_family(int af)
{
    char line[64];
    const char *afname;
    switch (af) {
        case AF_INET:
            afname = "Internet";
            break;
        case AF_INET6:
            afname = "Internet6";
            break;
        case AF_IPX:
            afname = "IPX";
            break;
        default:
            afname = NULL;
            break;
    }
    if (afname)
        snprintf(line, sizeof(line), "\n%s:\n", afname);
    else
        snprintf(line, sizeof(line), "\nProtocol Family %d:\n", af);
    
    return line;
}

/* column widths; each followed by one space */
#define    WID_DST(af) \
((af) == AF_INET6 ? (true ? 39 : (true ? 39: 18)) : 18)
#define    WID_GW(af) \
((af) == AF_INET6 ? (true ? 31 : (true ? 31 : 18)) : 18)
#define    WID_IF(af)    ((af) == AF_INET6 ? 8 : 7)


/*
 * Print header for routing table columns.
 */
std::string
pr_rthdr(int af)
{
    char line[256];
    
    snprintf(line, sizeof(line), "%-8.8s ","Address");
    if (af == AF_INET)
            snprintf(line, sizeof(line), "%-*.*s %-*.*s %-10.10s %6.6s %8.8s %6.6s %*.*s %6s\n",
                   WID_DST(af), WID_DST(af), "Destination",
                   WID_GW(af), WID_GW(af), "Gateway",
                   "Flags", "Refs", "Use", "Mtu",
                   WID_IF(af), WID_IF(af), "Netif", "Expire");
    return line;
}

/*
 * Print routing tables.
 */
std::string get_local_route_table()
{
    std::string result;
    
    size_t needed = 0;
    int mib[6] = {0};
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = 0;
    mib[4] = NET_RT_DUMP2;
    mib[5] = 0;
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
        result = "fail: sysctl: net.route.0.0.dump estimate";
        return result;
    }
    
    char *buf = nullptr;
    if ((buf = (char*)malloc(needed)) == 0) {
        result = "fail: malloc fail";
        return result;
    }
    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
        result = "fail: sysctl: net.route.0.0.dump";
        free(buf);
        return result;
    }
    
    const char* lim  = buf + needed;
    const char* next = buf;
    while (next < lim) {
        struct rt_msghdr2 *rtm = (struct rt_msghdr2 *)next;
        result += np_rtentry(rtm);
        next += rtm->rtm_msglen;
    }
    free(buf);
    return result;
}

static void get_rtaddrs(int addrs, struct sockaddr *sa, struct sockaddr **rti_info)
{
    int i;
    
    for (i = 0; i < RTAX_MAX; i++) {
        if (addrs & (1 << i)) {
            rti_info[i] = sa;
            sa = (struct sockaddr *)(ROUNDUP(sa->sa_len) + (char *)sa);
        } else {
            rti_info[i] = NULL;
        }
    }
}

static std::string np_rtentry(struct rt_msghdr2 *rtm)
{
    std::string result;
    char line[256] = {0};
    
    struct sockaddr *sa = (struct sockaddr *)(rtm + 1);
    struct sockaddr *rti_info[RTAX_MAX];
    static int old_fam;
    int fam = 0;
    u_short lastindex = 0xffff;
    static char ifname[IFNAMSIZ + 1];
    sa_u addr, mask;
    
    /*
     * Don't print protocol-cloned routes unless -a.
     */
    if ((rtm->rtm_flags & RTF_WASCLONED) &&
        (rtm->rtm_parentflags & RTF_PRCLONING)) {
        return "";
    }
    
    fam = sa->sa_family;
    if (fam != old_fam) {
        result += pr_family(fam);
        result += pr_rthdr(fam);
        old_fam = fam;
    }
    get_rtaddrs(rtm->rtm_addrs, sa, rti_info);
    bzero(&addr, sizeof(addr));
    if ((rtm->rtm_addrs & RTA_DST))
        bcopy(rti_info[RTAX_DST], &addr, rti_info[RTAX_DST]->sa_len);
    bzero(&mask, sizeof(mask));
    if ((rtm->rtm_addrs & RTA_NETMASK))
        bcopy(rti_info[RTAX_NETMASK], &mask, rti_info[RTAX_NETMASK]->sa_len);
    result += p_sockaddr(&addr.u_sa, &mask.u_sa, rtm->rtm_flags,
               WID_DST(addr.u_sa.sa_family));
    
    result += p_sockaddr(rti_info[RTAX_GATEWAY], NULL, RTF_HOST,
               WID_GW(addr.u_sa.sa_family));
    
    result += p_flags(rtm->rtm_flags, "%-10.10s ");
    
    if (addr.u_sa.sa_family == AF_INET) {
        snprintf(line, sizeof(line), "%6u %8u ", rtm->rtm_refcnt, (unsigned int)rtm->rtm_use);
        result += line;
        if (rtm->rtm_rmx.rmx_mtu != 0){
            snprintf(line, sizeof(line), "%6u ", rtm->rtm_rmx.rmx_mtu);
            result += line;
        }
    }
    if (rtm->rtm_index != lastindex) {
        if_indextoname(rtm->rtm_index, ifname);
        lastindex = rtm->rtm_index;
    }
    snprintf(line, sizeof(line), "%*.*s", WID_IF(addr.u_sa.sa_family),
           WID_IF(addr.u_sa.sa_family), ifname);
    result += line;
    
    if (rtm->rtm_rmx.rmx_expire) {
        time_t expire_time;
        
        if ((expire_time =
             rtm->rtm_rmx.rmx_expire - time((time_t *)0)) > 0){
            snprintf(line, sizeof(line), " %6d", (int)expire_time);
            result += line;
        }
    }
    result += "\n";
    
    return result;
}

std::string p_sockaddr(struct sockaddr *sa, struct sockaddr *mask, int flags, int width)
{
    std::string result;
    
    switch(sa->sa_family) {
        case AF_INET: {
            struct sockaddr_in *sin = (struct sockaddr_in *)sa;
            
            if ((sin->sin_addr.s_addr == INADDR_ANY) &&
                mask &&
                (ntohl(((struct sockaddr_in *)mask)->sin_addr.s_addr) == 0L || mask->sa_len == 0))
                result = "default" ;
            else if (flags & RTF_HOST)
                result = routename(sin->sin_addr.s_addr);
            else if (mask)
                result = netname(sin->sin_addr.s_addr,
                             ntohl(((struct sockaddr_in *)mask)->
                                   sin_addr.s_addr));
            else
                result = netname(sin->sin_addr.s_addr, 0L);
            break;
        }

        case AF_INET6: {
            struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)sa;
            struct in6_addr *in6 = &sa6->sin6_addr;
            
            /*
             * XXX: This is a special workaround for KAME kernels.
             * sin6_scope_id field of SA should be set in the future.
             */
            if (IN6_IS_ADDR_LINKLOCAL(in6) ||
                IN6_IS_ADDR_MC_NODELOCAL(in6) ||
                IN6_IS_ADDR_MC_LINKLOCAL(in6)) {
                /* XXX: override is ok? */
                sa6->sin6_scope_id = (u_int32_t)ntohs(*(u_short *)&in6->s6_addr[2]);
                *(u_short *)&in6->s6_addr[2] = 0;
            }
            
            if (flags & RTF_HOST)
                result = routename6(sa6);
            else if (mask)
                result = netname6(sa6, mask);
            else
                result = netname6(sa6, NULL);
            break;
        }
    }
    
    char line[256] = {0};
    if (width < 0 ) {
        snprintf(line, sizeof(line), "%s ", result.c_str());
    } else {
        snprintf(line, sizeof(line), "%-*s ", width, result.c_str());
    }
    
    return line;
}

#else
std::string get_local_route_table(){
    return "*** NOT IMPL ***";
}
#endif

