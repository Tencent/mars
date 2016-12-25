/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

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
