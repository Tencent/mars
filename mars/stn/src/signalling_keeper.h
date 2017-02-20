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
//  signalling_keeper.h
//  network
//
//  Created by liucan on 13-12-24.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifndef STN_SRC_SIGNALLING_KEEPER_H_
#define STN_SRC_SIGNALLING_KEEPER_H_

#include "boost/function.hpp"

#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/socket/udpclient.h"

#include "longlink.h"

namespace mars {
    namespace stn {

class SignallingKeeper: IAsyncUdpClientEvent {
  public:
    static void SetStrategy(unsigned int  _period, unsigned int _keep_time);  // ms
  public:
    SignallingKeeper(const LongLink& _longlink, MessageQueue::MessageQueue_t _messagequeue_id, bool _use_UDP = true);
    ~SignallingKeeper();

    void OnNetWorkDataChanged(const char*, ssize_t, ssize_t);

    void Keep();
    void Stop();

    virtual void OnError(UdpClient* _this, int _errno);
    virtual void OnDataGramRead(UdpClient* _this, void* _buf, size_t _len);
    virtual void OnDataSent(UdpClient* _this);
  public:
    boost::function<unsigned int (const AutoBuffer&, const AutoBuffer&, int)> fun_send_signalling_buffer_;

  private:
    void __SendSignallingBuffer();
    void __OnTimeOut();

  private:
    MessageQueue::ScopeRegister msgreg_;
    uint64_t last_touch_time_;
    bool keeping_;
    MessageQueue::MessagePost_t postid_;
    const LongLink& longlink_;
    std::string ip_;
    unsigned int port_;
    UdpClient udp_client_;
    bool use_UDP_;
};
        
    }
}


#endif // STN_SRC_SIGNALLING_KEEPER_H_
