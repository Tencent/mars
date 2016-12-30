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
 * NetCheckTrafficMonitor.h
 *
 *  Created on: 2015年1月27日
 *      Author: wutianqiang
 */

#ifndef NETCHECKTRAFFICMONITOR_H_
#define NETCHECKTRAFFICMONITOR_H_

#include <limits.h>

#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"

class NetCheckTrafficMonitor {
  public:
    NetCheckTrafficMonitor(unsigned long mobileDataThreshold, bool isIgnoreRecvData = true, unsigned long wifiDataThreshold = ULONG_MAX);
    ~NetCheckTrafficMonitor();
    bool sendLimitCheck(unsigned long sendDataSize);
    bool recvLimitCheck(unsigned long recvDataSize);
    void reset();

  private:
    int __data(unsigned long sendDataSize, unsigned long recvDataSize);
    void __dumpDataSize();

  private:
    NetCheckTrafficMonitor(const NetCheckTrafficMonitor&);
    NetCheckTrafficMonitor& operator=(const NetCheckTrafficMonitor&);

  private:
    unsigned long wifi_recv_data_size_;
    unsigned long wifi_send_data_size_;
    unsigned long mobile_recv_data_size_;
    unsigned long mobile_send_data_size_;
    unsigned long wifi_data_threshold_;
    unsigned long mobile_data_threshold_;
    bool is_ignore_recv_data_;
    Mutex mutex_;
};



#endif /* NETCHECKTRAFFICMONITOR_H_ */
