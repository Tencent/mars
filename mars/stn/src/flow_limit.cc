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
 * flow_limit.cc
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#include "flow_limit.h"

#include <algorithm>

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/stn/stn.h"

#if true
static const int kInactiveSpeed = (2 * 1024 * 1024 / 3600);
static const int kActiveSpeed = (8 * 1024 * 1024 / 3600);
static const int kInactiveMinvol = (6 * 1024 * 1024);
static const int kMaxVol = (8 * 1024 * 1024);
#else
static const int kInactiveSpeed = (1);
static const int kActiveSpeed = (3);
static const int kInactiveMinvol = (1500);
static const int kMaxVol = (2 * 1024);
#endif

using namespace mars::stn;

FlowLimit::FlowLimit(bool _isactive)
    : funnel_speed_(_isactive ? kActiveSpeed : kInactiveSpeed)
    , cur_funnel_vol_(0)
    , time_lastflow_computer_(::gettickcount())
{}

FlowLimit::~FlowLimit()
{}

bool FlowLimit::Check(const mars::stn::Task& _task, const void* _buffer, int _len) {
    xverbose_function();

    if (!_task.limit_flow) {
        return true;
    }

    __FlashCurVol();

    if (cur_funnel_vol_ + _len > kMaxVol) {
        xerror2(TSF"Task Info: ptr=%_, cmdid=%_, need_authed=%_, cgi:%_, channel_select=%_, limit_flow=%_, cur_funnel_vol_(%_)+_len(%_)=%_,MAX_VOL:%_ ",
                &_task, _task.cmdid, _task.need_authed, _task.cgi, _task.channel_select, _task.limit_flow, cur_funnel_vol_ + _len, cur_funnel_vol_, _len, cur_funnel_vol_ + _len, kMaxVol);

        return false;
    }

    cur_funnel_vol_ += _len;
    return true;
}

void FlowLimit::Active(bool _isactive) {
    __FlashCurVol();

    if (!_isactive) {
        xdebug2(TSF"iCurFunnelVol=%0, INACTIVE_MIN_VOL=%1", cur_funnel_vol_, kInactiveMinvol);

        if (cur_funnel_vol_ > kInactiveMinvol)
            cur_funnel_vol_ = kInactiveMinvol;
    }

    funnel_speed_ = _isactive ? kActiveSpeed : kInactiveSpeed;
    xdebug2(TSF"Active:%0, iFunnelSpeed=%1", _isactive, funnel_speed_);
}

void FlowLimit::__FlashCurVol() {
    uint64_t timeCur = ::gettickcount();
	xassert2(timeCur >= time_lastflow_computer_, TSF"%_, %_", timeCur, time_lastflow_computer_);
    uint64_t interval = (timeCur - time_lastflow_computer_) / 1000;

    xdebug2(TSF"iCurFunnelVol=%0, iFunnelSpeed=%1, interval=%2", cur_funnel_vol_, funnel_speed_, interval);
    cur_funnel_vol_ -= interval * funnel_speed_;
    cur_funnel_vol_ = std::max(0, cur_funnel_vol_);
    xdebug2(TSF"iCurFunnelVol=%0", cur_funnel_vol_);

    time_lastflow_computer_ = timeCur;
}
