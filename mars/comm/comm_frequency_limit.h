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
//  comm_frequency_limit.h
//  comm
//
//  Created by liucan on 13-11-23.
//

#ifndef COMM_COMM_FREQUENCY_LIMIT_H_
#define COMM_COMM_FREQUENCY_LIMIT_H_

#include <stdint.h>
#include <cstddef>
#include <list>

class CommFrequencyLimit {
  public:
    CommFrequencyLimit(size_t _count, uint64_t _time_span);
    ~CommFrequencyLimit();

    bool Check();  // true pass, false limit

  private:
    CommFrequencyLimit(CommFrequencyLimit&);
    CommFrequencyLimit& operator=(CommFrequencyLimit&);

    void __DelOlderTouchTime(uint64_t _time);

  private:
    size_t count_;
    uint64_t time_span_;
    std::list<uint64_t> touch_times_;
};



#endif /* defined(COMM_COMM_FREQUENCY_LIMIT_H_) */
