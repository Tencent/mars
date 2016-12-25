/*
 * zombie_task_manager.h
 *
 *  Created on: 2014-6-19
 *      Author: yerungui
 *  Copyright (c) 2013-2015 Tencent. All rights reserved.
 *
 */

#ifndef STN_SRC_ZOMBIE_TASK_MANAGER_H_
#define STN_SRC_ZOMBIE_TASK_MANAGER_H_

#include <list>
#include <stdint.h>

#include "boost/function.hpp"

#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/messagequeue/message_queue_utils.h"
#include "mars/stn/stn.h"

struct ZombieTask;

namespace mars {
    namespace stn {

class ZombieTaskManager {
  public:
    boost::function<void (const Task& _task)> fun_start_task_;
    boost::function<int (ErrCmdType _errtype, int _errcode, int _fail_handle, const Task& _task, unsigned int _taskcosttime)> fun_callback_;

  public:
    ZombieTaskManager(MessageQueue::MessageQueue_t _messagequeueid);
    ~ZombieTaskManager();

    bool SaveTask(const Task& _task, unsigned int _taskcosttime /*ms*/);
    bool StopTask(uint32_t _taskid);
    bool HasTask(uint32_t _taskid) const;
    void ClearTasks();
    void RedoTasks();
    void OnNetCoreStartTask();

  private:
    ZombieTaskManager(const ZombieTaskManager&);
    ZombieTaskManager& operator=(const ZombieTaskManager&);

  private:
    void __StartTask();
    void __TimerChecker();

  private:
    MessageQueue::ScopeRegister m_asyncreg;
    std::list<ZombieTask> lsttask_;
    uint64_t net_core_last_start_task_time_;
};
        
    }
}

#endif // STN_SRC_ZOMBIE_TASK_MANAGER_H_
