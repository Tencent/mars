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
 * active_logic.h
 *
 *  Created on: 2012-8-22
 *      Author: yerungui
 */

#ifndef MMCOMM_SRC_ACTIVE_LOGIC_H_
#define MMCOMM_SRC_ACTIVE_LOGIC_H_

#include "boost/signals2.hpp"

#include "mars/comm/alarm.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/singleton.h"

class ActiveLogic
{
public:
    SINGLETON_INTRUSIVE(ActiveLogic, new ActiveLogic, delete);
    boost::signals2::signal<void (bool _isForeground)> SignalForeground;
    boost::signals2::signal<void (bool _isactive)> SignalActive;

public:
    ActiveLogic();
    virtual ~ActiveLogic();

    void OnForeground(bool _isforeground);
    bool IsActive() const;

    bool IsForeground() const;
    uint64_t LastForegroundChangeTime() const;

private:
    void __OnInActive();

private:
    bool   isforeground_;
    bool   isactive_;
    Alarm  alarm_;
    uint64_t lastforegroundchangetime_;
};

#endif // MMCOMM_SRC_ACTIVE_LOGIC_H_
