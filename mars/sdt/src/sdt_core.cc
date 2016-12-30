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
 * netchecker_service.cc
 *
 *  Created on: 2014-6-17
 *      Author: renlibin caoshaokun
 */

#include <algorithm>

#include "boost/bind.hpp"

#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/singleton.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/sdt/constants.h"

#include "activecheck/dnschecker.h"
#include "activecheck/httpchecker.h"
#include "activecheck/pingchecker.h"
#include "activecheck/tcpchecker.h"
#include "sdt_core.h"

using namespace mars::sdt;

#define RETURN_NETCHECKER_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, async_reg_.Get(), )

SdtCore::SdtCore()
    : thread_(boost::bind(&SdtCore::__RunOn, this))
    , check_list_(std::list<BaseChecker*>())
    , cancel_(false)
    , checking_(false) {
    xinfo_function();
}

SdtCore::~SdtCore() {
    xinfo_function();

    cancel_ = true;

    if (!thread_.isruning()) {
    	__Reset();
    } else {
        CancelAndWait();
    }
}

void SdtCore::StartCheck(CheckIPPorts& _longlink_items, CheckIPPorts& _shortlink_items, int _mode, int _timeout) {
    xinfo_function();
    ScopedLock lock(checking_mutex_);

    if (checking_) return;

    __InitCheckReq(_longlink_items, _shortlink_items, _mode, _timeout);

	if (thread_.isruning() || thread_.start() != 0)
		return;
}

void SdtCore::__InitCheckReq(CheckIPPorts& _longlink_items, CheckIPPorts& _shortlink_items, int _mode, int _timeout) {
	xverbose_function();
	checking_ = true;

	check_request_.Reset();
	check_request_.longlink_items.insert(_longlink_items.begin(), _longlink_items.end());
	check_request_.mode = _mode;
	check_request_.total_timeout = _timeout;

    if (MODE_BASIC(_mode)) {
        PingChecker* ping_checker = new PingChecker();
        check_list_.push_back(ping_checker);
        DnsChecker* dns_checker = new DnsChecker();
        check_list_.push_back(dns_checker);
    }

    if (MODE_SHORT(_mode)) {
    	check_request_.shortlink_items.insert(_shortlink_items.begin(), _shortlink_items.end());
        HttpChecker* http_checker = new HttpChecker();
        check_list_.push_back(http_checker);
    }

    if (MODE_LONG(_mode)) {
        TcpChecker* tcp_checker = new TcpChecker();
        check_list_.push_back(tcp_checker);
    }
}

void SdtCore::__Reset() {
    xinfo_function();

    //check_request_.report

    std::list<BaseChecker*>::iterator iter = check_list_.begin();

    for (; iter != check_list_.end();) {
        if (NULL != (*iter)) {
            delete(*iter);
            (*iter) = NULL;
        }

        iter = check_list_.erase(iter);
    }

    checking_ = false;
}

void SdtCore::__RunOn() {
    xinfo_function();

    for (std::list<BaseChecker*>::iterator iter = check_list_.begin(); iter != check_list_.end(); ++iter) {
        if (cancel_ || check_request_.check_status == kCheckFinish)
            break;

        (*iter)->StartDoCheck(check_request_);
    }

    xinfo2(TSF"all checkers end! cancel_=%_, check_request_.check_status_=%_, check_list__size=%_", cancel_, check_request_.check_status, check_list_.size());

    __DumpCheckResult();
    __Reset();

}

void SdtCore::__DumpCheckResult() {
    
    std::vector<CheckResultProfile>::iterator iter = check_request_.checkresult_profiles.begin();
    for (; iter != check_request_.checkresult_profiles.end(); ++iter) {
        switch(iter->netcheck_type) {
        case kTcpCheck:
        	xinfo2(TSF"tcp check result, error_code:%_, ip:%_, port:%_, network_type:%_, rtt:%_", iter->error_code, iter->ip, iter->port, iter->network_type, iter->rtt);
        	break;
        case kHttpCheck:
        	xinfo2(TSF"http check result, status_code:%_, url:%_, ip:%_, port:%_, network_type:%_, rtt:%_", iter->status_code, iter->url, iter->ip, iter->port, iter->network_type, iter->rtt);
        	break;
        case kPingCheck:
        	xinfo2(TSF"ping check result, error_code:%_, ip:%_, network_type:%_, loss_rate:%_, rtt:%_", iter->error_code, iter->ip, iter->network_type, iter->loss_rate, iter->rtt_str);
        	break;
        case kDnsCheck:
        	xinfo2(TSF"dns check result, error_code:%_, domain_name:%_, network_type:%_, ip1:%_, rtt:%_", iter->error_code, iter->domain_name, iter->network_type, iter->ip1, iter->rtt);
        	break;
        }
    }
    ReportNetCheckResult(check_request_.checkresult_profiles);
}

void SdtCore::CancelCheck() {
    xinfo_function();
    cancel_ = true;
}

void SdtCore::CancelAndWait() {
    xinfo_function();
    thread_.join();
}
