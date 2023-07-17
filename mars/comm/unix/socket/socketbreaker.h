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
 * socketbreaker.h
 *
 *  Created on: 2015-12-9
 *      Author: yerungui
 */

#ifndef _SOCKSTBREAKER_
#define _SOCKSTBREAKER_

#include <mutex>

namespace mars {
namespace comm {

class SocketBreaker {
 public:
    SocketBreaker();
    ~SocketBreaker();

    bool IsCreateSuc() const;
    bool ReCreate();
    void Close();

    bool Break();
    bool Break(int reason);
    bool Clear();
    bool PreciseBreak(uint32_t cookie);
    bool PreciseClear(uint32_t* cookie);

    bool IsBreak() const;
    int BreakerFD() const;
    int BreakReason() const;

 private:
    SocketBreaker(const SocketBreaker&);
    SocketBreaker& operator=(const SocketBreaker&);
    void _Cleanup();

 private:
    int pipes_[2];
    bool create_success_ = false;
    bool breaked_ = false;
    int reason_ = 0;
    mutable std::mutex mutex_;
};

}  // namespace comm
}  // namespace mars

#endif
