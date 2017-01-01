/* $Id: getgateway.c,v 1.12 2008/10/06 10:04:16 nanard Exp $ */
/* libnatpmp
 * Copyright (c) 2007-2008, Thomas BERNARD <miniupnp@free.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */
#include <stdio.h>
#include <ctype.h>
#ifndef WIN32
#include <netinet/in.h>
#include <sys/param.h>
#endif

/* There is no portable method to get the default route gateway.
 * So below are three differents functions implementing this.
 * Parsing /proc/net/route is for linux.
 * sysctl is the way to access such informations on BSD systems.
 * Many systems should provide route information through raw PF_ROUTE
 * sockets. */

#define __ROUTE_HEADER_FILE__ <net/route.h>

#ifdef __linux__
#define USE_PROC_NET_ROUTE
#undef USE_SOCKET_ROUTE
#undef USE_SYSCTL_NET_ROUTE
#endif

#ifdef BSD
#undef USE_PROC_NET_ROUTE
#define USE_SOCKET_ROUTE
#undef USE_SYSCTL_NET_ROUTE
#endif

#ifdef __APPLE__
#undef USE_PROC_NET_ROUTE
#undef USE_SOCKET_ROUTE
#define USE_SYSCTL_NET_ROUTE

#include <TargetConditionals.h>
#if !TARGET_OS_SIMULATOR
#undef __ROUTE_HEADER_FILE__
#define __ROUTE_HEADER_FILE__  "comm/objc/route.h"
#endif

#endif //__APPLE__

#if (defined(sun) && defined(__SVR4))
#undef USE_PROC_NET_ROUTE
#define USE_SOCKET_ROUTE
#undef USE_SYSCTL_NET_ROUTE
#endif

#ifdef WIN32
#undef USE_PROC_NET_ROUTE
#undef USE_SOCKET_ROUTE
#undef USE_SYSCTL_NET_ROUTE
#define USE_WIN32_CODE
#include <winsock2.h>
#endif

#ifdef USE_SYSCTL_NET_ROUTE
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include __ROUTE_HEADER_FILE__
#endif
#ifdef USE_SOCKET_ROUTE
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include __ROUTE_HEADER_FILE__
#endif
#ifdef WIN32
#include <unknwn.h>
#include <winreg.h>
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_LENGTH 16383
#endif
#include "comm/network/getgateway.h"

#ifndef WIN32
#define SUCCESS (0)
#define FAILED  (-1)
#endif

typedef uint32_t in_addr_t;
#ifdef USE_PROC_NET_ROUTE
int getdefaultgateway(struct in_addr * addr)
{
    long d, g;
    char buf[256];
    int line = 0;
    FILE * f;
    char * p;
    f = fopen("/proc/net/route", "r");
    if (!f)
        return FAILED;
    while(fgets(buf, sizeof(buf), f)) {
        if (line > 0) {
            p = buf;
            while(*p && !isspace(*p))
                p++;
            while(*p && isspace(*p))
                p++;
            if (sscanf(p, "%lx%lx", &d, &g)==2) {
                if (d == 0) { /* default */
                	addr->s_addr = (in_addr_t)g;
                    fclose(f);
                    return SUCCESS;
                }
            }
        }
        line++;
    }
    /* default route not found ! */
    if (f)
        fclose(f);
    return FAILED;
}

int getdefaultgateway6(struct in6_addr * addr) {
	return FAILED;
}

#endif /* #ifdef USE_PROC_NET_ROUTE */


#ifdef USE_SYSCTL_NET_ROUTE

#define ROUNDUP(a) \
    ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

int getdefaultgateway(struct in_addr * addr)
{
#if 0
    /* net.route.0.inet.dump.0.0 ? */
    int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET,
                 NET_RT_DUMP, 0, 0/*tableid*/};
#endif
    /* net.route.0.inet.flags.gateway */
    int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET,
                 NET_RT_FLAGS, RTF_GATEWAY};
    size_t l;
    char * buf, * p;
    struct rt_msghdr * rt;
    struct sockaddr * sa;
    struct sockaddr * sa_tab[RTAX_MAX];
    int i;
    int r = FAILED;
    if (sysctl(mib, sizeof(mib)/sizeof(int), 0, &l, 0, 0) < 0) {
        return FAILED;
    }
    if (l>0) {
        buf = malloc(l);
        if (sysctl(mib, sizeof(mib)/sizeof(int), buf, &l, 0, 0) < 0) {
            free(buf);
            return FAILED;
        }
        for(p=buf; p<buf+l; p+=rt->rtm_msglen) {
            rt = (struct rt_msghdr *)p;
            sa = (struct sockaddr *)(rt + 1);
            for(i=0; i<RTAX_MAX; i++) {
                if (rt->rtm_addrs & (1 << i)) {
                    sa_tab[i] = sa;
                    sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
                } else {
                    sa_tab[i] = NULL;
                }
            }
            if ( ((rt->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY))
              && sa_tab[RTAX_DST]->sa_family == AF_INET) {
//              && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET) {
                if (((struct sockaddr_in *)sa_tab[RTAX_DST])->sin_addr.s_addr == 0) {
                    *addr = ((struct sockaddr_in *)(sa_tab[RTAX_GATEWAY]))->sin_addr;
                    r = SUCCESS;
                    break;
                }
            }
        }
        free(buf);
    }
    return r;
}

int getdefaultgateway6(struct in6_addr * addr)
{
    /* net.route.0.inet6.flags.gateway */
    int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET6,
        NET_RT_FLAGS, RTF_GATEWAY};
    size_t l;
    char * buf, * p;
    struct rt_msghdr * rt;
    struct sockaddr * sa;
    struct sockaddr * sa_tab[RTAX_MAX];
    int i;
    int r = FAILED;
    if (sysctl(mib, sizeof(mib)/sizeof(int), 0, &l, 0, 0) < 0) {
        return FAILED;
    }
    if (l>0) {
        buf = malloc(l);
        if (sysctl(mib, sizeof(mib)/sizeof(int), buf, &l, 0, 0) < 0) {
            free(buf);
            return FAILED;
        }
        for(p=buf; p<buf+l; p+=rt->rtm_msglen) {
            rt = (struct rt_msghdr *)p;
            sa = (struct sockaddr *)(rt + 1);
            for(i=0; i<RTAX_MAX; i++) {
                if (rt->rtm_addrs & (1 << i)) {
                    sa_tab[i] = sa;
                    sa = (struct sockaddr *)((char *)sa + ROUNDUP(sa->sa_len));
                } else {
                    sa_tab[i] = NULL;
                }
            }
            if ( ((rt->rtm_addrs & (RTA_DST|RTA_GATEWAY)) == (RTA_DST|RTA_GATEWAY))
               && sa_tab[RTAX_DST]->sa_family == AF_INET6) {
//               && sa_tab[RTAX_GATEWAY]->sa_family == AF_INET6) {
                if (IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6 *)sa_tab[RTAX_DST])->sin6_addr)) {
                    *addr = ((struct sockaddr_in6 *)(sa_tab[RTAX_GATEWAY]))->sin6_addr;
                    r = SUCCESS;
                    break;
                }
            }
        }
        free(buf);
    }
    return r;
}
#endif /* #ifdef USE_SYSCTL_NET_ROUTE */


#ifdef USE_SOCKET_ROUTE
/* Thanks to Darren Kenny for this code */
#define NEXTADDR(w, u) \
        if (rtm_addrs & (w)) {\
            l = sizeof(struct sockaddr); memmove(cp, &(u), l); cp += l;\
        }

#define rtm m_rtmsg.m_rtm

struct {
  struct rt_msghdr m_rtm;
  char       m_space[512];
} m_rtmsg;

int getdefaultgateway(in_addr_t *addr)
{
  int s, seq, l, rtm_addrs, i;
  pid_t pid;
  struct sockaddr so_dst, so_mask;
  char *cp = m_rtmsg.m_space; 
  struct sockaddr *gate = NULL, *sa;
  struct rt_msghdr *msg_hdr;

  pid = getpid();
  seq = 0;
  rtm_addrs = RTA_DST | RTA_NETMASK;

  memset(&so_dst, 0, sizeof(so_dst));
  memset(&so_mask, 0, sizeof(so_mask));
  memset(&rtm, 0, sizeof(struct rt_msghdr));

  rtm.rtm_type = RTM_GET;
  rtm.rtm_flags = RTF_UP | RTF_GATEWAY;
  rtm.rtm_version = RTM_VERSION;
  rtm.rtm_seq = ++seq;
  rtm.rtm_addrs = rtm_addrs; 

  so_dst.sa_family = AF_INET;
  so_mask.sa_family = AF_INET;

  NEXTADDR(RTA_DST, so_dst);
  NEXTADDR(RTA_NETMASK, so_mask);

  rtm.rtm_msglen = l = cp - (char *)&m_rtmsg;

  s = socket(PF_ROUTE, SOCK_RAW, 0);

  if (write(s, (char *)&m_rtmsg, l) < 0) {
      close(s);
      return FAILED;
  }

  do {
    l = read(s, (char *)&m_rtmsg, sizeof(m_rtmsg));
  } while (l > 0 && (rtm.rtm_seq != seq || rtm.rtm_pid != pid));
                        
  close(s);

  msg_hdr = &rtm;

  cp = ((char *)(msg_hdr + 1));
  if (msg_hdr->rtm_addrs) {
    for (i = 1; i; i <<= 1)
      if (i & msg_hdr->rtm_addrs) {
        sa = (struct sockaddr *)cp;
        if (i == RTA_GATEWAY )
          gate = sa;

        cp += sizeof(struct sockaddr);
      }
  } else {
      return FAILED;
  }


  if (gate != NULL ) {
      *addr = ((struct sockaddr_in *)gate)->sin_addr.s_addr;
      return SUCCESS;
  } else {
      return FAILED;
  }
}
#endif /* #ifdef USE_SOCKET_ROUTE */

#ifdef USE_WIN32_CODE
int getdefaultgateway(in_addr_t * addr)
{
    HKEY networkCardsKey;
    HKEY networkCardKey;
    HKEY interfacesKey;
    HKEY interfaceKey;
    DWORD i = 0;
    DWORD numSubKeys = 0;
    TCHAR keyName[MAX_KEY_LENGTH];
    DWORD keyNameLength = MAX_KEY_LENGTH;
    TCHAR keyValue[MAX_VALUE_LENGTH];
    DWORD keyValueLength = MAX_VALUE_LENGTH;
    DWORD keyValueType = REG_SZ;
    TCHAR gatewayValue[MAX_VALUE_LENGTH];
    DWORD gatewayValueLength = MAX_VALUE_LENGTH;
    DWORD gatewayValueType = REG_MULTI_SZ;
    int done = 0;
    
    char networkCardsPath[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards";
    char interfacesPath[] = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";
    
    // The windows registry lists its primary network devices in the following location:
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\NetworkCards
    // 
    // Each network device has its own subfolder, named with an index, with various properties:
    // -NetworkCards
    //   -5
    //     -Description = Broadcom 802.11n Network Adapter
    //     -ServiceName = {E35A72F8-5065-4097-8DFE-C7790774EE4D}
    //   -8
    //     -Description = Marvell Yukon 88E8058 PCI-E Gigabit Ethernet Controller
    //     -ServiceName = {86226414-5545-4335-A9D1-5BD7120119AD}
    // 
    // The above service name is the name of a subfolder within:
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces
    // 
    // There may be more subfolders in this interfaces path than listed in the network cards path above:
    // -Interfaces
    //   -{3a539854-6a70-11db-887c-806e6f6e6963}
    //     -DhcpIPAddress = 0.0.0.0
    //     -[more]
    //   -{E35A72F8-5065-4097-8DFE-C7790774EE4D}
    //     -DhcpIPAddress = 10.0.1.4
    //     -DhcpDefaultGateway = 10.0.1.1
    //     -[more]
    //   -{86226414-5545-4335-A9D1-5BD7120119AD}
    //     -DhcpIpAddress = 10.0.1.5
    //     -DhcpDefaultGateay = 10.0.1.1
    //     -[more]
    // 
    // In order to extract this information, we enumerate each network card, and extract the ServiceName value.
    // This is then used to open the interface subfolder, and attempt to extract a DhcpDefaultGateway value.
    // Once one is found, we're done.
    // 
    // It may be possible to simply enumerate the interface folders until we find one with a DhcpDefaultGateway value.
    // However, the technique used is the technique most cited on the web, and we assume it to be more correct.
    
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, // Open registry key or predifined key 
                                     networkCardsPath,   // Name of registry subkey to open
                                     0,                  // Reserved - must be zero
                                     KEY_READ,           // Mask - desired access rights
                                     &networkCardsKey))  // Pointer to output key
    {
        // Unable to open network cards keys
        return -1;
    }
    
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, // Open registry key or predefined key
                                     interfacesPath,     // Name of registry subkey to open
                                     0,                  // Reserved - must be zero
                                     KEY_READ,           // Mask - desired access rights
                                     &interfacesKey))    // Pointer to output key
    {
        // Unable to open interfaces key
        RegCloseKey(networkCardsKey);
        return -1;
    }
    
    // Figure out how many subfolders are within the NetworkCards folder
    RegQueryInfoKey(networkCardsKey, NULL, NULL, NULL, &numSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    
    //printf( "Number of subkeys: %u\n", (unsigned int)numSubKeys);
    
    // Enumrate through each subfolder within the NetworkCards folder
    for(i = 0; i < numSubKeys && !done; i++)
    {
        keyNameLength = MAX_KEY_LENGTH;
        if (ERROR_SUCCESS == RegEnumKeyEx(networkCardsKey, // Open registry key
                                         i,               // Index of subkey to retrieve
                                         keyName,         // Buffer that receives the name of the subkey
                                         &keyNameLength,  // Variable that receives the size of the above buffer
                                         NULL,            // Reserved - must be NULL
                                         NULL,            // Buffer that receives the class string
                                         NULL,            // Variable that receives the size of the above buffer
                                         NULL))           // Variable that receives the last write time of subkey
        {
            if (RegOpenKeyEx(networkCardsKey,  keyName, 0, KEY_READ, &networkCardKey) == ERROR_SUCCESS)
            {
                keyValueLength = MAX_VALUE_LENGTH;
                if (ERROR_SUCCESS == RegQueryValueEx(networkCardKey,   // Open registry key
                                                    "ServiceName",    // Name of key to query
                                                    NULL,             // Reserved - must be NULL
                                                    &keyValueType,    // Receives value type
                                                    keyValue,         // Receives value
                                                    &keyValueLength)) // Receives value length in bytes
                {
                    //printf("keyValue: %s\n", keyValue);
                    
                    if (RegOpenKeyEx(interfacesKey, keyValue, 0, KEY_READ, &interfaceKey) == ERROR_SUCCESS)
                    {
                        gatewayValueLength = MAX_VALUE_LENGTH;
                        if (ERROR_SUCCESS == RegQueryValueEx(interfaceKey,         // Open registry key
                                                            "DhcpDefaultGateway", // Name of key to query
                                                            NULL,                 // Reserved - must be NULL
                                                            &gatewayValueType,    // Receives value type
                                                            gatewayValue,         // Receives value
                                                            &gatewayValueLength)) // Receives value length in bytes
                        {
                            // Check to make sure it's a string
                            if (gatewayValueType == REG_MULTI_SZ || gatewayValueType == REG_SZ)
                            {
                                //printf("gatewayValue: %s\n", gatewayValue);
                                done = 1;
                            }
                        }
                        RegCloseKey(interfaceKey);
                    }
                }
                RegCloseKey(networkCardKey);
            }
        }
    }
    
    RegCloseKey(interfacesKey);
    RegCloseKey(networkCardsKey);
    
    if (done)
    {
        *addr = inet_addr(gatewayValue);
        return 0;
    }
    
    return -1;
}
#endif /* #ifdef USE_WIN32_CODE */

