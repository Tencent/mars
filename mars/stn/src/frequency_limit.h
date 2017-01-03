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
 * frequency_limit.h
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#ifndef STN_SRC_FREQUENCY_LIMIT_H_
#define STN_SRC_FREQUENCY_LIMIT_H_

#include <vector>

namespace mars {
namespace stn {

struct Task;
struct STAvalancheRecord;

struct STAvalancheRecord {
    unsigned long hash_;
    int count_;
    unsigned long time_last_update_;
};

class FrequencyLimit {
  public:
    FrequencyLimit();
    virtual ~FrequencyLimit();

    bool Check(const mars::stn::Task& _task, const void* _buffer, int _len, unsigned int& _span);

  private:
    void __ClearRecord();
    void __InsertRecord(unsigned long _hash);
    bool __CheckRecord(int _index) const;
    void __UpdateRecord(int _index);
    unsigned int __GetLastUpdateTillNow(int _index);
    int __LocateIndex(unsigned long _hash) const;

  private:
    std::vector<STAvalancheRecord> iarr_record_;
    unsigned long itime_record_clear_;
};

}
}

#endif // STN_SRC_FREQUENCY_LIMIT_H_
