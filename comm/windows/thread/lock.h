/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#ifndef LOCK_H_
#define LOCK_H_

#include "assert/__assert.h"
#include "thread/mutex.h"
#include "thread/spinlock.h"

template <typename MutexType>
class BaseScopedLock {
  public:
    explicit BaseScopedLock(MutexType& mutex, bool initiallyLocked = true)
        : mutex_(mutex)
        , islocked_(false) {
        if (initiallyLocked) lock();
    }

    ~BaseScopedLock() {
        if (islocked_) unlock();
    }

    bool islocked() const {
        return islocked_;
    }

    bool timedlock(long _millisecond) {
        ASSERT(!islocked_);

        if (islocked_) return true;

        unsigned long start = gettickcount();
        unsigned long cur = start;

        while (cur <= start + _millisecond) {
            if (trylock()) break;

            usleep(50 * 1000);
            cur = gettickcount();
        }

        return islocked_;
    }

    void lock() {
        ASSERT(!islocked_);

        if (!islocked_ && mutex_.lock()) islocked_ = true;

        ASSERT(islocked_);
    }

    void unlock() {
        ASSERT(islocked_);

        if (islocked_) {
            mutex_.unlock();
            islocked_ = false;
        }
    }

    bool trylock() {
        if (islocked_) {
            return false;
        }

        islocked_ = mutex_.trylock();
        return islocked_;
    }

    MutexType& internal() {
        return mutex_;
    }

  private:
    BaseScopedLock(const BaseScopedLock&);
    BaseScopedLock& operator=(const BaseScopedLock&);

  private:
    MutexType& mutex_;
    bool islocked_;
};

typedef BaseScopedLock<Mutex> ScopedLock;
typedef BaseScopedLock<SpinLock> ScopedSpinLock;
typedef BaseScopedLock<RecursiveMutex> ScopedRecursiveLock;

#endif /* LOCK_H_ */
