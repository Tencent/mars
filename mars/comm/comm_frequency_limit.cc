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
//  comm_frequency_limit.cc
//  comm
//
//  Created by liucan on 13-11-23.
//

#include "comm/comm_frequency_limit.h"
#include "comm/xlogger/xlogger.h"
#include "comm/time_utils.h"


CommFrequencyLimit::CommFrequencyLimit(size_t _count, uint64_t _time_span)
    : count_(_count)
    , time_span_(_time_span) {
    xassert2(count_ > 0);
    xassert2(time_span_ > 0);
}

CommFrequencyLimit::~CommFrequencyLimit()
{}

// true pass, false limit
bool CommFrequencyLimit::Check() {
    uint64_t now = ::gettickcount();
    if (!touch_times_.empty() && (now<touch_times_.front()) ) { //if user modify the time, amend it
    	xwarn2(TSF"Must be modified time.now=%_", now);
    	size_t size = touch_times_.size();
    	touch_times_.clear();
    	for (size_t i=0; i<size; ++i) {
    		touch_times_.push_back(now-1);
    	}
    }

    if (touch_times_.size() <= count_) {
        touch_times_.push_back(now);
        return true;
    }

    xassert2(now > touch_times_.front());

    if ((now - touch_times_.front()) <= time_span_) {
        xwarn2(TSF"Freq Limit, count:%0 in %1 milsec", count_, time_span_);
        return false;
    }

    __DelOlderTouchTime(now);
    touch_times_.push_back(now);
    return true;
}

void CommFrequencyLimit::__DelOlderTouchTime(uint64_t _time) {
    for (std::list<uint64_t>::iterator iter = touch_times_.begin(); iter != touch_times_.end();) {
        if ((_time - (*iter)) > time_span_) {
            iter = touch_times_.erase(iter);
            continue;
        }

        break;
    }
}




