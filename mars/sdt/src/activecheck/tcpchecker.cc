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
//  tcpchecker.cc
//  netchecker
//
//  Author: renlibin caoshaokun on 24/7/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//
#include "tcpchecker.h"

#include "mars/stn/stn_logic.h"

#include "mars/comm/singleton.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/stn/proto/longlink_packer.h"
#include "mars/sdt/constants.h"

#include "sdt/src/checkimpl/tcpquery.h"
#include "sdt/src/tools/netchecker_socketutils.hpp"

using namespace mars::sdt;
using namespace mars::stn;

TcpChecker::TcpChecker() {
    xverbose_function();
}

TcpChecker::~TcpChecker() {
    xverbose_function();
}

int TcpChecker::StartDoCheck(CheckRequestProfile& _check_request) {
    xinfo_function();
    return BaseChecker::StartDoCheck(_check_request);
}

int TcpChecker::CancelDoCheck() {
    xinfo_function();
    return BaseChecker::CancelDoCheck();
}

void TcpChecker::__DoCheck(CheckRequestProfile& _check_request) {
    xinfo_function();

    for (CheckIPPorts_Iterator iter = _check_request.longlink_items.begin(); iter != _check_request.longlink_items.end(); ++iter) {
    	std::string host = iter->first;
    	for (std::vector<CheckIPPort>::iterator ipport = iter->second.begin(); ipport != iter->second.end(); ++ipport) {
    		CheckResultProfile profile;
			profile.netcheck_type = kTcpCheck;
    		profile.ip = (*ipport).ip;
    		profile.port = (*ipport).port;
			profile.network_type = ::getNetInfo();

    		unsigned int timeout = UNUSE_TIMEOUT == _check_request.total_timeout ? DEFAULT_TCP_CONN_TIMEOUT : _check_request.total_timeout;
			xinfo2(TSF"tcp check ip: %0, port: %1, timeout: %2", profile.ip, profile.port, timeout);

    		uint64_t start_time = ::gettickcount();
    		TcpQuery tcp_query(profile.ip.c_str(), profile.port, 0);

            AutoBuffer noop_send;
            __NoopReq(noop_send);

            int ret = tcp_query.tcp_send((const unsigned char *)noop_send.Ptr(), (int)noop_send.Length(), timeout);

			if (ret < 0) {
				profile.error_code = kSndRcvErr;
				xerror2(TSF"tcp send nooping data error.");
			} else {
				xinfo2(TSF"tcp check send nooping data success.");
			}

			uint64_t cost_time = 0;
			if (ret >= 0) {
				AutoBuffer recv_buff;
				recv_buff.AllocWrite(64 * 1024, false);

				ret = tcp_query.tcp_receive(recv_buff, 64*1024, timeout);
				cost_time = ::gettickcount() - start_time;

				if (ret < 0) {
					profile.error_code = kSndRcvErr;
					xerror2(TSF"tcp recv nooping data error.");
					_check_request.checkresult_profiles.push_back(profile);
					continue;
				} else {
					uint32_t cmdid = 0, seq = 0; size_t packlen = 0; AutoBuffer recv_body;
					profile.rtt = cost_time;
					if (!__NoopResp(recv_buff, cmdid, seq, packlen, recv_body)) {	//not noop resp
						profile.error_code = kTcpRespErr;
					}
				}
			}

			_check_request.checkresult_profiles.push_back(profile);
			_check_request.check_status = (profile.error_code == 0) ? kCheckContinue : kCheckFinish;

			if (_check_request.total_timeout != UNUSE_TIMEOUT) {
				_check_request.total_timeout -= cost_time;
				if (_check_request.total_timeout <= 0) {
					xinfo2(TSF"tcp check, host: %0, timeout.", host);
					break;
				}
			}

    	}
    }
}

void TcpChecker::__NoopReq(AutoBuffer& _noop_send) {
	AutoBuffer noop_body;
	AutoBuffer noop_extension;
	longlink_noop_req_body(noop_body, noop_extension);
	longlink_pack(longlink_noop_cmdid(), Task::kNoopTaskID, noop_body, noop_extension, _noop_send, NULL);
}

bool TcpChecker::__NoopResp(const AutoBuffer& _packed, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, AutoBuffer& _body) {
    AutoBuffer extension;
	int unpackret = longlink_unpack(_packed, _cmdid, _seq, _package_len, _body, extension, NULL);
	if (unpackret == LONGLINK_UNPACK_OK) {
        if (longlink_noop_isresp(Task::kNoopTaskID, _cmdid, _seq, _body, extension)) {
			longlink_noop_resp_body(_body, extension);
			return true;
		}
	}

	return false;
}
