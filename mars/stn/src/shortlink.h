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
 * shortlink.h
 *
 *  Created on: 2012-8-22
 *      Author: zhouzhijie
 */

#ifndef STN_SRC_SHORTLINK_H_
#define STN_SRC_SHORTLINK_H_

#include <string>
#include <map>
#include <vector>

#include "boost/signals2.hpp"
#include "boost/function.hpp"

#include "mars/comm/thread/thread.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/http.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"

#include "net_source.h"
#include "shortlink_interface.h"

namespace mars {
namespace stn {
    
class shortlink_tracker;
    
class ShortLink : public ShortLinkInterface {
  public:
    ShortLink(MessageQueue::MessageQueue_t _messagequeueid, NetSource& _netsource, const Task& _task, bool _use_proxy);
    virtual ~ShortLink();

    ConnectProfile   Profile() const { return conn_profile_;}

  protected:
    virtual void 	 SendRequest(AutoBuffer& _buffer_req, AutoBuffer& _task_extend);

    virtual void     __Run();
    virtual SOCKET   __RunConnect(ConnectProfile& _conn_profile);
    virtual void     __RunReadWrite(SOCKET _sock, int& _errtype, int& _errcode, ConnectProfile& _conn_profile);
    void             __CancelAndWaitWorkerThread();

    void			 __UpdateProfile(const ConnectProfile& _conn_profile);

    void 			 __RunResponseError(ErrCmdType _type, int _errcode, ConnectProfile& _conn_profile, bool _report = true);
    void 			 __OnResponse(ErrCmdType _err_type, int _status, AutoBuffer& _body, AutoBuffer& _extension, ConnectProfile& _conn_profile, bool _report = true);
    
  protected:
    MessageQueue::ScopeRegister     asyncreg_;
    NetSource&                      net_source_;
    Task                            task_;
    Thread                          thread_;

    SocketBreaker                   breaker_;
    ConnectProfile                  conn_profile_;
    NetSource::DnsUtil              dns_util_;
    const bool                      use_proxy_;
    AutoBuffer                      send_body_;
    AutoBuffer                      send_extend_;
    
    boost::scoped_ptr<shortlink_tracker> tracker_;
};
        
}}

#endif // STN_SRC_MMSHORTLINK_H_
