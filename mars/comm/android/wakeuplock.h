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
 * WakeUpLock.h
 *
 *  Created on: 2012-9-28
 *      Author: yerungui
 */

#ifndef WAKEUPLOCK_H_
#define WAKEUPLOCK_H_

#ifdef ANDROID

#include <stdint.h>

class WakeUpLock {
  public:
    WakeUpLock();
    ~WakeUpLock();

    void Lock(int64_t _timelock);  // ms
    void Lock();
    void Unlock();
    bool IsLocking();

  private:
    void* object_;
};

#endif

#endif /* WAKEUPLOCK_H_ */
