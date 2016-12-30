// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef MUTEX_H_
#define MUTEX_H_

#include <errno.h>

#include "boost/thread/mutex.hpp"
#include "boost/thread/recursive_mutex.hpp"

#include "assert/__assert.h"

class Mutex {
  public:
    typedef  boost::mutex handle_type;
    Mutex() {
    }

    ~Mutex() {
    }

    bool lock() {
        // 成功返回0，失败返回错误码
        mutex_.lock();
        return true;
    }

    bool unlock() {
        mutex_.unlock();
        return true;
    }

    bool trylock() {
        return mutex_.try_lock();
    }

    bool islocked() {
        bool ret = trylock();

        if (ret) unlock();

        return true != ret;
    }

    handle_type& internal() { return mutex_; }
  private:
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);
  private:
    boost::mutex mutex_;
};

class RecursiveMutex {
  public:
    typedef  boost::recursive_mutex handle_type;
    RecursiveMutex() {
    }

    ~RecursiveMutex() {
    }

    bool lock() {
        // 成功返回0，失败返回错误码
        m_mutex.lock();
        return true;
    }

    bool unlock() {
        m_mutex.unlock();
        return true;
    }

    bool trylock() {
        return m_mutex.try_lock();
    }

    bool islocked() {
        bool ret = trylock();

        if (ret) unlock();

        return true != ret;
    }

    handle_type& internal() { return m_mutex; }
  private:
    RecursiveMutex(const RecursiveMutex&);
    RecursiveMutex& operator=(const RecursiveMutex&);
  private:
    boost::recursive_mutex m_mutex;
};


#endif /* MUTEX_H_ */
