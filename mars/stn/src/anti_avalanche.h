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
 * anti_avalanche.h
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#ifndef STN_SRC_ANTI_AVALANCHE_H_
#define STN_SRC_ANTI_AVALANCHE_H_

namespace mars {
namespace stn {

struct Task;

class FrequencyLimit;
class FlowLimit;

enum {
	kFrequencyLimit = 1,
	kFlowLimit = 2
};

class AntiAvalanche {
  public:
    AntiAvalanche(bool _isactive);
    virtual ~AntiAvalanche();

    bool Check(const Task& _task, const void* _buffer, int _len);
    void OnSignalActive(bool _isactive);

  public:
    AntiAvalanche(const AntiAvalanche&);
    AntiAvalanche& operator=(const AntiAvalanche&);

  private:
    FrequencyLimit* frequency_limit_;
    FlowLimit* flow_limit_;
};

}}

#endif // STN_SRC_ANTI_AVALANCHE_H_
