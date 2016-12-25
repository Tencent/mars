/*
 * config.h
 *
 *  Created on: 2016年3月18日
 *      Author: caoshaokun
 */

#ifndef SDT_INTERFACE_CONSTANTS_H_
#define SDT_INTERFACE_CONSTANTS_H_

#define NET_CHECK_BASIC 1
#define NET_CHECK_LONG (1 << 1)
#define NET_CHECK_SHORT (1 << 2)

// Error sequence.
#define ERR_SEQ (-1)

#define MODE_BASIC(mode) ((mode) & NET_CHECK_BASIC)
#define MODE_LONG(mode) ((mode) & NET_CHECK_LONG)
#define MODE_SHORT(mode) ((mode) & NET_CHECK_SHORT)

// For default host.
#define DEFAULT_HTTP_HOST "www.qq.com"
#define DEFAULT_PING_HOST "www.qq.com"
#define DUMMY_HOST "DUMMY HOST"

// For report builder.
#define NET_CHECK_TAG "NET_CHECK"

#define CHECK_SUC "check success"
#define CHECK_FAIL "check failed"

// For net type report.
#define NoNetType "NoNet"
#define WifiType "Wifi Net"
#define MobileType "Mobile Net"
#define OtherNetType "Other Net"

// For HTTP
#define HTTP_DEFAULT_TIMEOUT        (5*1000)   // 5000ms
#define HTTP_DUMMY_RECV_DATA_SIZE   (1)
// For PING
#define DEFAULT_PING_TIMEOUT        (4)        // 4s
#define DEFAULT_PING_COUNT          (2)        // times
#define DEFAULT_PING_INTERVAL       (1)        // 1s
// For TCP
#define DEFAULT_TCP_CONN_TIMEOUT 5000  // ms
#define DEFAULT_TCP_RECV_TIMEOUT    (5*1000)   // 5000ms
// For DNS
#define DEFAULT_DNS_TIMEOUT         (3*1000)   // 3000ms
// For net check timeout
#define UNUSE_TIMEOUT               (INT_MAX)        // ms

// For HTTP User agent.
#ifdef ANDROID
#define USER_AGENT   "Mozilla/5.0  (Linux; Android 4.1.1; Nexus 7 Build/JRO03S) AppleWebKit/535.19 (KHTML,  like Gecko) Chrome/18.0.1025.166 Safari/535.19"
#elif defined(__APPLE__)
#define USER_AGENT   "Mozilla/5.0  (iPhone; CPU iPhone OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML,  like Gecko) Version/6.0 Mobile/10A403 Safari/8536.25"
#else
#define USER_AGENT   "Mozilla/5.0 (compatible; MSIE9.0 Windows NT 6.1; WOW64; Trident/5.0)"
#endif

#endif /* SDT_INTERFACE_CONSTANTS_H_ */
