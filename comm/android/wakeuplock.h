/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * WakeUpLock.h
 *
 *  Created on: 2012-9-28
 *      Author: 叶润桂
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
