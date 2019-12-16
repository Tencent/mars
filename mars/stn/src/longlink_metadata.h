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
 * longlink_manager.h
 *
 *  Created on: 2019-08-13
 *      Author: zhouzhijie
 */

#ifndef STN_SRC_LONGLINK_MANAGER_H_
#define STN_SRC_LONGLINK_MANAGER_H_

#include <memory>
#include "longlink_connect_monitor.h"
#include "netsource_timercheck.h"
#include "signalling_keeper.h"
#include "longlink.h"
#include "mars/stn/stn.h"
#include "mars/comm/messagequeue/message_queue.h"

namespace mars {
    namespace stn {

class LongLinkMetaData {
public: 
    LongLinkMetaData(const LonglinkConfig& _config, NetSource& _netsource, ActiveLogic& _activeLogic, MessageQueue::MessageQueue_t _message_id);

    virtual ~LongLinkMetaData();
    std::shared_ptr<LongLink> Channel() {
        if(longlink_ == nullptr) {
            xassert2(false, TSF"null longlink, name:%_", config_.name.c_str());
        }
        return longlink_;
    }
    
    std::shared_ptr<LongLinkConnectMonitor> Monitor() {
        return longlink_monitor_;
    }

    LonglinkConfig& Config() { return config_; }
    std::shared_ptr<SignallingKeeper> SignalKeeper() {
        return signal_keeper_;
    }
    
    std::shared_ptr<NetSourceTimerCheck> Checker() {
        return netsource_checker_;
    }

private:
    void __OnTimerCheckSuc(const std::string& _name);

private: 
    std::shared_ptr<LongLink> longlink_;
    std::shared_ptr<LongLinkConnectMonitor> longlink_monitor_;
    std::shared_ptr<NetSourceTimerCheck> netsource_checker_;
    std::shared_ptr<SignallingKeeper> signal_keeper_;
    LonglinkConfig config_;
    MessageQueue::ScopeRegister asyncreg_;
};

    }
}

#endif
