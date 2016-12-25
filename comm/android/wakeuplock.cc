/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * WakeUpLock.cpp
 *
 *  Created on: 2012-9-28
 *      Author: 叶润桂
 */

#ifdef ANDROID

#include "wakeuplock.h"
#include "assert/__assert.h"
#include "xlogger/xlogger.h"

#include "platform_comm.h"

WakeUpLock::WakeUpLock() {
    object_ = ::wakeupLock_new();
    ASSERT(object_);
    xinfo2("new wakeuplock:%p", object_);
}

WakeUpLock::~WakeUpLock() {
    ASSERT(object_);
    xinfo2("delete wakeuplock:%p", object_);

    if (IsLocking())
        ::wakeupLock_Unlock(object_);

    ::wakeupLock_delete(object_);
}

void WakeUpLock::Lock(int64_t _timelock) {
    ::wakeupLock_Lock_Timeout(object_, _timelock);
}

void WakeUpLock::Lock() {
    ::wakeupLock_Lock(object_);
}

void WakeUpLock::Unlock() {
    ::wakeupLock_Unlock(object_);
}

bool WakeUpLock::IsLocking() {
    return ::wakeupLock_IsLocking(object_);
}

#endif
