// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * frequency_limit.cc
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#include "frequency_limit.h"

#include "mars/comm/adler32.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/stn.h"

#define MAX_RECORD_COUNT (30)
#define RECORD_INTERCEPT_COUNT (105)

#define NOT_CLEAR_INTERCEPT_COUNT_RETRY (99)
#define NOT_CLEAR_INTERCEPT_COUNT (75)
#define NOT_CLEAR_INTERCEPT_INTERVAL_MINUTE (10*60*1000)
#define RUN_CLEAR_RECORDS_INTERVAL_MINUTE  (60*60*1000)

using namespace mars::stn;

FrequencyLimit::FrequencyLimit()
    : itime_record_clear_(::gettickcount())
{}

FrequencyLimit::~FrequencyLimit()
{}

bool FrequencyLimit::Check(const mars::stn::Task& _task, const void* _buffer, int _len, unsigned int& _span) {
    xverbose_function();

    if (!_task.limit_frequency) return true;

    unsigned long time_cur = ::gettickcount();
    xassert2(time_cur >= itime_record_clear_);
    unsigned long interval = time_cur - itime_record_clear_;

    if (RUN_CLEAR_RECORDS_INTERVAL_MINUTE <= interval) {
        xdebug2(TSF"__ClearRecord interval=%0, timeCur=%1, itimeRecordClear=%2", interval, time_cur, itime_record_clear_);
        itime_record_clear_ = time_cur;
        __ClearRecord();
    }

    unsigned long hash = ::adler32(0, (const unsigned char*)_buffer, _len);
    int find_index = __LocateIndex(hash);

    if (0 <= find_index) {
    	_span = __GetLastUpdateTillNow(find_index);
        __UpdateRecord(find_index);

        if (!__CheckRecord(find_index)) {
            xerror2(TSF"Anti-Avalanche had Catch Task, Task Info: ptr=%0, cmdid=%1, need_authed=%2, cgi:%3, channel_select=%4, limit_flow=%5",
                    &_task, _task.cmdid, _task.need_authed, _task.cgi, _task.channel_select, _task.limit_flow);
            xerror2(TSF"apBuffer Len=%0, Hash=%1, Count=%2, timeLastUpdate=%3",
                    _len, iarr_record_[find_index].hash_, iarr_record_[find_index].count_, iarr_record_[find_index].time_last_update_);
            xassert2(false);

            return false;
        }
    } else {
        xdebug2(TSF"InsertRecord Task Info: ptr=%0, cmdid=%1, need_authed=%2, cgi:%3, channel_select=%4, limit_flow=%5",
                &_task, _task.cmdid, _task.need_authed, _task.cgi, _task.channel_select, _task.limit_flow);

        __InsertRecord(hash);
    }

    return true;
}

void FrequencyLimit::__ClearRecord() {
    xdebug2(TSF"iarrRecord size=%0", iarr_record_.size());

    unsigned long time_cur = ::gettickcount();

    std::vector<STAvalancheRecord>::iterator first = iarr_record_.begin();

    while (first != iarr_record_.end()) {
        xassert2(time_cur >= first->time_last_update_);
        unsigned long interval = time_cur - first->time_last_update_;

        if (interval <= NOT_CLEAR_INTERCEPT_INTERVAL_MINUTE && NOT_CLEAR_INTERCEPT_COUNT <= first->count_) {
            int oldcount = first->count_;

            if (NOT_CLEAR_INTERCEPT_COUNT_RETRY < first->count_) first->count_ = NOT_CLEAR_INTERCEPT_COUNT_RETRY;

            xwarn2(TSF"timeCur:%_,  first->timeLastUpdate:%_, interval:%_, Hash:%_, oldcount:%_, Count:%_", time_cur, first->time_last_update_, interval, first->hash_, oldcount, first->count_);
            ++first;
        } else {
            first = iarr_record_.erase(first);
        }
    }
}

int FrequencyLimit::__LocateIndex(unsigned long _hash) const {
    for (int i = (int)iarr_record_.size() - 1; i >= 0; --i) {
        if (iarr_record_[i].hash_ == _hash)
            return i;
    }

    return -1;
}

void FrequencyLimit::__InsertRecord(unsigned long _hash) {
    if (MAX_RECORD_COUNT < iarr_record_.size()) {
        xassert2(false);
        return;
    }

    STAvalancheRecord temp;
    temp.count_ = 1;
    temp.hash_ = _hash;
    temp.time_last_update_ = ::gettickcount();

    if (MAX_RECORD_COUNT == iarr_record_.size()) {
        unsigned int del_index = 0;

        for (unsigned int i = 1; i < iarr_record_.size(); i++) {
            if (iarr_record_[del_index].time_last_update_ > iarr_record_[i].time_last_update_) {
                del_index = i;
            }
        }

        std::vector<STAvalancheRecord>::iterator it = iarr_record_.begin();
        it += del_index;
        iarr_record_.erase(it);
    }

    iarr_record_.push_back(temp);
}

void FrequencyLimit::__UpdateRecord(int _index) {
    xassert2(0 <= _index && (unsigned int)_index < iarr_record_.size());

    iarr_record_[_index].count_ += 1;
    iarr_record_[_index].time_last_update_ = ::gettickcount();
}

unsigned int FrequencyLimit::__GetLastUpdateTillNow(int _index) {
    xassert2(0 <= _index && (unsigned int)_index < iarr_record_.size());

    return (unsigned int)(::gettickcount() - iarr_record_[_index].time_last_update_);
}

bool FrequencyLimit::__CheckRecord(int _index) const {
    xassert2(0 <= _index && (unsigned int)_index < iarr_record_.size());
    return (iarr_record_[_index].count_) <= RECORD_INTERCEPT_COUNT;
}
