// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/* netsource.cc
 *
 *  Created on: 2012-7-17
 *      Author: yerungui
 */

#include "net_source.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <set>

#include "boost/bind.hpp"

#include "mars/comm/marcotoolkit.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/strutil.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/platform_comm.h"
#include "mars/stn/stn.h"
#include "mars/stn/dns_profile.h"
#include "mars/stn/config.h"

using namespace mars::stn;
using namespace mars::comm;

static const char* const kItemDelimiter = ":";

static const int kNumMakeCount = 5;

//mmnet ipport settings
static std::vector<std::string> sg_longlink_hosts;
static std::vector<uint16_t> sg_longlink_ports;
static std::string sg_longlink_debugip;

static std::string sg_minorlong_debugip;
static uint16_t sg_minorlong_port = 0;

static int sg_shortlink_port;
static std::string sg_shortlink_debugip;
static std::map< std::string, std::vector<std::string> > sg_host_backupips_mapping;
static std::vector<uint16_t> sg_lowpriority_longlink_ports;

static std::map< std::string, std::string > sg_host_debugip_mapping;

static std::map<std::string, std::pair<std::string, uint16_t>> sg_cgi_debug_mapping;
static tickcount_t sg_quic_reopen_tick(true);
static bool sg_quic_enabled = true;

static Mutex sg_ip_mutex;

NetSource::DnsUtil::DnsUtil():
new_dns_(OnNewDns) {
}

NetSource::DnsUtil::~DnsUtil() {}

void NetSource::DnsUtil::Cancel(const std::string& host) {
    if (host.empty()) {
        new_dns_.Cancel();
        dns_.Cancel();
    }
    else {
        new_dns_.Cancel(host);
        dns_.Cancel(host);
    }
}

NetSource::NetSource(ActiveLogic& _active_logic)
	: active_logic_(_active_logic)
    , v4_timeout_(0)
    , v6_timeout_(0)
{
    xinfo_function();
}

NetSource::~NetSource() {
    xinfo_function();
}

/**
 *	host ip port setting from java
 */
void NetSource::SetLongLink(const std::vector<std::string>& _hosts, const std::vector<uint16_t>& _ports, const std::string& _debugip) {
	ScopedLock lock(sg_ip_mutex);

	xgroup2_define(addr_print);
	xinfo2(TSF"task set longlink server addr, ") >> addr_print;
	for (std::vector<std::string>::const_iterator host_iter = _hosts.begin(); host_iter != _hosts.end(); ++host_iter) {
		xinfo2(TSF "host:%_ ", *host_iter) >> addr_print;
	}
	for (std::vector<uint16_t>::const_iterator port_iter = _ports.begin(); port_iter != _ports.end(); ++port_iter) {
		xinfo2(TSF "port:%_ ", *port_iter) >> addr_print;
	}
	xinfo2(TSF"debugip:%_", _debugip) >> addr_print;

    sg_longlink_debugip = _debugip;
    if (!_hosts.empty()) {
    	sg_longlink_hosts = _hosts;
    }
    else {
    	xerror2(TSF"host list should not be empty");
    }
	sg_longlink_ports = _ports;
}

void NetSource::SetShortlink(const uint16_t _port, const std::string& _debugip) {
	ScopedLock lock(sg_ip_mutex);

	xinfo2(TSF "task set shortlink server addr, port:%_, debugip:%_", _port, _debugip);

	sg_shortlink_port = _port;
    sg_shortlink_debugip = _debugip;
}

void NetSource::SetMinorLongDebugIP(const std::string& _ip, const uint16_t _port) {
    ScopedLock lock(sg_ip_mutex);
    xinfo2(TSF "task set minorlong server addr, port:%_, debugip:%_", _port, _ip);
    sg_minorlong_debugip = _ip;
    sg_minorlong_port = _port;
}

void NetSource::SetBackupIPs(const std::string& _host, const std::vector<std::string>& _ips) {
	ScopedLock lock(sg_ip_mutex);

	xgroup2_define(addr_print);
	xinfo2(TSF"task set backup server addr, host:%_", _host) >> addr_print;
	for (std::vector<std::string>::const_iterator ip_iter = _ips.begin(); ip_iter != _ips.end(); ++ip_iter) {
		xinfo2(TSF "ip:%_ ", *ip_iter) >> addr_print;
	}

	sg_host_backupips_mapping[_host] = _ips;
}

void NetSource::SetDebugIP(const std::string& _host, const std::string& _ip) {
	ScopedLock lock(sg_ip_mutex);

	xinfo2(TSF "task set debugip:%_ for host:%_", _ip, _host);
    
    if (_ip.empty()){
        sg_host_debugip_mapping.erase(_host);
    }else{
        sg_host_debugip_mapping[_host] = _ip;
    }
}

const std::string& NetSource::GetLongLinkDebugIP() {
	ScopedLock lock(sg_ip_mutex);

	return sg_longlink_debugip;
}

const std::string& NetSource::GetShortLinkDebugIP() {
    ScopedLock lock(sg_ip_mutex);
    return sg_shortlink_debugip;
}

const std::string& NetSource::GetMinorLongLinkDebugIP() {
    ScopedLock lock(sg_ip_mutex);
    return sg_minorlong_debugip;
}

void NetSource::SetLowPriorityLonglinkPorts(const std::vector<uint16_t>& _lowpriority_longlink_ports) {
    sg_lowpriority_longlink_ports = _lowpriority_longlink_ports;
}

/**
 *
 * longlink functions
 *
 */
const std::vector<std::string>& NetSource::GetLongLinkHosts() {
    ScopedLock lock(sg_ip_mutex);
	return sg_longlink_hosts;
}

void NetSource::GetLonglinkPorts(std::vector<uint16_t>& _ports) {
    ScopedLock lock(sg_ip_mutex);
	_ports = sg_longlink_ports;
}

bool NetSource::GetLongLinkItems(const struct LonglinkConfig& _config, DnsUtil& _dns_util, std::vector<IPPortItem>& _ipport_items) {
    xinfo_function();
    ScopedLock lock(sg_ip_mutex);

    if (__GetLonglinkDebugIPPort(_config, _ipport_items)) {
        return true;
    }
    
    lock.unlock();

    std::vector<std::string> longlink_hosts = _config.host_list;
    if(longlink_hosts.empty())
        longlink_hosts = NetSource::GetLongLinkHosts();
 	if (longlink_hosts.empty()) {
 		xerror2("longlink host empty.");
 		return false;
 	}

 	__GetIPPortItems(_ipport_items, longlink_hosts, _dns_util, true);

	return !_ipport_items.empty();
}

bool NetSource::__GetLonglinkDebugIPPort(const struct LonglinkConfig& _config, std::vector<IPPortItem>& _ipport_items) {

	for (std::vector<std::string>::iterator ip_iter = sg_longlink_hosts.begin(); ip_iter != sg_longlink_hosts.end(); ++ip_iter) {
		if (sg_host_debugip_mapping.find(*ip_iter) != sg_host_debugip_mapping.end()) {
			for (std::vector<uint16_t>::iterator iter = sg_longlink_ports.begin(); iter != sg_longlink_ports.end(); ++iter) {
				IPPortItem item;
				item.str_ip = (*sg_host_debugip_mapping.find(*ip_iter)).second;
				item.str_host = *ip_iter;
				item.port = *iter;
				item.source_type = kIPSourceDebug;
				_ipport_items.push_back(item);
			}
			return true;
		}
	}

    if ((_config.link_type == Task::kChannelLong && !sg_longlink_debugip.empty()) ||
        (_config.link_type == Task::kChannelMinorLong && !sg_minorlong_debugip.empty())) {
        for (std::vector<uint16_t>::iterator iter = sg_longlink_ports.begin(); iter != sg_longlink_ports.end(); ++iter) {
            IPPortItem item;
            
            if (_config.link_type == Task::kChannelLong){
                item.str_ip = sg_longlink_debugip;
                item.str_host = sg_longlink_hosts.front();
            }else{
                item.str_ip = sg_minorlong_debugip;
                item.str_host = _config.host_list.front();
            }
            item.port = *iter;
            item.source_type = kIPSourceDebug;
            _ipport_items.push_back(item);
        }
        return true;
    }
    
	return false;
}

void NetSource::GetBackupIPs(std::string _host, std::vector<std::string>& _iplist) {
	ScopedLock lock(sg_ip_mutex);
	if (sg_host_backupips_mapping.find(_host) != sg_host_backupips_mapping.end()) {
		_iplist = (*sg_host_backupips_mapping.find(_host)).second;
	}
}

void NetSource::ReportLongIP(bool _is_success, const std::string& _ip, uint16_t _port) {
    xinfo2_if(!_is_success, TSF"_is_success=%0, ip=%1, port=%2", _is_success, _ip, _port);

    if (_ip.empty() || 0 == _port) return;

    if (kNoNet == getNetInfo()) return;

    ipportstrategy_.Update(_ip, _port, _is_success);
}

void NetSource::RemoveLongBanIP(const std::string& _ip) {
    ipportstrategy_.RemoveBannedList(_ip);
}

/**
 *
 * shortlink functions
 *
 */
uint16_t NetSource::GetShortLinkPort() {
	ScopedLock lock(sg_ip_mutex);
	return sg_shortlink_port;
}

bool NetSource::__HasShortLinkDebugIP(const std::vector<std::string>& _hostlist) {
	if (!sg_shortlink_debugip.empty()) {
		return true;
	}

	for (std::vector<std::string>::const_iterator host = _hostlist.begin(); host != _hostlist.end(); ++host) {
		if (sg_host_debugip_mapping.find(*host) != sg_host_debugip_mapping.end()) {
			return true;
		}
	}

	return false;
}

bool NetSource::GetShortLinkItems(const std::vector<std::string>& _hostlist, std::vector<IPPortItem>& _ipport_items, DnsUtil& _dns_util, const std::string& _cgi) {
	
    ScopedLock lock(sg_ip_mutex);
    
	if (__GetShortlinkDebugIPPort(_hostlist, _ipport_items, _cgi)) {
		return true;
    }
    
    lock.unlock();

    if (_hostlist.empty()) return false;
    __GetIPPortItems(_ipport_items, _hostlist, _dns_util, false);

	return !_ipport_items.empty();
}

bool NetSource::__GetShortlinkDebugIPPort(const std::vector<std::string>& _hostlist, std::vector<IPPortItem>& _ipport_items, const std::string& _cgi) {

    if (!_cgi.empty()) {
        std::map<std::string,std::pair<std::string, uint16_t>>::iterator itr = sg_cgi_debug_mapping.find(_cgi);
        if (itr != sg_cgi_debug_mapping.end()) {
            IPPortItem item;
            item.str_ip = itr->second.first;
            item.str_host = _hostlist.front();
            item.port = itr->second.second;
            item.source_type = kIPSourceDebug;
            _ipport_items.push_back(item);
            return true;
        }
    }
    
	for (std::vector<std::string>::const_iterator host = _hostlist.begin(); host != _hostlist.end(); ++host) {
		if (sg_host_debugip_mapping.find(*host) != sg_host_debugip_mapping.end()) {
			IPPortItem item;
			item.str_ip = (*sg_host_debugip_mapping.find(*host)).second;
			item.str_host = *host;
			item.port = sg_shortlink_port;
			item.source_type = kIPSourceDebug;
			_ipport_items.push_back(item);
			return true;
		}
	}
    
    if (!sg_shortlink_debugip.empty()) {
        IPPortItem item;
        item.str_ip = sg_shortlink_debugip;
        item.str_host = _hostlist.front();
        item.port = sg_shortlink_port;
        item.source_type = kIPSourceDebug;
        _ipport_items.push_back(item);
    }

	return !_ipport_items.empty();
}

void NetSource::__GetIPPortItems(std::vector<IPPortItem>& _ipport_items, const std::vector<std::string>& _hostlist, DnsUtil& _dns_util, bool _islonglink) {
	if (active_logic_.IsActive()) {
		unsigned int merge_type_count = 0;
		unsigned int makelist_count = kNumMakeCount;

		for (std::vector<std::string>::const_iterator iter = _hostlist.begin(); iter != _hostlist.end(); ++iter) {
			if (merge_type_count == 1 && _ipport_items.size() == kNumMakeCount) makelist_count = kNumMakeCount + 1;

			if (0 < __MakeIPPorts(_ipport_items, *iter, makelist_count, _dns_util, /*_isbackup=*/false, _islonglink)) merge_type_count++;
		}

		for (std::vector<std::string>::const_iterator iter = _hostlist.begin(); iter != _hostlist.end(); ++iter) {
			if (merge_type_count == 1 && _ipport_items.size() == kNumMakeCount) makelist_count = kNumMakeCount + 1;

			if (0 < __MakeIPPorts(_ipport_items, *iter, makelist_count, _dns_util, /*_isbackup=*/true, _islonglink)) merge_type_count++;
		}
	}
	else {
		size_t host_count = _hostlist.size();
		size_t ret = (kNumMakeCount - 1) / host_count;
		size_t ret2 = (kNumMakeCount - 1) % host_count;
		size_t i = 0;
		size_t count = 0;

		for (std::vector<std::string>::const_iterator host_iter = _hostlist.begin(); host_iter != _hostlist.end() && count < kNumMakeCount - 1; ++host_iter) {
			count += i < ret2 ? ret + 1 : ret;
			__MakeIPPorts(_ipport_items, *host_iter, count, _dns_util, /*_isbackup=*/false, _islonglink);
			i++;
		}

		for (std::vector<std::string>::const_iterator host_iter = _hostlist.begin(); host_iter != _hostlist.end() && count < kNumMakeCount; ++host_iter) {
			__MakeIPPorts(_ipport_items, *host_iter, kNumMakeCount, _dns_util, /*_isbackup=*/true, _islonglink);
		}
	}
}

size_t NetSource::__MakeIPPorts(std::vector<IPPortItem>& _ip_items, const std::string& _host, size_t _count, DnsUtil& _dns_util, bool _isbackup, bool _islonglink) {

	IPSourceType ist = kIPSourceNULL;
	std::vector<std::string> iplist;
    std::vector<uint16_t> ports;

	if (!_isbackup) {
		DnsProfile dns_profile;
		dns_profile.host = _host;

		bool ret = _dns_util.GetNewDNS().GetHostByName(_host, iplist, 2 * 1000, NULL, _islonglink);

		dns_profile.end_time = gettickcount();
		if (!ret) dns_profile.OnFailed();
		ReportDnsProfile(dns_profile);

		xgroup2_define(dnsxlog);
		xdebug2(TSF"link host:%_, new dns ret:%_, size:%_ ", _host, ret, iplist.size()) >> dnsxlog;

		if (iplist.empty()) {
			dns_profile.Reset();
			dns_profile.dnstype = kType_Dns;

			ist = kIPSourceDNS;
			ret = _dns_util.GetDNS().GetHostByName(_host, iplist);

			dns_profile.end_time = gettickcount();
			if (!ret) dns_profile.OnFailed();
			ReportDnsProfile(dns_profile);

			xdebug2(TSF "dns ret:%_, size:%_,", ret, iplist.size()) >> dnsxlog;
		}
		else {
			ist = kIPSourceNewDns;
		}

		if (_islonglink) {
			NetSource::GetLonglinkPorts(ports);
		}
		else {
			ports.push_back(NetSource::GetShortLinkPort());
		}
	}
	else {
		NetSource::GetBackupIPs(_host, iplist);
		xdebug2(TSF"link host:%_, backup ips size:%_", _host, iplist.size());
        
        if (iplist.empty() && _dns_util.GetDNS().GetHostByName(_host, iplist)) {
            ScopedLock lock(sg_ip_mutex);
            sg_host_backupips_mapping[_host] = iplist;
        }
        
		if (_islonglink) {
            if (sg_lowpriority_longlink_ports.empty()) {
                NetSource::GetLonglinkPorts(ports);
            } else {
                ports = sg_lowpriority_longlink_ports;
            }
		}
		else {
			ports.push_back(NetSource::GetShortLinkPort());
		}
		ist = kIPSourceBackup;
		if (!iplist.empty() && !ports.empty())
		{
			std::set<std::string> setIps;
			for (auto it = _ip_items.begin(); it != _ip_items.end(); ++it)
			{
				setIps.insert(it->str_ip);
			}
			size_t ports_cnt = ports.size();
			size_t require_cnt = _count - _ip_items.size();
			if (require_cnt < ports_cnt) require_cnt += ports_cnt;
			size_t cur_cnt = iplist.size() * ports_cnt;
			size_t i = 0;
			while (cur_cnt > require_cnt && i < iplist.size())
			{
				if (setIps.find(iplist[i]) != setIps.end())
				{
					iplist.erase(iplist.begin() + i);
					cur_cnt -= ports_cnt;
				}
				else
				{
					i++;
				}
			}
		}
	}

	if (iplist.empty()) return 0;

	size_t len = _ip_items.size();

	std::vector<IPPortItem> temp_items;
	for (std::vector<std::string>::iterator ip_iter = iplist.begin(); ip_iter != iplist.end(); ++ip_iter) {
		for (std::vector<uint16_t>::iterator port_iter = ports.begin(); port_iter != ports.end(); ++port_iter) {
			IPPortItem item;
			item.str_ip = *ip_iter;
			item.source_type = ist;
			item.str_host = _host;
			item.port = *port_iter;
			temp_items.push_back(item);
		}
	}

	if (!_isbackup) {
		ipportstrategy_.SortandFilter(temp_items, (int)(_count - len), true);
		_ip_items.insert(_ip_items.end(), temp_items.begin(), temp_items.end());
	}
	else {
		_ip_items.insert(_ip_items.end(), temp_items.begin(), temp_items.end());
		srand((unsigned)gettickcount());
		std::random_shuffle(_ip_items.begin() + len, _ip_items.end());
		_ip_items.resize(std::min(_ip_items.size(), (size_t)_count));
	}

	return _ip_items.size();
}

void NetSource::ReportShortIP(bool _is_success, const std::string& _ip, const std::string& _host, uint16_t _port) {
    xinfo2_if(!_is_success, TSF"_is_success=%0, ip=%1, port=%2 host=%3", _is_success, _ip, _port, _host);

    if (_ip.empty()) return;

    if (kNoNet == getNetInfo()) return;

    ipportstrategy_.Update(_ip, _port, _is_success);
}

void NetSource::ClearCache() {
    xinfo_function();
    ipportstrategy_.InitHistory2BannedList(true);
    
    ScopedLock lock(sg_ip_mutex);
    sg_quic_enabled = true;
}

void NetSource::DisableQUIC(int64_t seconds/* = 20 * 60*/){
    ScopedLock lock(sg_ip_mutex);
	sg_quic_enabled = false;
    sg_quic_reopen_tick.gettickcount();
    sg_quic_reopen_tick += seconds * 1000;
}

bool NetSource::CanUseQUIC(){
    ScopedLock lock(sg_ip_mutex);
    if (sg_quic_enabled)
        return true;
    
    if (sg_quic_reopen_tick.gettickspan() >= 0){
        sg_quic_enabled = true;
    }
    
    return sg_quic_enabled;
}

std::string NetSource::DumpTable(const std::vector<IPPortItem>& _ipport_items) {
    XMessage stream;

    for (unsigned int i = 0; i < _ipport_items.size(); ++i) {
        stream << _ipport_items[i].str_ip << kItemDelimiter << _ipport_items[i].port << kItemDelimiter << _ipport_items[i].str_host
               << kItemDelimiter << IPSourceTypeString[_ipport_items[i].source_type];

        if (i != _ipport_items.size() - 1) {
            stream << "|";
        }
    }

    return stream.String();
}

void NetSource::SetCgiDebugIP(const std::string &_cgi, const std::string &_ip, const uint16_t _port) {
    if (_cgi.empty()) {
        xinfo2(TSF"cgi is empty. ignore");
        return;
    }
    if (_ip.empty()) {
        xinfo2(TSF"ip is empty. remove cgi %_ debug ip", _cgi);
        std::map<std::string, std::pair<std::string, uint16_t>>::iterator it = sg_cgi_debug_mapping.find(_cgi);
        if( it != sg_cgi_debug_mapping.end()) {
            sg_cgi_debug_mapping.erase(it);
        }
        return;
    }
    xinfo2(TSF "set debug ip:%_ for cgi :%_", _ip, _cgi);
    uint64_t port = 80;
    if (_port > 0 ){
        port = _port;
    }
    sg_cgi_debug_mapping[_cgi] = std::pair<std::string, uint16_t>(_ip,port);
}

bool NetSource::GetLongLinkSpeedTestIPs(std::vector<IPPortItem>& _ip_vec) {
    xverbose_function();

    return true;
}
void NetSource::InitHistory2BannedList(bool _save) {
    ipportstrategy_.InitHistory2BannedList(_save);
}

void NetSource::ReportLongLinkSpeedTestResult(std::vector<IPPortItem>& _ip_vec) {
}

void NetSource::AddServerBan(const std::string& _ip) {
    ipportstrategy_.AddServerBan(_ip);
}

void NetSource::SetIpConnectTimeout(uint32_t _v4_timeout, uint32_t _v6_timeout) {
    v4_timeout_ = _v4_timeout;
    v6_timeout_ = _v6_timeout;
}
