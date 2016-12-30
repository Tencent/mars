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
 * netinfo_util.cc
 *
 *  Created on: Dec 19, 2016
 *      Author: wutianqiang
 */
#include <vector>

#include "comm/xlogger/xlogger.h"
#include "comm/socket/local_ipstack.h"
#include "comm/platform_comm.h"
#include "comm/network/getgateway.h"
#include "comm/network/getdnssvraddrs.h"
#include "comm/network/getifaddrs.h"
#include "comm/socket/unix_socket.h"
#include "comm/socket/socket_address.h"

std::string GetDetailNetInfo() {
	XMessage detail_net_info;
	//1.网络信息
	 switch (::getNetInfo()) {
	    case kNoNet:
	    	detail_net_info << "current network:no network, ipstack:" << TLocalIPStackStr[local_ipstack_detect()] << "\n";
	        break;

	    case kWifi: {
	        WifiInfo info;
	        getCurWifiInfo(info);
	        detail_net_info << "current network:wifi, ssid:" << info.ssid << ",ipstack:" << TLocalIPStackStr[local_ipstack_detect()] << "\n";
	    }
	    break;

	    case kMobile: {
	        SIMInfo info;
	        getCurSIMInfo(info);
	        RadioAccessNetworkInfo raninfo;
	        getCurRadioAccessNetworkInfo(raninfo);
	        detail_net_info << "current network:mobile, ispname:" << info.isp_name << ", info.isp_code" << info.isp_code;
	        detail_net_info	<< ", ran:" << raninfo.radio_access_network << ",ipstack:" << TLocalIPStackStr[local_ipstack_detect()] << "\n";
	    }
	    break;

	    case kOtherNet:
	    	detail_net_info << "current network:other, ipstack:" << TLocalIPStackStr[local_ipstack_detect()] << "\n";
	        break;

	    default:
	        xassert2(false);
	        break;
	    }
	 detail_net_info << "--------NetConfig Info----------"<< "\n";
	 //2.网络配置信息（默认网关、dns svr、路由表）
	 	in6_addr addr6_gateway;
	 	memset(&addr6_gateway, 0, sizeof(addr6_gateway));
	    if (0 == getdefaultgateway6(&addr6_gateway)) {
	    	detail_net_info << "getdefaultgateway6:" << socket_address(addr6_gateway).ipv6() << "\n";
	    } else {
	    	detail_net_info << "getdefaultgateway6:" << "failed. ";
	    }
	    in_addr addr_gateway;
	    memset(&addr_gateway, 0, sizeof(addr_gateway));
	    if (0 == getdefaultgateway(&addr_gateway)) {
	    	detail_net_info << "getdefaultgateway:" << socket_address(addr_gateway).ip() << "\n";
	    } else {
	    	detail_net_info << "getdefaultgateway:" << "failed. ";
	    }

	    std::vector<socket_address> dnssvraddrs;
	    getdnssvraddrs(dnssvraddrs);
	    if (!dnssvraddrs.empty()) {
			for (size_t i = 0; i < dnssvraddrs.size(); ++i) {
				if (AF_INET == dnssvraddrs[i].address().sa_family) {
					detail_net_info << "dns server" << i << ":AF_INET, " << dnssvraddrs[i].ip() << "\n";
				}
				if (AF_INET6 == dnssvraddrs[i].address().sa_family) {
					detail_net_info << "dns server" << i << ":AF_INET6, " << dnssvraddrs[i].ipv6() << "\n";
				}
			}
	    } else {
	    	detail_net_info << "dns server: empty." << "\n";
	    }
	    //TODO 路由表信息

	 //3.网卡信息
	    detail_net_info << "----------NIC Info-----------" << "\n";
	    std::vector<ifaddrinfo_ip_t> v4_addrs;
	    if (getifaddrs_ipv4_filter(v4_addrs, 0)) {
	    	for (size_t i = 0; i < v4_addrs.size(); ++i) {
	    		detail_net_info << "interface name:"<<v4_addrs[i].ifa_name << ", " << (v4_addrs[i].ifa_family==AF_INET?"AF_INET":"XX_INET")
	    				<< ", ip:" << v4_addrs[i].ip << "\n";
	    	}
	    } else {
	    	detail_net_info << "getifaddrs_ipv4_filter:false" << "\n";
	    }
	    std::vector<ifaddrinfo_ip_t> v6_addrs;
	    if (getifaddrs_ipv6_filter(v6_addrs, 0)) {
	    	for (size_t i = 0; i < v6_addrs.size(); ++i) {
	    		detail_net_info << "interface name:"<<v6_addrs[i].ifa_name << ", " << (v6_addrs[i].ifa_family==AF_INET6?"AF_INET6":"XX_INET")
	    		    	<< ", ip:" << v6_addrs[i].ip << "\n";
	    	}
	    } else {
	    	detail_net_info << "getifaddrs_ipv6_filter:false" << "\n";
	    }
	return detail_net_info.Message();
}
