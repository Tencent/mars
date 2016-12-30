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
 * shortlink_interface.h
 *
 *  Created on: Jul 21, 2016
 *      Author: wutianqiang
 */

#ifndef SRC_SHORTLINK_INTERFACE_H_
#define SRC_SHORTLINK_INTERFACE_H_

#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

class ShortLinkInterface {
  public:
	virtual ~ShortLinkInterface(){};

	virtual void            SendRequest(AutoBuffer& _buf_req) = 0;
	virtual ConnectProfile  Profile() const { return ConnectProfile();}

    boost::function<void (int _line, ErrCmdType _errtype, int _errcode, const std::string& _ip, const std::string& _host, uint16_t _port)> func_network_report;
    boost::function<void (ShortLinkInterface* _worker, ErrCmdType _err_type, int _status, AutoBuffer& _body, bool _cancel_retry, ConnectProfile& _conn_profile)> OnResponse;
    boost::function<void (ShortLinkInterface* _worker)> OnSend;
	boost::function<void (ShortLinkInterface* _worker, unsigned int _cached_size, unsigned int _total_size)> OnRecv;
};
    
}
}

#endif /* SRC_SHORTLINK_INTERFACE_H_ */
