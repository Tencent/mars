#include <stdlib.h>
#import <libkern/OSAtomic.h>
#import <os/lock.h>
#import "comm/thread/spinlock.h"

SpinLock::SpinLock() {
    if (@available(iOS 10, *)) { 
        // Use iOS 10 APIs. 
        os_unfair_lock* lock = (os_unfair_lock*)malloc(sizeof(os_unfair_lock));
        *lock = OS_UNFAIR_LOCK_INIT;
        lock_ = lock;
    } else { 
        // Alternative code for earlier versions of iOS. 
        OSSpinLock* lock = (OSSpinLock*)malloc(sizeof(OSSpinLock));
        *lock = OS_SPINLOCK_INIT;
        lock_ = lock;
    }
}

SpinLock::~SpinLock() {
    free(lock_);
}

bool SpinLock::lock() {
    if (@available(iOS 10, *)) {
        os_unfair_lock_lock((os_unfair_lock*)lock_);
    } else {
        OSSpinLockLock((OSSpinLock*)lock_);
    }
    return true;
}

bool SpinLock::unlock() {
    if (@available(iOS 10, *)) {
        os_unfair_lock_unlock((os_unfair_lock*)lock_);
    } else {
        OSSpinLockUnlock((OSSpinLock*)lock_);
    }
    return true;
}

bool SpinLock::trylock() {
    if (@available(iOS 10, *)) {
        return os_unfair_lock_trylock((os_unfair_lock*)lock_);
    } else {
        return OSSpinLockTry((OSSpinLock*)lock_);
    }
}

