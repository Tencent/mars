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
 * sdt.h
 *
 *  Created on: 2016年3月18日
 *      Author: caoshaokun
 */

#ifndef SDT_INTERFACE_SDT_H_
#define SDT_INTERFACE_SDT_H_

#include <stdlib.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

namespace mars {
namespace sdt {

struct CheckResultProfile;

struct CheckIPPort {

	CheckIPPort() {
		Reset();
	}

	CheckIPPort(std::string _ip, uint16_t _port)
	: ip(_ip), port(_port){}

	bool operator < (const CheckIPPort& _rhs) const {
		return ip < _rhs.ip;
	}

	CheckIPPort& operator = (const CheckIPPort& _rhs)  {
		ip = _rhs.ip;
		port = _rhs.port;
		return (*this);
	}

	void Reset() {
		ip.clear();
		port = 0;
	}

	std::string ip;
	uint16_t port;

};

typedef std::map< std::string, std::vector<CheckIPPort> > CheckIPPorts;
typedef std::map< std::string, std::vector<CheckIPPort> >::iterator CheckIPPorts_Iterator;

enum NetCheckStatus {
	kNone = 0,
	kChecking = 1,
	kCheckEnd,
};

enum NetCheckType {
	kPingCheck = 0,
	kDnsCheck = 1,
	kNewDnsCheck,
	kTcpCheck,
	kHttpCheck,
	kTracerouteCheck,
	kReqBufCheck,
};

enum CheckErrCode {
	kCheckOK = 0,
	kDns = 1,
	kSocket = 2,
	kIsRunning,
};

enum TcpErrCode {
	kTcpSucc      = 0,
	kTcpNonErr    = 1,
	kSelectErr    = -1,
	kPipeIntr     = -2,
	kSndRcvErr    = -3,
	kAssertErr    = -4,
	kTimeoutErr   = -5,
	kSelectExpErr = -6,
	kPipeExp      = -7,
	kConnectErr   = -8,
	kTcpRespErr	  = -9
};

enum CheckStatus {
	kCheckContinue = 0,
	kCheckFinish = 1,
} ;

extern void ReportNetCheckResult(std::vector<CheckResultProfile>& _check_results);

}}


#endif /* SDT_INTERFACE_SDT_H_ */
