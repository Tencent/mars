//
//  comm_frequency_limit.h
//  comm
//
//  Created by 刘粲 on 13-11-23.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifndef COMM_COMM_FREQUENCY_LIMIT_H_
#define COMM_COMM_FREQUENCY_LIMIT_H_

#include <stdint.h>
#include <cstddef>
#include <list>

class CommFrequencyLimit {
  public:
    CommFrequencyLimit(size_t _count, uint64_t _time_span);
    ~CommFrequencyLimit();

    bool Check();  // true pass, false limit

  private:
    CommFrequencyLimit(CommFrequencyLimit&);
    CommFrequencyLimit& operator=(CommFrequencyLimit&);

    void __DelOlderTouchTime(uint64_t _time);

  private:
    size_t count_;
    uint64_t time_span_;
    std::list<uint64_t> touch_times_;
};



#endif /* defined(COMM_COMM_FREQUENCY_LIMIT_H_) */
