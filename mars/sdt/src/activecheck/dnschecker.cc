// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  dnschecker.cc
//  netchecker
//
//  Author: renlibin caoshaokun on 24/6/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//

#include "dnschecker.h"

#include "mars/comm/singleton.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/sdt/constants.h"

#include "checkimpl/dnsquery.h"

using namespace mars::sdt;

DnsChecker::DnsChecker() {
    xverbose_function();
}

DnsChecker::~DnsChecker() {
    xverbose_function();
}

int DnsChecker::StartDoCheck(CheckRequestProfile& _check_request) {
    xinfo_function();
    return BaseChecker::StartDoCheck(_check_request);
}

int DnsChecker::CancelDoCheck() {
    xinfo_function();
    return BaseChecker::CancelDoCheck();
}

void DnsChecker::__DoCheck(CheckRequestProfile& _check_request) {
    xinfo_function();

    //lonlgink host dns
    for (CheckIPPorts_Iterator iter = _check_request.longlink_items.begin(); iter != _check_request.longlink_items.end(); ++iter) {
		CheckResultProfile profile;
		profile.domain_name = iter->first;
		profile.netcheck_type = kDnsCheck;
		profile.network_type = ::getNetInfo();

		struct socket_ipinfo_t ipinfo;
		int timeout = (_check_request.total_timeout == UNUSE_TIMEOUT ? DEFAULT_DNS_TIMEOUT : _check_request.total_timeout);
        uint64_t start_time = gettickcount();
        int ret = socket_gethostbyname(profile.domain_name.c_str(), &ipinfo, timeout, NULL);
        uint64_t cost_time = gettickcount() - start_time;

        profile.error_code = ret;
        profile.rtt = cost_time;

        if (0 == ret) {
			xinfo2(TSF"%0, check dns, host: %1, ret: %2", NET_CHECK_TAG, profile.domain_name, CHECK_SUC);
			if (ipinfo.size >= 2){
				profile.ip1 = inet_ntoa(ipinfo.ip[0]);
				profile.ip2 = inet_ntoa(ipinfo.ip[1]);
			}else if (1 == ipinfo.size){
				profile.ip1 = inet_ntoa(ipinfo.ip[0]);
			}else{
				xerror2(TSF"ret = 0, but ipinfo.size = %d", ipinfo.size);
			}
		} else {
			xinfo2(TSF"%0, check dns, host: %1, ret: %2", NET_CHECK_TAG, profile.domain_name, CHECK_FAIL);
		}

        _check_request.checkresult_profiles.push_back(profile);
        _check_request.check_status = (ret >= 0 ? kCheckContinue : kCheckFinish);

		if (_check_request.total_timeout != UNUSE_TIMEOUT) {
			_check_request.total_timeout -= cost_time;
			if (_check_request.total_timeout <= 0) {
				xinfo2(TSF"dns check, host: %0, timeout.", profile.domain_name);
				break;
			}
		}
	}

    //shortlink host dns
    for (CheckIPPorts_Iterator iter = _check_request.shortlink_items.begin(); iter != _check_request.shortlink_items.end(); ++iter) {
		CheckResultProfile profile;
		profile.domain_name = iter->first;
		profile.netcheck_type = kDnsCheck;
		profile.network_type = ::getNetInfo();

		struct socket_ipinfo_t ipinfo;
		int timeout = (_check_request.total_timeout == UNUSE_TIMEOUT ? DEFAULT_DNS_TIMEOUT : _check_request.total_timeout);
        uint64_t start_time = gettickcount();
        int ret = socket_gethostbyname(profile.domain_name.c_str(), &ipinfo, timeout, NULL);
        uint64_t cost_time = gettickcount() - start_time;

        profile.error_code = ret;
        profile.rtt = cost_time;

        if (0 == ret) {
			xinfo2(TSF"%0, check dns, host: %1, ret: %2", NET_CHECK_TAG, profile.domain_name, CHECK_SUC);
			if (ipinfo.size >= 2){
				profile.ip1 = inet_ntoa(ipinfo.ip[0]);
				profile.ip2 = inet_ntoa(ipinfo.ip[1]);
			}else if (1 == ipinfo.size){
				profile.ip1 = inet_ntoa(ipinfo.ip[0]);
			}else{
				xerror2(TSF"ret = 0, but ipinfo.size = %d", ipinfo.size);
			}
		} else {
			xinfo2(TSF"%0, check dns, host: %1, ret: %2", NET_CHECK_TAG, profile.domain_name, CHECK_FAIL);
		}

        _check_request.checkresult_profiles.push_back(profile);
        _check_request.check_status = (ret >= 0 ? kCheckContinue : kCheckFinish);

		if (_check_request.total_timeout != UNUSE_TIMEOUT) {
			_check_request.total_timeout -= cost_time;
			if (_check_request.total_timeout <= 0) {
				xinfo2(TSF"dns check, host: %0, timeout.", profile.domain_name);
				break;
			}
		}
    }
}

