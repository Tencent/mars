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
 * flow_limit.h
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#ifndef STN_SRC_FLOW_LIMIT_H_
#define STN_SRC_FLOW_LIMIT_H_

#include <stdint.h>

namespace mars {
namespace stn {

struct Task;

class FlowLimit {
  public:
    FlowLimit(bool _isactive);
    virtual ~FlowLimit();

    bool Check(const mars::stn::Task& _task, const void* _buffer, int _len);
    void Active(bool _isactive);

  private:
    void __FlashCurVol();

  private:
    int funnel_speed_;
    int cur_funnel_vol_;
    uint64_t time_lastflow_computer_;
};

}}

#endif // STN_SRC_FLOW_LIMIT_H_
