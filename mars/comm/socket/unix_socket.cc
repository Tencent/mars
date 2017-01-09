// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#ifndef _WIN32
#include <fcntl.h>
#endif

#include "socket/unix_socket.h"

#ifdef ANDROID
#include <asm/ioctls.h>
#endif

#ifndef _WIN32

int socket_set_nobio(SOCKET fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    if(ret >= 0) {
        long flags = ret | O_NONBLOCK;
        ret = fcntl(fd, F_SETFL, flags);
    }

    return ret;
}
#else
int socket_set_nobio(SOCKET fd) {
    static const int noblock = 1;
    return ioctlsocket(fd, FIONBIO, (u_long*)&noblock);
}
#endif

#ifdef _WIN32
#include <string.h>
#include <windows.h>
#include <WS2tcpip.h>
#include <stdio.h>
#define NS_INADDRSZ 4
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;  // NOLINT
typedef short int16;  // NOLINT
static int socket_inet_pton4(const char *src, void *dst) {
    static const char digits[] = "0123456789";
    int saw_digit, octets, ch;
    unsigned char tmp[NS_INADDRSZ], *tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0') {
        const char *pch;

        if ((pch = strchr(digits, ch)) != NULL) {
            size_t newNum = *tp * 10 + (pch - digits);

            if (newNum > 255)
                return (0);
            *tp = (unsigned char) newNum;
            if (! saw_digit) {
                if (++octets > 4)
                    return (0);
                saw_digit = 1;
            }
        } else if (ch == '.' && saw_digit) {
            if (octets == 4)
                return (0);
            *++tp = 0;
            saw_digit = 0;
        } else
            return (0);
    }
    if (octets < 4)
        return (0);
    memcpy(dst, tmp, NS_INADDRSZ);
    return (1);
}
//https://chromium.googlesource.com/external/webrtc/+/edc6e57a92d2b366871f4c2d2e926748326017b9/webrtc/base/win32.cc
// Helper function for inet_pton for IPv6 addresses.
static int socket_inet_pton6(const char* src, void* dst) {
  // sscanf will pick any other invalid chars up, but it parses 0xnnnn as hex.
  // Check for literal x in the input string.
  const char* readcursor = src;
  char c = *readcursor++;
  while (c) {
    if (c == 'x') {
      return 0;
    }
    c = *readcursor++;
  }
  readcursor = src;
  struct in6_addr an_addr;
  memset(&an_addr, 0, sizeof(an_addr));
  uint16* addr_cursor = (uint16*)(&an_addr.s6_addr[0]);
  uint16* addr_end = (uint16*)(&an_addr.s6_addr[16]);
  int seencompressed = 0; //false c89 not define bool type
  // Addresses that start with "::" (i.e., a run of initial zeros) or
  // "::ffff:" can potentially be IPv4 mapped or compatibility addresses.
  // These have dotted-style IPv4 addresses on the end (e.g. "::192.168.7.1").
  if (*readcursor == ':' && *(readcursor+1) == ':' &&
      *(readcursor + 2) != 0) {
    // Check for periods, which we'll take as a sign of v4 addresses.
    const char* addrstart = readcursor + 2;
    if (strchr(addrstart, '.')) {
      const char* colon = strchr(addrstart, ':');
      if (colon) {
        uint16 a_short;
        int bytesread = 0;
        if (sscanf(addrstart, "%hx%n", &a_short, &bytesread) != 1 ||
            a_short != 0xFFFF || bytesread != 4) {
          // Colons + periods means has to be ::ffff:a.b.c.d. But it wasn't.
          return 0;
        } else {
          an_addr.s6_addr[10] = 0xFF;
          an_addr.s6_addr[11] = 0xFF;
          addrstart = colon + 1;
        }
      }
      struct in_addr v4;
      if (socket_inet_pton4(addrstart, &v4.s_addr)) {
        memcpy(&an_addr.s6_addr[12], &v4, sizeof(v4));
        memcpy(dst, &an_addr, sizeof(an_addr));
        return 1;
      } else {
        // Invalid v4 address.
        return 0;
      }
    }
  }
  // For addresses without a trailing IPv4 component ('normal' IPv6 addresses).
  while (*readcursor != 0 && addr_cursor < addr_end) {
    if (*readcursor == ':') {
      if (*(readcursor + 1) == ':') {
        if (seencompressed) {
          // Can only have one compressed run of zeroes ("::") per address.
          return 0;
        }
        // Hit a compressed run. Count colons to figure out how much of the
        // address is skipped.
        readcursor += 2;
        const char* coloncounter = readcursor;
        int coloncount = 0;
        if (*coloncounter == 0) {
          // Special case - trailing ::.
          addr_cursor = addr_end;
        } else {
          while (*coloncounter) {
            if (*coloncounter == ':') {
              ++coloncount;
            }
            ++coloncounter;
          }
          // (coloncount + 1) is the number of shorts left in the address.
          addr_cursor = addr_end - (coloncount + 1);
          seencompressed = 1;
        }
      } else {
        ++readcursor;
      }
    } else {
      uint16 word;
      int bytesread = 0;
      if (sscanf(readcursor, "%hx%n", &word, &bytesread) != 1) {
        return 0;
      } else {
        *addr_cursor = htons(word);
        ++addr_cursor;
        readcursor += bytesread;
        if (*readcursor != ':' && *readcursor != '\0') {
          return 0;
        }
      }
    }
  }
  if (*readcursor != '\0' || addr_cursor < addr_end) {
    // Catches addresses too short or too long.
    return 0;
  }
  memcpy(dst, &an_addr, sizeof(an_addr));
  return 1;
}
int socket_inet_pton(int af, const char *src, void *dst) {
    switch (af) {
    case AF_INET :
        return socket_inet_pton4 (src, dst);
    case AF_INET6 :
        return socket_inet_pton6 (src, dst);
    default :
        //xerror("EAFNOSUPPORT");
        return 0;
    }
}

static const char *inet_ntop_v4 (const void *src, char *dst, socklen_t size) {
    const char digits[] = "0123456789";
    int i;
    struct in_addr *addr = (struct in_addr *)src;
    u_long a = ntohl(addr->s_addr);
    const char *orig_dst = dst;

    if (size < 16) {
        //xerror("ENOSPC: size = %0", size);
        return NULL;
    }
    for (i = 0; i < 4; ++i) {
        int n = (a >> (24 - i * 8)) & 0xFF;
        int non_zerop = 0;

        if (non_zerop || n / 100 > 0) {
            *dst++ = digits[n / 100];
            n %= 100;
            non_zerop = 1;
        }
        if (non_zerop || n / 10 > 0) {
            *dst++ = digits[n / 10];
            n %= 10;
            non_zerop = 1;
        }
        *dst++ = digits[n];
        if (i != 3)
            *dst++ = '.';
    }
    *dst++ = '\0';
    return orig_dst;
}

// Helper function for inet_ntop for IPv6 addresses.
static const char* inet_ntop_v6(const void* src, char* dst, socklen_t size) {
  if (size < INET6_ADDRSTRLEN) {
    return NULL;
  }
  const uint16* as_shorts =
      reinterpret_cast<const uint16*>(src);
  int runpos[8];
  int current = 1;
  int max = 1;
  int maxpos = -1;
  int run_array_size = sizeof(runpos)/sizeof(runpos[0]);
  // Run over the address marking runs of 0s.
  for (int i = 0; i < run_array_size; ++i) {
    if (as_shorts[i] == 0) {
      runpos[i] = current;
      if (current > max) {
        maxpos = i;
        max = current;
      }
      ++current;
    } else {
      runpos[i] = -1;
      current =1;
    }
  }
  if (max > 1) {
    int tmpmax = maxpos;
    // Run back through, setting -1 for all but the longest run.
    for (int i = run_array_size - 1; i >= 0; i--) {
      if (i > tmpmax) {
        runpos[i] = -1;
      } else if (runpos[i] == -1) {
        // We're less than maxpos, we hit a -1, so the 'good' run is done.
        // Setting tmpmax -1 means all remaining positions get set to -1.
        tmpmax = -1;
      }
    }
  }
  char* cursor = dst;
  // Print IPv4 compatible and IPv4 mapped addresses using the IPv4 helper.
  // These addresses have an initial run of either eight zero-bytes followed
  // by 0xFFFF, or an initial run of ten zero-bytes.
  if (runpos[0] == 1 && (maxpos == 5 ||
                         (maxpos == 4 && as_shorts[5] == 0xFFFF))) {
    *cursor++ = ':';
    *cursor++ = ':';
    if (maxpos == 4) {
      cursor += snprintf(cursor, INET6_ADDRSTRLEN - 2, "ffff:");
    }
    const struct in_addr* as_v4 =
        reinterpret_cast<const struct in_addr*>(&(as_shorts[6]));
    inet_ntop_v4(as_v4, cursor,
                 static_cast<socklen_t>(INET6_ADDRSTRLEN - (cursor - dst)));
  } else {
    for (int i = 0; i < run_array_size; ++i) {
      if (runpos[i] == -1) {
        cursor += snprintf(cursor,
                                      INET6_ADDRSTRLEN - (cursor - dst),
                                      "%x", ntohs(as_shorts[i]));
        if (i != 7 && runpos[i + 1] != 1) {
          *cursor++ = ':';
        }
      } else if (runpos[i] == 1) {
        // Entered the run; print the colons and skip the run.
        *cursor++ = ':';
        *cursor++ = ':';
        i += (max - 1);
      }
    }
  }
  return dst;
}
const char * socket_inet_ntop(int af, const void *src, char *dst, unsigned int size) {
    switch (af) {
    case AF_INET :
        return inet_ntop_v4 (src, dst, size);
	case AF_INET6 :
        return inet_ntop_v6 (src, dst, size);		
    default :
        //xerror("EAFNOSUPPORT");
        return NULL;
    }
}
#endif

//not support in windows
int socket_set_tcp_mss(SOCKET sockfd, int size) {
#ifdef _WIN32
    return 0;
#else
    return setsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &size, sizeof(size));
#endif
}

int socket_get_tcp_mss(SOCKET sockfd, int* size) {
#ifdef _WIN32
    return 0;
#else
    if (size == NULL)
        return -1;
    socklen_t len = sizeof(int);
    return getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, (void*)size, &len);
#endif
}

int socket_fix_tcp_mss(SOCKET sockfd) {
    return socket_set_tcp_mss(sockfd, 1400);
}

int socket_disable_nagle(SOCKET sock, int nagle) {
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&nagle, sizeof(nagle));
}


int socket_error(SOCKET sock) {
    int error = 0;
    socklen_t len = sizeof(error);
    if (0 != getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len))
    { error = socket_errno; }
    return error;
}

int socket_isnonetwork(int error) {
    if (error==SOCKET_ERRNO(ENETDOWN)) return 1;
    if (error==SOCKET_ERRNO(ENETUNREACH)) return 1;

    return 0;
}

int socket_reuseaddr(SOCKET sock, int optval) {
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval , sizeof(int));
}

int socket_get_nwrite(SOCKET _sock, int* _nwriteLen) {
#if defined(__APPLE__)
    socklen_t len = sizeof(int);
    return getsockopt(_sock, SOL_SOCKET, SO_NWRITE, _nwriteLen, &len);
#elif defined(ANDROID)
    return ioctl(_sock, SIOCOUTQ, _nwriteLen);
#else
    *_nwriteLen = -1;
    return 0;
#endif
}

int socket_get_nread(SOCKET _sock, int* _nreadLen) {
#if defined(__APPLE__)
    socklen_t len = sizeof(int);
    return getsockopt(_sock, SOL_SOCKET, SO_NREAD, _nreadLen, &len);
#elif defined(ANDROID)
    return ioctl(_sock, SIOCINQ, _nreadLen);
#else
    *_nreadLen = -1;
    return 0;
#endif

}

int socket_nwrite(SOCKET _sock) {
    int value = 0;
    int ret = socket_get_nwrite(_sock, &value);

    if (0==ret) return value;
    else return ret;
}

int socket_nread(SOCKET _sock) {
    int value = 0;
    int ret = socket_get_nread(_sock, &value);

    if (0==ret) return value;
    else return ret;
}

int socket_ipv6only(SOCKET _sock, int _only) {
    return  setsockopt(_sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&_only, sizeof(_only));
}
