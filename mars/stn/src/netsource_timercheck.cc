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
 * netsource_timercheck.cc
 *
 *  Created on: 2013-5-16
 *      Author: yanguoyue
 */

#include "netsource_timercheck.h"

#include <unistd.h>

#include "boost/bind.hpp"

#include "mars/comm/comm_frequency_limit.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/config.h"
#include "mars/stn/stn.h"

#include "longlink.h"
#include "longlink_speed_test.h"

using namespace mars::stn;

static const unsigned int kTimeCheckPeriod = 2.5 * 60 * 1000;     // 2.5min
// const static unsigned int TIME_CHECK_PERIOD = 30 * 1000;     //30min
static const int kTimeout = 10*1000;     // s
static const int kMaxSpeedTestCount = 30;
static const unsigned long kIntervalTime = 1 * 60 * 60 * 1000;    // ms

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_NETCORE_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

NetSourceTimerCheck::NetSourceTimerCheck(NetSource* _net_source, ActiveLogic& _active_logic, LongLink& _longlink, MessageQueue::MessageQueue_t  _messagequeue_id)
    : net_source_(_net_source)
    , seletor_(breaker_)
    , longlink_(_longlink)
	, asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeue_id)){
    xassert2(breaker_.IsCreateSuc(), "create breaker fail");

    frequency_limit_ = new CommFrequencyLimit(kMaxSpeedTestCount, kIntervalTime);

    active_connection_ = _active_logic.SignalActive.connect(boost::bind(&NetSourceTimerCheck::__OnActiveChanged, this, _1));

    if (_active_logic.IsActive()) {
    	__StartCheck();
    }
}

NetSourceTimerCheck::~NetSourceTimerCheck() {
    
    do {
        if (!thread_.isruning()) {
            break;
        }
        
        if (!breaker_.Break()) {
            xerror2(TSF"write into pipe error");
            break;
        }
    
        thread_.join();
    } while (false);
    
    delete frequency_limit_;
}

void NetSourceTimerCheck::CancelConnect() {
	RETURN_NETCORE_SYNC2ASYNC_FUNC(boost::bind(&NetSourceTimerCheck::CancelConnect, this));
    xdebug_function();

    if (!thread_.isruning()) {
        return;
    }

    if (!breaker_.Break()) {
        xerror2(TSF"write into pipe error");
    }

}

void NetSourceTimerCheck::__StartCheck() {

	RETURN_NETCORE_SYNC2ASYNC_FUNC(boost::bind(&NetSourceTimerCheck::__StartCheck, this));
    xdebug_function();

    if (asyncpost_ != MessageQueue::KNullPost) return;

    asyncpost_ = MessageQueue::AsyncInvokePeriod(kTimeCheckPeriod, kTimeCheckPeriod, boost::bind(&NetSourceTimerCheck::__Check, this), asyncreg_.Get());

}

void NetSourceTimerCheck::__Check() {

    IPSourceType pre_iptype = longlink_.Profile().ip_type;
    if (kIPSourceDebug == pre_iptype || kIPSourceNULL == pre_iptype
    		|| kIPSourceNewDns == pre_iptype || kIPSourceDNS == pre_iptype) {
    	return;
    }

    if (thread_.isruning()) {
        return;
    }

    // limit the frequency of speed test
    if (!frequency_limit_->Check()) {
        xwarn2(TSF"frequency limit");
        return;
    }

    if (!breaker_.IsCreateSuc() && !breaker_.ReCreate()) {
        xassert2(false, "break error!");
        return;
    }

    std::string linkedhost = longlink_.Profile().host;
    xdebug2(TSF"current host:%0", linkedhost);

    thread_.start(boost::bind(&NetSourceTimerCheck::__Run, this, linkedhost));

}

void NetSourceTimerCheck::__StopCheck() {

	RETURN_NETCORE_SYNC2ASYNC_FUNC(boost::bind(&NetSourceTimerCheck::__StopCheck, this));

    xdebug_function();

    if (asyncpost_ == MessageQueue::KNullPost) return;

    if (!thread_.isruning()) {
        return;
    }

    if (!breaker_.Break()) {
        xerror2(TSF"write into pipe error");
        return;
    }

    thread_.join();

    asyncreg_.Cancel();
    asyncpost_ = MessageQueue::KNullPost;
}

void NetSourceTimerCheck::__Run(const std::string& _host) {
    //clear the pipe
    breaker_.Clear();

	if (__TryConnnect(_host)) {

		xassert2(fun_time_check_suc_);

		if (fun_time_check_suc_) {
			// reset the long link
			fun_time_check_suc_();
		}

	}

}


bool NetSourceTimerCheck::__TryConnnect(const std::string& _host) {
    std::vector<std::string> ip_vec;

    dns_util_.GetNewDNS().GetHostByName(_host, ip_vec);

    if (ip_vec.empty()) dns_util_.GetDNS().GetHostByName(_host, ip_vec);
    if (ip_vec.empty()) return false;

    for (std::vector<std::string>::iterator iter = ip_vec.begin(); iter != ip_vec.end(); ++iter) {
    	if (*iter == longlink_.Profile().ip) {
    		return false;
    	}
    }

    std::vector<uint16_t> port_vec;
    NetSource::GetLonglinkPorts(port_vec);

    if (port_vec.empty()) {
        xerror2(TSF"get ports empty!");
        return false;
    }

    // random get speed test ip and port
    srand((unsigned)gettickcount());
    size_t ip_index = rand() % ip_vec.size();
    size_t port_index = rand() % port_vec.size();

    LongLinkSpeedTestItem speed_item(ip_vec[ip_index], port_vec[port_index]);

    while (true) {
        seletor_.PreSelect();
        speed_item.HandleSetFD(seletor_);

        int select_ret = seletor_.Select(kTimeout);

        if (select_ret == 0) {
            xerror2(TSF"time out");
            break;
        }

        if (select_ret < 0) {
            xerror2(TSF"select errror, ret:%0, strerror(errno):%1", select_ret, strerror(errno));
        }

        if (seletor_.IsException()) {
            xerror2(TSF"pipe exception");
            break;
        }

        if (seletor_.IsBreak()) {
            xwarn2(TSF"FD_ISSET(pipe_[0], &readfd)");
            break;
        }

        speed_item.HandleFDISSet(seletor_);

        if (kLongLinkSpeedTestSuc == speed_item.GetState() || kLongLinkSpeedTestFail == speed_item.GetState()) {
            break;
        }
    }

    speed_item.CloseSocket();



    if (kLongLinkSpeedTestSuc == speed_item.GetState()) {
        net_source_->RemoveLongBanIP(speed_item.GetIP());
        return true;
    }

    return false;
}

void NetSourceTimerCheck::__OnActiveChanged(bool _is_active) {
    xdebug2(TSF"_is_active:%0", _is_active);

    if (_is_active) {
        __StartCheck();
    } else {
    	__StopCheck();
    }
}
