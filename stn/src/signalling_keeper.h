//
//  signalling_keeper.h
//  network
//
//  Created by 刘粲 on 13-12-24.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifndef STN_SRC_SIGNALLING_KEEPER_H_
#define STN_SRC_SIGNALLING_KEEPER_H_

#include "boost/function.hpp"

#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/messagequeue/message_queue_utils.h"
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
    boost::function<unsigned int (const unsigned char* _buf, int _len, int anCmdID)> fun_send_signalling_buffer_;

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
