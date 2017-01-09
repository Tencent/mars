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
//  signalling_keeper.cc
//  network
//
//  Created by liucan on 13-12-24.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#include "signalling_keeper.h"

#include <vector>
#include <string>

#include "boost/bind.hpp"

#include "mars/comm/socket/udpclient.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/proto/longlink_packer.h"

using namespace mars::stn;

static unsigned int g_period = 5 * 1000;  // ms
static unsigned int g_keepTime = 20 *1000;  // ms


SignallingKeeper::SignallingKeeper(const LongLink& _longlink, MessageQueue::MessageQueue_t _messagequeue_id, bool _use_UDP)
:msgreg_(MessageQueue::InstallAsyncHandler(_messagequeue_id))
, last_touch_time_(0)
, keeping_(false)
, longlink_(_longlink)
, port_(0)
, udp_client_(ip_, port_, this)
, use_UDP_(_use_UDP)
{
    xinfo2(TSF"SignallingKeeper messagequeue_id=%_", MessageQueue::Handler2Queue(msgreg_.Get()));
}

SignallingKeeper::~SignallingKeeper()
{
    Stop();
}


void SignallingKeeper::SetStrategy(unsigned int _period, unsigned int _keep_time)
{
    xinfo2(TSF"signal keeper period:%0, keepTime:%1", _period, _keep_time);
    xassert2(_period > 0);
    xassert2(_keep_time > 0);
    if (_period == 0 || _keep_time == 0)
    {
        xerror2(TSF"wrong strategy");
        return;
    }
    
    g_period = _period;
    g_keepTime = _keep_time;
}

void SignallingKeeper::OnNetWorkDataChanged(const char*, ssize_t, ssize_t)
{
    if (!keeping_) return;
    uint64_t now = ::gettickcount();
    xassert2(now >= last_touch_time_);
    
    if (now < last_touch_time_ || now - last_touch_time_ > g_keepTime)
    {
        keeping_ = false;
        return;
    }
    
    if (postid_ != MessageQueue::KNullPost) {
        MessageQueue::CancelMessage(postid_);
    }
    
    postid_ = MessageQueue::AsyncInvokeAfter(g_period, boost::bind(&SignallingKeeper::__OnTimeOut, this), msgreg_.Get());
}



void SignallingKeeper::Keep()
{
    xinfo2(TSF"start signalling, period:%0, keepTime:%1, use udp:%2, keeping_:%3", g_period, g_keepTime, use_UDP_, keeping_);
    last_touch_time_ = ::gettickcount();

    if (!keeping_)
    {
        __SendSignallingBuffer();
        keeping_ = true;
    }
}

void SignallingKeeper::Stop()
{
    xinfo2(TSF"stop signalling");
    
    if (keeping_ && postid_ != MessageQueue::KNullPost) {
        keeping_ = false;
        MessageQueue::CancelMessage(postid_);
    }
}

void SignallingKeeper::__SendSignallingBuffer()
{
    if (use_UDP_)
    {
        ConnectProfile link_info = longlink_.Profile();
        if (udp_client_.HasBuuferToSend()) return;
        
        if (link_info.ip != "" && link_info.port != 0
           && link_info.ip != ip_ && link_info.port != port_)
        {
            ip_ = link_info.ip;
            port_ = link_info.port;
        }
        
        if (ip_ != "" && port_ != 0)
        {
            udp_client_.SetIpPort(ip_, port_);
            AutoBuffer buffer;
            longlink_pack(signal_keep_cmdid(), 0, NULL, 0, buffer);
            udp_client_.SendAsync(buffer.Ptr(), buffer.Length());
        }
    } else {
        if (fun_send_signalling_buffer_)
        {
            fun_send_signalling_buffer_(NULL, 0, signal_keep_cmdid());
        }
    }
}

void SignallingKeeper::__OnTimeOut()
{
    xdebug2(TSF"sent signalling, period:%0", g_period);
    __SendSignallingBuffer();
}

void SignallingKeeper::OnError(UdpClient* _this, int _errno)
{
}
void SignallingKeeper::OnDataGramRead(UdpClient* _this, void* _buf, size_t _len)
{
}

void SignallingKeeper::OnDataSent(UdpClient* _this)
{
    OnNetWorkDataChanged("", 0, 0);
}
