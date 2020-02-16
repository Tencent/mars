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
 * longlink_metadata.cc
 *
 *  Created on: 2019-08-13
 *      Author: zhouzhijie
 */

#include "longlink_metadata.h"
#include "net_channel_factory.h"

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_LONKLINK_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

namespace mars {
    namespace stn {

LongLinkMetaData::LongLinkMetaData(const LonglinkConfig& _config, NetSource& _netsource, ActiveLogic& _activeLogic, MessageQueue::MessageQueue_t _message_id)
    :longlink_(LongLinkChannelFactory::Create(_message_id, _netsource, _config)), longlink_monitor_(nullptr), netsource_checker_(nullptr)
    , signal_keeper_(nullptr), config_(_config)
    , asyncreg_(MessageQueue::InstallAsyncHandler(_message_id)) {
        xinfo_function(TSF"create longlink with name:%_, group:%_", _config.name, _config.group);

        netsource_checker_ = std::make_shared<NetSourceTimerCheck>(&_netsource, _activeLogic, *(longlink_.get()), _message_id);
        netsource_checker_->fun_time_check_suc_ = boost::bind(&LongLinkMetaData::__OnTimerCheckSuc, this, config_.name);
        
        longlink_monitor_ = std::make_shared<LongLinkConnectMonitor>(_activeLogic, *(longlink_.get()), _message_id, _config.is_keep_alive);

        signal_keeper_ = std::make_shared<SignallingKeeper>(*(longlink_.get()), _message_id);
        signal_keeper_->fun_send_signalling_buffer_ = boost::bind(&LongLink::SendWhenNoData, longlink_.get(), _1, _2, _3, Task::kSignallingKeeperTaskID);
}

LongLinkMetaData::~LongLinkMetaData() {
    xinfo_function();
    
}

void LongLinkMetaData::__OnTimerCheckSuc(const std::string& _name) {
    SYNC2ASYNC_FUNC(boost::bind(&LongLinkMetaData::__OnTimerCheckSuc, this, _name));

    if(longlink_->Profile().ip_type != IPSourceType::kIPSourceBackup) {
        xinfo2(TSF"longlink %_ is not using backip, ignore", _name);
        return;
    }
    longlink_->Disconnect(LongLink::kTimeCheckSucc);
}

    }
}
