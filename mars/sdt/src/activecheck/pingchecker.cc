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
//  pingchecker.cc
//  netchecker
//
//  Author: renlibin caoshaokun on 25/6/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//

#include "pingchecker.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/singleton.h"
#include "mars/comm/time_utils.h"
#include "mars/sdt/constants.h"

#include "sdt/src/checkimpl/pingquery.h"

using namespace mars::sdt;

PingChecker::PingChecker() {
    xverbose_function();
}

PingChecker::~PingChecker() {
    xverbose_function();
}

int PingChecker::StartDoCheck(CheckRequestProfile& _check_request) {
#if defined(ANDROID) || defined(__APPLE__)
    xinfo_function();
    return BaseChecker::StartDoCheck(_check_request);
#else
    xinfo2(TSF"neither android nor ios");
    return -1;
#endif
}

int PingChecker::CancelDoCheck() {
    xinfo_function();
    return BaseChecker::CancelDoCheck();
}

void PingChecker::__DoCheck(CheckRequestProfile& _check_request) {

#if defined(ANDROID) || defined(__APPLE__)
    xinfo_function();

    // longlink ip ping
    for (CheckIPPorts_Iterator iter = _check_request.longlink_items.begin(); iter != _check_request.longlink_items.end(); ++iter) {
		for (std::vector<CheckIPPort>::iterator ipport = iter->second.begin(); ipport != iter->second.end(); ++ipport) {
			CheckResultProfile profile;
			std::string host = (*ipport).ip.empty() ? DEFAULT_PING_HOST : (*ipport).ip;
			profile.ip = host;
			profile.netcheck_type = kPingCheck;
			profile.network_type = ::getNetInfo();

			uint64_t start_time = gettickcount();
			PingQuery ping_query;
			int ret = ping_query.RunPingQuery(0, 0, (UNUSE_TIMEOUT == _check_request.total_timeout ? 0 : _check_request.total_timeout / 1000), host.c_str());
			uint64_t cost_time = gettickcount() - start_time;

			profile.error_code = ret;
			profile.checkcount = DEFAULT_PING_COUNT;

			struct PingStatus ping_status;  // = {0};  //can not define pingStatus in if(0==ret),because we need pingStatus.ip
			char loss_rate[16] = {0};
			char avgrtt[16] = {0};

			if (0 == ret) {
				ping_query.GetPingStatus(ping_status);
				const float EPSINON = 0.00001;

				if ((ping_status.loss_rate - 1.0) >= -EPSINON && (ping_status.loss_rate - 1.0) <= EPSINON) {
					xinfo2(TSF"ping check, host: %_ failed.", host);
				} else {
					xinfo2(TSF"ping check, host: %_ success.", host);
				}

				snprintf(loss_rate, 16, "%f", ping_status.loss_rate);
				snprintf(avgrtt, 16, "%f", ping_status.avgrtt);

				profile.loss_rate = loss_rate;
				profile.rtt_str = avgrtt;
			}

			_check_request.checkresult_profiles.push_back(profile);
			_check_request.check_status = (profile.error_code == 0) ? kCheckContinue : kCheckFinish;

			if (_check_request.total_timeout != UNUSE_TIMEOUT) {
				_check_request.total_timeout -= cost_time;
				if (_check_request.total_timeout <= 0) {
					xinfo2(TSF"ping check, host: %_, timeout.", host);
					break;
				}
			}
		}
    }

    // shortlink ip ping
    for (CheckIPPorts_Iterator iter = _check_request.shortlink_items.begin(); iter != _check_request.shortlink_items.end(); ++iter) {
		for (std::vector<CheckIPPort>::iterator ipport = iter->second.begin(); ipport != iter->second.end(); ++ipport) {
			CheckResultProfile profile;
			std::string host = (*ipport).ip.empty() ? DEFAULT_PING_HOST : (*ipport).ip;
			profile.ip = host;
			profile.netcheck_type = kPingCheck;

			uint64_t start_time = gettickcount();
			PingQuery ping_query;
			int ret = ping_query.RunPingQuery(0, 0, (UNUSE_TIMEOUT == _check_request.total_timeout ? 0 : _check_request.total_timeout / 1000), host.c_str());
			uint64_t cost_time = gettickcount() - start_time;

			profile.error_code = ret;
			profile.checkcount = DEFAULT_PING_COUNT;

			struct PingStatus ping_status;  // = {0};  //can not define pingStatus in if(0==ret),because we need pingStatus.ip
			char loss_rate[16] = {0};
			char avgrtt[16] = {0};

			if (0 == ret) {
				ping_query.GetPingStatus(ping_status);
				const float EPSINON = 0.00001;

				if ((ping_status.loss_rate - 1.0) >= -EPSINON && (ping_status.loss_rate - 1.0) <= EPSINON) {
					xinfo2(TSF"ping check, host: %_ failed.", host);
				} else {
					xinfo2(TSF"ping check, host: %_ success.", host);
				}

				snprintf(loss_rate, 16, "%f", ping_status.loss_rate);
				snprintf(avgrtt, 16, "%f", ping_status.avgrtt);

				profile.loss_rate = loss_rate;
				profile.rtt_str = avgrtt;
			}

			_check_request.checkresult_profiles.push_back(profile);
			_check_request.check_status = (profile.error_code == 0) ? kCheckContinue : kCheckFinish;

			if (_check_request.total_timeout != UNUSE_TIMEOUT) {
				_check_request.total_timeout -= cost_time;
				if (_check_request.total_timeout <= 0) {
					xinfo2(TSF"ping check, host: %_, timeout.", host);
					break;
				}
			}

		}
	}

#endif
}


