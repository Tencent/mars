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

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "boost/function.hpp"
#include "boost/signals2.hpp"
#include "mars/boot/context.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/http.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/thread/thread.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"
#include "net_source.h"
#include "shortlink_interface.h"
#include "socket_operator.h"

namespace mars {
namespace stn {

class shortlink_tracker;

class ShortLink : public ShortLinkInterface {
 public:
    ShortLink(boot::Context* _context,
              comm::MessageQueue::MessageQueue_t _messagequeueid,
              std::shared_ptr<NetSource> _netsource,
              const Task& _task,
              bool _use_proxy,
              std::unique_ptr<SocketOperator> _operator = nullptr);
    virtual ~ShortLink();

    ConnectProfile Profile() const {
        return conn_profile_;
    }

    void SetConnectParams(const std::vector<IPPortItem>& _out_addr, uint32_t v4timeout_ms, uint32_t v6timeout_ms);

 protected:
    virtual void SendRequest();
    virtual void SetSentCount(int count);
    virtual void SendRequest(AutoBuffer& _buffer_req, AutoBuffer& _task_extend);
    virtual bool IsKeepAlive() const {
        return is_keep_alive_;
    }

    virtual void __Run();
    virtual SOCKET __RunConnect(ConnectProfile& _conn_profile);
    virtual void __RunReadWrite(SOCKET _sock, int& _errtype, int& _errcode, ConnectProfile& _conn_profile);
    void __CancelAndWaitWorkerThread();
    void __CancelAndWaitReq2BufThread();

    void __UpdateProfile(const ConnectProfile _conn_profile);

    void __RunResponseError(ErrCmdType _type, int _errcode, ConnectProfile& _conn_profile, bool _report = true);
    void __OnResponse(ErrCmdType _err_type,
                      int _status,
                      AutoBuffer& _body,
                      AutoBuffer& _extension,
                      ConnectProfile& _conn_profile,
                      bool _report = true);
    void __OnResponseImp(ErrCmdType _errType,
                         int _status,
                         AutoBuffer& _body,
                         AutoBuffer& _extension,
                         ConnectProfile& _conn_profile);

    bool __AsyncCheckAuth();
    void __CancelAsyncCheckAuth();
    bool __WaitAsyncReq2buf();

 private:
    bool __ContainIPv6(const std::vector<socket_address>& _vecaddr);

 public:
    bool __Req2Buf();
    std::string __GetTheadName(const std::string& fullcgi);

 protected:
    boot::Context* context_;
    comm::MessageQueue::ScopeRegister asyncreg_;
    std::shared_ptr<NetSource> net_source_;
    std::unique_ptr<SocketOperator> socketOperator_;
    Task task_;
    comm::Thread thread_;
    std::shared_ptr<comm::Thread> req2buf_thread_;
    std::mutex req2buf_ready_mtx;
    std::condition_variable req2buf_ready_cv;
    std::atomic<bool> is_req2buf_ready{false};

    ConnectProfile conn_profile_;
    NetSource::DnsUtil dns_util_;
    const bool use_proxy_;
    AutoBuffer send_body_;
    AutoBuffer send_extend_;

    std::vector<IPPortItem> outter_vec_addr_;
    uint32_t v4connect_timeout_ms_ = 1000;
    uint32_t v6connect_timeout_ms_ = 1000;

    boost::scoped_ptr<shortlink_tracker> tracker_;
    bool is_keep_alive_;
    bool is_start_req2buf_thread;
    bool is_req2buf_result = false;
    int sent_count;
};

}  // namespace stn
}  // namespace mars

#endif  // STN_SRC_MMSHORTLINK_H_
