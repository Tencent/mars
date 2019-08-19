//
//  base_netinfo_query.cpp
//  sdt
//
//  Created by elviswu on 2017/7/6.
//  Copyright © 2017年 Tencent. All rights reserved.
//
#include "base_netinfo_query.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/network/getgateway.h"
#include "mars/comm/network/getdnssvraddrs.h"
#include "mars/comm/network/getifaddrs.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/socket/socket_address.h"


BaseNetInfo::BaseNetInfo()
: nettype_(kNoNet)
, ip_stack_(ELocalIPStack_None) {

}
BaseNetInfo::~BaseNetInfo() {
    
}
const BaseNetInfo& BaseNetInfo::Query() {
    __QueryNetType();
    __QueryIPStack();
    __QueryNetConfig();
    return *this;
}

void BaseNetInfo::__QueryNetType() {
    xinfo_function();
    nettype_ = ::getNetInfo();
    XMessage xmsg;
    switch (nettype_) {
        case kNoNet:
            break;
            
        case kWifi: {
            WifiInfo info;
            getCurWifiInfo(info);
            xmsg(TSF"wifi, ssid:%_, bssid:%_.", info.ssid, info.bssid);
        }
            break;
            
        case kMobile: {
            SIMInfo info;
            getCurSIMInfo(info);
            RadioAccessNetworkInfo raninfo;
            getCurRadioAccessNetworkInfo(raninfo);
            xmsg(TSF"mobile, ispname:%_, ispcode:%_, ran:%_", info.isp_name, info.isp_code, raninfo.radio_access_network);
        }
            break;
            
        case kOtherNet:
            break;
            
        default:
            xassert2(false, TSF"nettype_:%_", nettype_);
            break;
    }
    detail_nettype_info_.append(xmsg.String());
}
void BaseNetInfo::__QueryIPStack() {
    xinfo_function();
    ip_stack_ = local_ipstack_detect_log(ipstack_detect_info_);
}

void BaseNetInfo::__QueryNetConfig(){
    xinfo_function();
    //默认网关
    XMessage xmsg1;
    in6_addr addr6_gateway;
    memset(&addr6_gateway, 0, sizeof(addr6_gateway));
    if (0 == getdefaultgateway6(&addr6_gateway)) {
        xmsg1(TSF"getdefaultgateway6:%_.", socket_address(addr6_gateway).ipv6());
    } else {
        xmsg1(TSF"getdefaultgateway6:failed.");
    }
    in_addr addr_gateway;
    memset(&addr_gateway, 0, sizeof(addr_gateway));
    if (0 == getdefaultgateway(&addr_gateway)) {
        xmsg1(TSF"getdefaultgateway:%_.", socket_address(addr_gateway).ip());
    } else {
        xmsg1(TSF"getdefaultgateway:failed.");
    }
    
    net_gateway_info_ = xmsg1.String();
    
    //DNS SVR
    XMessage xmsg2;
    std::vector<socket_address> dnssvraddrs;
    mars::comm::getdnssvraddrs(dnssvraddrs);
    if (!dnssvraddrs.empty()) {
        for (size_t i = 0; i < dnssvraddrs.size(); ++i) {
            if (AF_INET == dnssvraddrs[i].address().sa_family) {
                xmsg2(TSF"dns server %_ :AF_INET, %_.", i, dnssvraddrs[i].ip());
            }
            if (AF_INET6 == dnssvraddrs[i].address().sa_family) {
                xmsg2(TSF"dns server %_ :AF_INET6, %_.", i, dnssvraddrs[i].ipv6());
            }
        }
    } else {
        xmsg2(TSF"dns server: empty.");
    }
    
    net_dnssvr_info_ = xmsg2.String();
    
    //网卡信息
    XMessage xmsg3;
    std::vector<ifaddrinfo_ip_t> v4_addrs;
    if (getifaddrs_ipv4_filter(v4_addrs, 0)) {
        for (size_t i = 0; i < v4_addrs.size(); ++i) {
            xmsg3(TSF"interface name:%_, %_(%_), ip:%_", v4_addrs[i].ifa_name, (v4_addrs[i].ifa_family==AF_INET?"AF_INET":"XX_INET"), v4_addrs[i].ifa_family, v4_addrs[i].ip);
        }
    } else {
        xmsg3(TSF"getifaddrs_ipv4_filter:false");
    }
    std::vector<ifaddrinfo_ip_t> v6_addrs;
    if (getifaddrs_ipv6_filter(v6_addrs, 0)) {
        for (size_t i = 0; i < v6_addrs.size(); ++i) {
            xmsg3(TSF"interface name:%_, %_(%_), ip:%_", v6_addrs[i].ifa_name, (v6_addrs[i].ifa_family==AF_INET6?"AF_INET6":"XX_INET"), v6_addrs[i].ifa_family, v6_addrs[i].ip);
        }
    } else {
        xmsg3(TSF"getifaddrs_ipv6_filter:false");
    }
    net_interface_info_ = xmsg3.String();
    //路由表信息
    //TODO

}


std::string BaseNetInfo::ToString() const {
    XMessage xmsg;
    xmsg(TSF"nettype:%_,(%_);", nettype_, detail_nettype_info_);
    xmsg(TSF"ip_stack:%_,(%_);", ip_stack_, ipstack_detect_info_);
    xmsg(TSF"net_gateway_info:%_", net_gateway_info_);
    xmsg(TSF"net_dnssvr_info:%_", net_dnssvr_info_);
    xmsg(TSF"net_interface_info:%_", net_interface_info_);
    xmsg(TSF"route_info:%_", route_info_);
    return xmsg.String();
}
