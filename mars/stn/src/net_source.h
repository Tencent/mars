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
 * netsource.h
 *
 *  Created on: 2012-7-17
 *      Author: yerungui
 */

#ifndef STN_SRC_NETSOURCE_H_
#define STN_SRC_NETSOURCE_H_

#include <vector>
#include <string>
#include <map>
#include <tuple>

#include "boost/function.hpp"

#include "mars/baseevent/active_logic.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/dns/dns.h"
#include "mars/stn/config.h"

#include "simple_ipport_sort.h"

namespace mars {
    namespace stn {

struct IPPortItem;

class NetSource {
  public:
    class DnsUtil {
    public:
    	DnsUtil();
        ~DnsUtil();
        
    public:
        comm::DNS& GetNewDNS() {	return new_dns_;}
        comm::DNS& GetDNS() {	return dns_;}

        void Cancel(const std::string& host = "");
        
    private:
        DnsUtil(const DnsUtil&);
        DnsUtil& operator=(const DnsUtil&);
        
    private:
        comm::DNS new_dns_;
        comm::DNS dns_;
    };

  public:
    boost::function<bool ()> fun_need_use_IPv6_;

  public:
    //set longlink host and ports
    static void SetLongLink(const std::vector<std::string>& _hosts, const std::vector<uint16_t>& _ports, const std::string& _debugip);
    //set shortlink port
    static void SetShortlink(const uint16_t _port, const std::string& _debugip);
    //set backup ips for host, these ips would be used when host dns failed
    static void SetBackupIPs(const std::string& _host, const std::vector<std::string>& _ips);
    //set debug ip
    static void SetDebugIP(const std::string& _host, const std::string& _ip);
    static const std::string& GetLongLinkDebugIP();
    static const std::string& GetShortLinkDebugIP();
    
    // set minorlong debugip
    static void SetMinorLongDebugIP(const std::string& _ip, const uint16_t _port);
    static const std::string& GetMinorLongLinkDebugIP();
    
    static void SetLowPriorityLonglinkPorts(const std::vector<uint16_t>& _lowpriority_longlink_ports);

    static void GetLonglinkPorts(std::vector<uint16_t>& _ports);
    static const std::vector<std::string>& GetLongLinkHosts();
    static uint16_t GetShortLinkPort();
    
    static void GetBackupIPs(std::string _host, std::vector<std::string>& _iplist);

    static std::string DumpTable(const std::vector<IPPortItem>& _ipport_items);
    
    static void SetCgiDebugIP(const std::string& _cgi, const std::string& _ip, const uint16_t _port);
	static bool CanUseQUIC();
    static void DisableQUIC(int64_t seconds = 20 * 60); // 20 minutes
    
    static unsigned GetQUICRWTimeoutMs(const std::string& _cgi, TimeoutSource* outsource);
    static void SetQUICRWTimeoutMs(const std::string& _cgi, unsigned ms);
    static void SetDefaultQUICRWTimeoutMs(unsigned ms);
    
  public:
    NetSource(comm::ActiveLogic& _active_logic);
    ~NetSource();

  public:
    // for long link
    bool GetLongLinkItems(const struct LonglinkConfig& _config, DnsUtil& _dns_util, std::vector<IPPortItem>& _ipport_items);

    // for short link
    bool GetShortLinkItems(const std::vector<std::string>& _hostlist, std::vector<IPPortItem>& _ipport_items, DnsUtil& _dns_util, const std::string& _cgi);

    void AddServerBan(const std::string& _ip);
    
    void ClearCache();

    void ReportLongIP(bool _is_success, const std::string& _ip, uint16_t _port);
    void ReportShortIP(bool _is_success, const std::string& _ip, const std::string& _host, uint16_t _port);

    void RemoveLongBanIP(const std::string& _ip);

    bool GetLongLinkSpeedTestIPs(std::vector<IPPortItem>& _ip_vec);
    void ReportLongLinkSpeedTestResult(std::vector<IPPortItem>& _ip_vec);
    void InitHistory2BannedList(bool _save);
    void SetIpConnectTimeout(uint32_t _v4_timeout, uint32_t _v6_timeout);
    std::tuple<uint32_t, uint32_t> GetIpConnectTimeout() {return std::make_tuple(v4_timeout_, v6_timeout_);}

  private:
    
    bool __HasShortLinkDebugIP(const std::vector<std::string>& _hostlist);
    
    bool __GetLonglinkDebugIPPort(const struct LonglinkConfig& _config, std::vector<IPPortItem>& _ipport_items);
    bool __GetShortlinkDebugIPPort(const std::vector<std::string>& _hostlist, std::vector<IPPortItem>& _ipport_items, const std::string& _cgi);

    void __GetIPPortItems(std::vector<IPPortItem>& _ipport_items, const std::vector<std::string>& _hostlist, DnsUtil& _dns_util, bool _islonglink);
    size_t __MakeIPPorts(std::vector<IPPortItem>& _ip_items, const std::string& _host, size_t _count, DnsUtil& _dns_util, bool _isbackup, bool _islonglink);

  private:
    comm::ActiveLogic&  active_logic_;
    SimpleIPPortSort    ipportstrategy_;
    uint32_t v4_timeout_;
    uint32_t v6_timeout_;
};
        
    }
}


#endif // STN_SRC_NETSOURCE_H_
