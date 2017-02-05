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
 * zombie_task_manager.cc
 *
 *  Created on: 2014-6-19
 *      Author: yerungui
 *  Copyright (c) 2013-2015 Tencent. All rights reserved.
 *
 */

#include "zombie_task_manager.h"

#include "boost/bind.hpp"

#include "mars/comm/time_utils.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/xlogger/xlogger.h"

using namespace mars::stn;

static uint64_t RETRY_INTERVAL = 60 * 1000;

struct ZombieTask
{
    Task task;
    uint64_t save_time;
};

static bool __compare_task(const ZombieTask& first, const ZombieTask& second)
{
  return first.task.priority < second.task.priority;
}

ZombieTaskManager::ZombieTaskManager(MessageQueue::MessageQueue_t _messagequeueid)
		: asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
		, net_core_last_start_task_time_(gettickcount()) {}

ZombieTaskManager::~ZombieTaskManager()
{
	asyncreg_.CancelAndWait();
}

bool ZombieTaskManager::SaveTask(const Task& _task, unsigned int _taskcosttime)
{
    if (_task.network_status_sensitive) return false;
    ZombieTask zombie_task = {_task, ::gettickcount()};
 
    zombie_task.task.retry_count = 0;
    zombie_task.task.total_timetout -= _taskcosttime;

    if (0 >= zombie_task.task.total_timetout) return false;

    lsttask_.push_back(zombie_task);
    
    xinfo2(TSF"task end callback zombie savetask cgi:%_, cmdid:%_, taskid:%_", _task.cgi, _task.cmdid, _task.taskid);

    MessageQueue::SingletonMessage(false, asyncreg_.Get(),
                                    MessageQueue::Message((MessageQueue::MessageTitle_t)this,
                                    boost::bind(&ZombieTaskManager::__TimerChecker, this)),
                                    MessageQueue::MessageTiming(3000, 3000));
    return true;
}

bool ZombieTaskManager::StopTask(uint32_t _taskid)
{
    std::list<ZombieTask>::iterator first = lsttask_.begin();
    std::list<ZombieTask>::iterator last = lsttask_.end();
    while (first != last)
    {
        if (_taskid == first->task.taskid)
        {
            xinfo2(TSF"find the task taskid:%0", _taskid);
            lsttask_.erase(first);
            return true;
        }
        ++first;
    }
    return false;
}

bool ZombieTaskManager::HasTask(uint32_t _taskid) const{
    xverbose_function();

    std::list<ZombieTask>::const_iterator first = lsttask_.begin();
    std::list<ZombieTask>::const_iterator last = lsttask_.end();

    while (first != last) {
    	if (_taskid == first->task.taskid) {
    		return true;
    	}
    	++first;
    }

    return false;
}

void ZombieTaskManager::ClearTasks()
{
    lsttask_.clear();
}

void ZombieTaskManager::RedoTasks()
{
    __StartTask();
}

void ZombieTaskManager::OnNetCoreStartTask()
{
    net_core_last_start_task_time_ = gettickcount();
}

void ZombieTaskManager::__StartTask()
{
    xassert2(fun_start_task_);

    if (lsttask_.empty()) return;
    
    std::list<ZombieTask> lsttask = lsttask_;
    lsttask_.clear();
    lsttask.sort(__compare_task);

    for (std::list<ZombieTask>::iterator it=lsttask.begin(); it != lsttask.end(); ++it)
    {
        uint64_t cur_time = ::gettickcount();

        if ((cur_time - it->save_time) >= (uint64_t)it->task.total_timetout)
        {
            xinfo2(TSF"task end callback zombie start timeout cgi:%_, cmdid:%_, taskid:%_, err(%_, %_), cost:%_", it->task.cgi, it->task.cmdid, it->task.taskid, kEctLocal, kEctLocalTaskTimeout, cur_time - it->save_time);
            fun_callback_(kEctLocal,  kEctLocalTaskTimeout, kTaskFailHandleTaskEnd, it->task, (unsigned int)(cur_time - it->save_time));
        } else {
            xinfo2(TSF"task start zombie cgi:%_, cmdid:%_, taskid:%_,", it->task.cgi, it->task.cmdid, it->task.taskid);
            it->task.total_timetout -= (cur_time - it->save_time);
            fun_start_task_(it->task);
        }
    }
}

void ZombieTaskManager::__TimerChecker()
{
    xassert2(fun_callback_);

    std::list<ZombieTask>& lsttask = lsttask_;
    uint64_t cur_time = ::gettickcount();
    uint64_t netCoreLastStartTaskTime = net_core_last_start_task_time_;

    for (std::list<ZombieTask>::iterator it=lsttask.begin(); it != lsttask.end(); )
    {
        if ((cur_time - it->save_time) >= (uint64_t)it->task.total_timetout)
        {
            xinfo2(TSF"task end callback zombie timeout cgi:%_, cmdid:%_, taskid:%_, err(%_, %_), cost:%_", it->task.cgi, it->task.cmdid, it->task.taskid, kEctLocal, kEctLocalTaskTimeout, cur_time - it->save_time);
            fun_callback_(kEctLocal,  kEctLocalTaskTimeout, kTaskFailHandleTaskEnd, it->task, (unsigned int)(cur_time - it->save_time));
            it = lsttask.erase(it);
        } else if ((cur_time - it->save_time) >= RETRY_INTERVAL && (cur_time - netCoreLastStartTaskTime) >= RETRY_INTERVAL) {
            xinfo2(TSF"task start zombie cgi:%_, cmdid:%_, taskid:%_,", it->task.cgi, it->task.cmdid, it->task.taskid);
            it->task.total_timetout -= (cur_time - it->save_time);
            fun_start_task_(it->task);
            it = lsttask.erase(it);
        } else {
            ++it;
        }
    }

    if (lsttask.empty()) MessageQueue::CancelMessage(asyncreg_.Get(), (MessageQueue::MessageTitle_t)this);
}
