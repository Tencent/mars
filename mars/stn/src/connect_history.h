// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STN_SRC_CONNECT_HISTORY_H_
#define STN_SRC_CONNECT_HISTORY_H_

#include <list>
#include <mutex>

#include "mars/comm/socket/socket_address.h"
#include "stn/stn.h"

namespace mars {
namespace stn {

struct HistoryRtt {
    int transport_protocol = Task::kTransportProtocolTCP;
    unsigned int rtt = 0;
    uint64_t connected_tickcount = 0;
    socket_address connected_address_;
};

class ConnectHistory {
 public:
    void AddRtt(const HistoryRtt& rtt);
    void Reset();
    bool GetRtt(const socket_address& address, int transport_protocol, struct HistoryRtt* outrtt);

 private:
    std::mutex mutex_;
    std::list<HistoryRtt> lst_history_rtt_;
};
}  // namespace stn
}  // namespace mars

#endif