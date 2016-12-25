/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * netsource_timercheck.h
 *
 *  Created on: 2013-5-16
 *      Author: yanguoyue
 */

#ifndef STN_SRC_NETSOURCE_TIMERCHECK_H_
#define STN_SRC_NETSOURCE_TIMERCHECK_H_

#include "boost/signals2.hpp"

#include "mars/comm/thread/thread.h"
#include "mars/baseevent/active_logic.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/messagequeue/message_queue_utils.h"
#include "mars/comm/messagequeue/message_queue.h"

#include "net_source.h"

class CommFrequencyLimit;

namespace mars {
    namespace stn {
        
class LongLink;

class NetSourceTimerCheck {
  public:
    NetSourceTimerCheck(NetSource* _net_source, ActiveLogic& _active_logic, LongLink& _longlink, MessageQueue::MessageQueue_t  _messagequeue_id);
    ~NetSourceTimerCheck();
    void CancelConnect();

  public:
    boost::function<void ()> fun_time_check_suc_;

  private:
    void __Run(const std::string& _host);
    bool __TryConnnect(const std::string& _host);
    void __OnActiveChanged(bool _is_active);
    void __StartCheck();
    void __Check();
    void __StopCheck();

  private:
    Thread thread_;
    boost::signals2::scoped_connection active_connection_;
    NetSource* net_source_;
    SocketSelectBreaker breaker_;
    SocketSelect seletor_;
    CommFrequencyLimit* frequency_limit_;
    LongLink& longlink_;

    MessageQueue::ScopeRegister asyncreg_;
    MessageQueue::MessagePost_t asyncpost_;
    NetSource::DnsUtil dns_util_;
};
        
    }
}


#endif // STN_SRC_NETSOURCE_TIMERCHECK_H_
