/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * scope_recursion_limit.h
 *
 *  Created on: Mar 12, 2014
 *      Author: yanguoyue
 */

#ifndef COMM_SCOPERECURSIONLIMIT_H_
#define COMM_SCOPERECURSIONLIMIT_H_

#include "comm/thread/tss.h"

class ScopeRecursionLimit {
  public:
    ScopeRecursionLimit(Tss* _tss): tss_(_tss) {
        tss_->set((void*)((intptr_t)tss_->get() + 1));
    }

    ~ScopeRecursionLimit() {
        tss_->set((void*)((intptr_t)tss_->get() - 1));
    }

    /**
     * return
     *     true-limit
     *     false-pass
     */
    bool CheckLimit(int _maxRecursionNum = 1) {
        return (intptr_t)tss_->get() > _maxRecursionNum;
    }

    intptr_t Get() const {
        return (intptr_t)tss_->get();
    }

  private:
    ScopeRecursionLimit(const ScopeRecursionLimit&);
    ScopeRecursionLimit& operator=(const ScopeRecursionLimit&);

  private:
    Tss* tss_;
};

#ifndef __CONCAT__
#define __CONCAT_IMPL(x, y)    x##y
#define __CONCAT__(x, y)    __CONCAT_IMPL(x, y)
#endif

#define    DEFINE_SCOPERECURSIONLIMIT(classname)\
    static Tss __CONCAT__(tss, __LINE__)(NULL);\
    ScopeRecursionLimit classname(&__CONCAT__(tss, __LINE__))


#endif /* COMM_SCOPERECURSIONLIMIT_H_ */
