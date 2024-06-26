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

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "boost/function.hpp"
#include "mars/baseevent/active_logic.h"
#include "mars/boot/context.h"
#include "mars/comm/dns/dns.h"
#include "mars/comm/thread/mutex.h"
#include "mars/stn/config.h"
#include "simple_ipport_sort.h"
#include "weak_network_logic.h"

namespace mars {
namespace stn {

struct IPPortItem;

class NetSource {
 public:
    class DnsUtil {
     public:
        DnsUtil(boot::Context* _context);
        ~DnsUtil();

     public:
        comm::DNS& GetNewDNS() {
            return new_dns_;
        }
        comm::DNS& GetDNS() {
            return dns_;
        }

        void Cancel(const std::string& host = "");

     private:
        DnsUtil(const DnsUtil&);
        DnsUtil& operator=(const DnsUtil&);

     private:
        std::vector<std::string> __OnNewDns(const std::string& _host, bool _longlink_host, const std::map<std::string, std::string>& _extra_info);

     private:
        boot::Context* context_;
        comm::DNS new_dns_;
        comm::DNS dns_;
        bool already_release_ = false;
    };  // end DnsUtil

 public:
    boost::function<bool()> fun_need_use_IPv6_;

 public:
    // set longlink host and ports
    void SetLongLink(const std::vector<std::string>& _hosts,
                     const std::vector<uint16_t>& _ports,
                     const std::string& _debugip);
    // set shortlink port
    void SetShortlink(const uint16_t _port, const std::string& _debugip);
    // set backup ips for host, these ips would be used when host dns failed
    void SetBackupIPs(const std::string& _host, const std::vector<std::string>& _ips);
    // set debug ip
    void SetDebugIP(const std::string& _host, const std::string& _ip);
    const std::string& GetLongLinkDebugIP();
    const std::string& GetShortLinkDebugIP();

    // set minorlong debugip
    void SetMinorLongDebugIP(const std::string& _ip, const uint16_t _port);
    const std::string& GetMinorLongLinkDebugIP();

    void SetLowPriorityLonglinkPorts(const std::vector<uint16_t>& _lowpriority_longlink_ports);

    void GetLonglinkPorts(std::vector<uint16_t>& _ports);
    const std::vector<std::string>& GetLongLinkHosts();
    uint16_t GetShortLinkPort();

    void GetBackupIPs(std::string _host, std::vector<std::string>& _iplist);

    std::string DumpTable(const std::vector<IPPortItem>& _ipport_items);

    void SetCgiDebugIP(const std::string& _cgi, const std::string& _ip, const uint16_t _port);
    bool CanUseQUIC();
    void DisableQUIC(int64_t seconds = 20 * 60);  // 20 minutes
    void ForbidQUIC(bool forbid);

    unsigned GetQUICRWTimeoutMs(const std::string& _cgi, TimeoutSource* outsource);
    void SetQUICRWTimeoutMs(const std::string& _cgi, unsigned ms);
    void SetDefaultQUICRWTimeoutMs(unsigned ms);

    unsigned GetQUICConnectTimeoutMs(const std::string& _cgi, TimeoutSource* outsource);
    void SetQUICConnectTimeoutMs(const std::string& _cgi, unsigned ms);
    void SetDefaultQUICConnectTimeoutMs(unsigned ms);

    void DisableIPv6();
    bool CanUseIPv6();
    
 public:
    NetSource(comm::ActiveLogic& _active_logic, boot::Context* _context);
    ~NetSource();

 public:
    // for long link
    bool GetLongLinkItems(const struct LonglinkConfig& _config,
                          DnsUtil& _dns_util,
                          std::vector<IPPortItem>& _ipport_items,
                          const std::map<std::string, std::string>& _extra_info);

    // for short link
    bool GetShortLinkItems(const std::vector<std::string>& _hostlist,
                           std::vector<IPPortItem>& _ipport_items,
                           DnsUtil& _dns_util,
                           const std::string& _cgi,
                           const std::map<std::string, std::string>& _extra_info);

    void AddServerBan(const std::string& _ip);

    void ClearCache();

    void ReportLongIP(bool _is_success, const std::string& _ip, uint16_t _port);
    void ReportShortIP(bool _is_success, const std::string& _ip, const std::string& _host, uint16_t _port);

    void RemoveLongBanIP(const std::string& _ip);

    bool GetLongLinkSpeedTestIPs(std::vector<IPPortItem>& _ip_vec);
    void ReportLongLinkSpeedTestResult(std::vector<IPPortItem>& _ip_vec);
    void InitHistory2BannedList(bool _save);
    void SetIpConnectTimeout(uint32_t _v4_timeout, uint32_t _v6_timeout);
    std::tuple<uint32_t, uint32_t> GetIpConnectTimeout() {
        return std::make_tuple(v4_timeout_, v6_timeout_);
    }

 public:
    WeakNetworkLogic* GetWeakNetworkLogic();

 private:
    bool __HasShortLinkDebugIP(const std::vector<std::string>& _hostlist);

    bool __GetLonglinkDebugIPPort(const struct LonglinkConfig& _config, std::vector<IPPortItem>& _ipport_items);
    bool __GetShortlinkDebugIPPort(const std::vector<std::string>& _hostlist,
                                   std::vector<IPPortItem>& _ipport_items,
                                   const std::string& _cgi);

    void __GetIPPortItems(std::vector<IPPortItem>& _ipport_items,
                          const std::vector<std::string>& _hostlist,
                          DnsUtil& _dns_util,
                          bool _islonglink,
                          const std::map<std::string, std::string>& _extra_info);
    size_t __MakeIPPorts(std::vector<IPPortItem>& _ip_items,
                         const std::string& _host,
                         size_t _count,
                         DnsUtil& _dns_util,
                         bool _isbackup,
                         bool _islonglink,
                         const std::map<std::string, std::string>& _extra_info);

 private:
    boot::Context* context_;
    comm::ActiveLogic& active_logic_;
    SimpleIPPortSort ipportstrategy_;
    uint32_t v4_timeout_;
    uint32_t v6_timeout_;
    WeakNetworkLogic* weak_network_logic_;

    // move from netsource.cc
 private:
    // mmnet ipport settings
    std::vector<std::string> sg_longlink_hosts;
    std::vector<uint16_t> sg_longlink_ports;
    std::string sg_longlink_debugip;

    std::string sg_minorlong_debugip;
    uint16_t sg_minorlong_port;

    int sg_shortlink_port;
    std::string sg_shortlink_debugip;
    std::map<std::string, std::vector<std::string>> sg_host_backupips_mapping;
    std::vector<uint16_t> sg_lowpriority_longlink_ports;

    std::map<std::string, std::string> sg_host_debugip_mapping;

    std::map<std::string, std::pair<std::string, uint16_t>> sg_cgi_debug_mapping;

    // quic
    tickcount_t sg_quic_reopen_tick = tickcount_t(true);
    bool sg_quic_enabled = true;

    
    TimeoutSource sg_quic_default_timeout_source = TimeoutSource::kClientDefault;
    unsigned sg_quic_default_rw_timeoutms = 5000;
    std::map<std::string, unsigned> sg_cgi_quic_rw_timeoutms_mapping;

    TimeoutSource quic_default_connect_timeout_source_ = TimeoutSource::kClientDefault;
    unsigned quic_default_conn_timeoutms_ = 250;
    std::map<std::string, unsigned> cgi_quic_connect_timeoutms_mapping_;
    bool quic_forbidden_ = true;

    // ipv6
    bool sg_ipv6_enabled = true;

    comm::Mutex sg_ip_mutex;
};

}  // namespace stn
}  // namespace mars

#endif  // STN_SRC_NETSOURCE_H_
