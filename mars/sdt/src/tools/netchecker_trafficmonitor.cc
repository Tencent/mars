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
 * NetCheckTrafficMonitor.cpp
 *
 *  Created on: 2015年1月27日
 *      Author: wutianqiang
 */
#include "netchecker_trafficmonitor.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/platform_comm.h"

NetCheckTrafficMonitor::NetCheckTrafficMonitor(unsigned long mobileDataThreshold, bool isIgnoreRecvData, unsigned long wifiDataThreshold)
    : wifi_recv_data_size_(0)
    , wifi_send_data_size_(0)
    , mobile_recv_data_size_(0)
    , mobile_send_data_size_(0)
    , wifi_data_threshold_(wifiDataThreshold)
    , mobile_data_threshold_(mobileDataThreshold)
    , is_ignore_recv_data_(isIgnoreRecvData) {
    xinfo_function();
}


NetCheckTrafficMonitor::~NetCheckTrafficMonitor() {
    __dumpDataSize();
}


bool NetCheckTrafficMonitor::sendLimitCheck(unsigned long sendDataSize) {
    ScopedLock lock(mutex_);

    if ((wifi_send_data_size_ + sendDataSize) > wifi_data_threshold_
            || (mobile_send_data_size_ + sendDataSize) > mobile_data_threshold_) {
        xwarn2(TSF"sendLimitCheck!!!wifi_data_threshold_=%0,mobile_data_threshold_=%1,wifi_send_=%2,wifi_recv_=%3,mobile_send_=%4"
               ",mobile_recv_=%5,sendDataSize=%6", wifi_data_threshold_, mobile_data_threshold_, wifi_send_data_size_, wifi_recv_data_size_, mobile_send_data_size_, mobile_recv_data_size_, sendDataSize);
        return true;
    } else {
        __data(sendDataSize, 0);
    }

    return false;
}
bool NetCheckTrafficMonitor::recvLimitCheck(unsigned long recvDataSize) {
    ScopedLock lock(mutex_);
    __data(0, recvDataSize);

    if (!is_ignore_recv_data_) {
        if ((wifi_send_data_size_ + wifi_recv_data_size_) > wifi_data_threshold_
                || (mobile_send_data_size_ + mobile_recv_data_size_) > mobile_data_threshold_) {
            xwarn2(TSF"recvLimitCheck!!!wifi_data_threshold_=%0,mobile_data_threshold_=%1,wifi_send_=%2,wifi_recv_=%3,mobile_send_=%4"
                   ",mobile_recv_=%5", wifi_data_threshold_, mobile_data_threshold_, wifi_send_data_size_, wifi_recv_data_size_, mobile_send_data_size_, mobile_recv_data_size_);
            return true;
        }
    }

    return false;
}
void NetCheckTrafficMonitor::reset() {
    ScopedLock lock(mutex_);
    wifi_recv_data_size_ = 0;
    wifi_send_data_size_ = 0;
    mobile_recv_data_size_ = 0;
    mobile_send_data_size_ = 0;
    wifi_data_threshold_ = 0;
    mobile_data_threshold_ = 0;
}

int NetCheckTrafficMonitor::__data(unsigned long sendDataSize, unsigned long recvDataSize) {
    if (0 < sendDataSize || 0 < recvDataSize) {
        if (kMobile != getNetInfo())
            wifi_recv_data_size_ += recvDataSize;
        else
            mobile_recv_data_size_ += recvDataSize;

        if (kMobile != getNetInfo())
            wifi_send_data_size_ += sendDataSize;
        else
            mobile_send_data_size_ += sendDataSize;
    }

    return 0;
}

void NetCheckTrafficMonitor::__dumpDataSize() {
    xinfo_function();
    xinfo2(TSF"m_wifiRecvDataSize=%_,wifi_send_data_size_=%_,mobile_recv_data_size_=%_,mobile_send_data_size_=%_,wifi_data_threshold_=%_,mobile_data_threshold_=%_,is_ignore_recv_data_=%_"
           , wifi_recv_data_size_, wifi_send_data_size_, mobile_recv_data_size_, mobile_send_data_size_, wifi_data_threshold_, mobile_data_threshold_, is_ignore_recv_data_);
}
