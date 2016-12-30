// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.




#ifndef CONDITION_H_
#define CONDITION_H_

#include <errno.h>
#include <climits>

#include "boost/thread/condition.hpp"

#include "assert/__assert.h"
#include "thread/atomic_oper.h"
#include "thread/lock.h"

class Condition {
  public:
    Condition(): condition_(), mutex_(), anyway_notify_(0) {
    }

    ~Condition() {
    }

    void wait(ScopedLock& lock) {
        ASSERT(lock.islocked());

        if (!atomic_cas32(&anyway_notify_, 0, 1))
            condition_.wait(lock.internal().internal());

        anyway_notify_ = 0;
    }

    int wait(ScopedLock& lock, unsigned long millisecond) {
        ASSERT(lock.islocked());

        bool ret = false;

        if (!atomic_cas32(&anyway_notify_, 0, 1))
            ret = condition_.timed_wait(lock.internal().internal(), boost::get_system_time() + boost::posix_time::milliseconds(millisecond));

        anyway_notify_ = 0;

        if (!ret) return ETIMEDOUT;

        return 0;
    }

    void wait() {
        ScopedLock scopedLock(mutex_);
        wait(scopedLock);
    }

    int wait(unsigned long millisecond) {
        ScopedLock scopedLock(mutex_);
        return wait(scopedLock, millisecond);
    }

    void notifyOne() {
        condition_.notify_one();
    }

    void notifyOne(ScopedLock& lock) {
        ASSERT(lock.islocked());
        notifyOne();
    }

    void notifyAll(bool anywaynotify = false) {
        if (anywaynotify) anyway_notify_ = 1;

        condition_.notify_all();
    }

    void notifyAll(ScopedLock& lock, bool anywaynotify = false) {
        ASSERT(lock.islocked());
        notifyAll(anywaynotify);
    }

    void cancelAnyWayNotify() {
        anyway_notify_ = 0;
    }

  private:
    Condition(const Condition&);
    Condition& operator=(const Condition&);
  private:
    boost::condition condition_;
    Mutex mutex_;
    volatile unsigned int anyway_notify_;
};



#endif
