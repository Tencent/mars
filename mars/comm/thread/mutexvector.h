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
//  mutexvector.h
//
//  Created by yerungui on 13-4-23.
//

#ifndef __MUTEXVECTOR__
#define __MUTEXVECTOR__

#include "comm/thread/lock.h"
#include "comm/thread/condition.h"

class MutexVector {
    friend class ScopedMutexVector;
  public:
    MutexVector() {
        m_vec = 0;
        m_count = 0;
    }
  private:
    Condition m_cond;
    Mutex m_mutex;
    int m_vec;
    int m_count;
};

class ScopedMutexVector {
  public:
    ScopedMutexVector(MutexVector& _mutex, int _vector, bool _initiallyLocked = true)
        : mutex_vector_(_mutex), vec_(_vector), islocked_(false)
    { if (_initiallyLocked) Lock(); }

    ~ScopedMutexVector() { if (islocked_) UnLock();}

    bool IsLocked() const { return islocked_;}

    void Lock() {
        ASSERT(!islocked_);

        if (islocked_) return;

        ScopedLock lock(mutex_vector_.m_mutex);

        if (mutex_vector_.m_vec == vec_) {
            ++mutex_vector_.m_count;
            islocked_ = true;
            return;
        }

        while (mutex_vector_.m_count > 0 && mutex_vector_.m_vec != vec_)
        { mutex_vector_.m_cond.wait(lock);}

        ASSERT(0 <= mutex_vector_.m_count);

        ++mutex_vector_.m_count;
        islocked_ = true;

        if (mutex_vector_.m_vec != vec_) {
            ASSERT(1 == mutex_vector_.m_count);
            mutex_vector_.m_vec = vec_;
            mutex_vector_.m_cond.notifyAll(lock);
        }
    }

    void UnLock() {
        ASSERT(islocked_);

        if (!islocked_) return;

        ScopedLock lock(mutex_vector_.m_mutex);
        ASSERT(vec_ == mutex_vector_.m_vec);
        --mutex_vector_.m_count;
        ASSERT(0 <= mutex_vector_.m_count);
        islocked_ = false;

        if (0 >= mutex_vector_.m_count)
            mutex_vector_.m_cond.notifyAll(lock);
    }

    bool TryLock() {
        if (islocked_) return false;

        ScopedLock lock(mutex_vector_.m_mutex);

        if (mutex_vector_.m_vec == vec_) {
            ++mutex_vector_.m_count;
            islocked_ = true;
            return true;
        }

        if (mutex_vector_.m_count > 0) return false;

        ASSERT(0 == mutex_vector_.m_count);

        mutex_vector_.m_vec = vec_;
        mutex_vector_.m_count = 1;
        islocked_ = true;
        mutex_vector_.m_cond.notifyAll(lock);
        return true;
    }

  private:
    MutexVector& mutex_vector_;
    int vec_;
    bool islocked_;
};

#endif
