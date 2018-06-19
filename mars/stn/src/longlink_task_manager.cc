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
 * longlink_task_manager.cc
 *
 *  Created on: 2012-7-17
 *      Author: yerungui
 */

#include "longlink_task_manager.h"

#include <algorithm>

#include "boost/bind.hpp"

#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/move_wrapper.h"
#include "mars/comm/platform_comm.h"
#ifdef ANDROID
#include "mars/comm/android/wakeuplock.h"
#endif
#include "mars/stn/config.h"
#include "mars/stn/task_profile.h"
#include "mars/stn/proto/longlink_packer.h"

#include "dynamic_timeout.h"
#include "net_channel_factory.h"

using namespace mars::stn;

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_LONKLINK_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

LongLinkTaskManager::LongLinkTaskManager(NetSource& _netsource, ActiveLogic& _activelogic, DynamicTimeout& _dynamictimeout, MessageQueue::MessageQueue_t  _messagequeue_id)
    : asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeue_id))
    , lastbatcherrortime_(0)
    , retry_interval_(0)
    , tasks_continuous_fail_count_(0)
    , longlink_(LongLinkChannelFactory::Create(_messagequeue_id, _netsource))
    , longlinkconnectmon_(new LongLinkConnectMonitor(_activelogic, *longlink_, _messagequeue_id))
    , dynamic_timeout_(_dynamictimeout)
#ifdef ANDROID
    , wakeup_lock_(new WakeUpLock())
#endif
{
    xinfo_function(TSF"handler:(%_,%_)", asyncreg_.Get().queue, asyncreg_.Get().seq);
    longlink_->OnSend = boost::bind(&LongLinkTaskManager::__OnSend, this, _1);
    longlink_->OnRecv = boost::bind(&LongLinkTaskManager::__OnRecv, this, _1, _2, _3);
    longlink_->OnResponse = boost::bind(&LongLinkTaskManager::__OnResponse, this, _1, _2, _3, _4, _5, _6, _7);
    longlink_->SignalConnection.connect(boost::bind(&LongLinkTaskManager::__SignalConnection, this, _1));
}

LongLinkTaskManager::~LongLinkTaskManager() {
    xinfo_function();
    longlink_->SignalConnection.disconnect(boost::bind(&LongLinkTaskManager::__SignalConnection, this, _1));
    asyncreg_.CancelAndWait();
    
    __BatchErrorRespHandle(kEctLocal, kEctLocalReset, kTaskFailHandleTaskEnd, Task::kInvalidTaskID, longlink_->Profile(), false);
    
    delete longlinkconnectmon_;
    LongLinkChannelFactory::Destory(longlink_);
#ifdef ANDROID
    delete wakeup_lock_;
#endif
}

bool LongLinkTaskManager::StartTask(const Task& _task) {
    xverbose_function();
    xdebug2(TSF"taskid=%0", _task.taskid);

    TaskProfile task(_task);
    task.link_type = Task::kChannelLong;

    lst_cmd_.push_back(task);
    lst_cmd_.sort(__CompareTask);

    __RunLoop();
    return true;
}

bool LongLinkTaskManager::StopTask(uint32_t _taskid) {
    xverbose_function();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        if (_taskid == first->task.taskid) {
            xinfo2(TSF"find the task taskid:%0", _taskid);

            longlink_->Stop(first->task.taskid);
            lst_cmd_.erase(first);
            return true;
        }

        ++first;
    }

    return false;
}

bool LongLinkTaskManager::HasTask(uint32_t _taskid) const {
    xverbose_function();

    std::list<TaskProfile>::const_iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::const_iterator last = lst_cmd_.end();

    while (first != last) {
    	if (_taskid == first->task.taskid) {
    		return true;
    	}
    	++first;
    }

    return false;
}

void LongLinkTaskManager::ClearTasks() {
    xverbose_function();
    longlink_->Disconnect(LongLink::kReset);
    MessageQueue::CancelMessage(asyncreg_.Get(), 0);
    lst_cmd_.clear();
}

unsigned int LongLinkTaskManager::GetTaskCount() {
    return (unsigned int)lst_cmd_.size();
}

unsigned int LongLinkTaskManager::GetTasksContinuousFailCount() {
    return tasks_continuous_fail_count_;
}

void LongLinkTaskManager::RedoTasks() {
    xinfo_function();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        first->InitSendParam();
        first->last_failed_dyntime_status = 0;

        first = next;
    }

    retry_interval_ = 0;

    MessageQueue::CancelMessage(asyncreg_.Get(), 0);
    __RunLoop();
}

void LongLinkTaskManager::RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid) {
    xverbose_function();
    __BatchErrorRespHandle(_err_type, _err_code, _fail_handle, _src_taskid, longlink_->Profile());
    __RunLoop();
}


void LongLinkTaskManager::__RunLoop() {
    
    if (lst_cmd_.empty()) {
#ifdef ANDROID
        /*cancel the last wakeuplock*/
        wakeup_lock_->Lock(500);
#endif
        return;
    }

    __RunOnTimeout();
    __RunOnStartTask();

    if (!lst_cmd_.empty()) {
#ifdef ANDROID
        wakeup_lock_->Lock(30 * 1000);
#endif
      MessageQueue::FasterMessage(asyncreg_.Get(),
                                  MessageQueue::Message((MessageQueue::MessageTitle_t)this, boost::bind(&LongLinkTaskManager::__RunLoop, this)),
                                  MessageQueue::MessageTiming(1000));
    } else {
#ifdef ANDROID
        /*cancel the last wakeuplock*/
        wakeup_lock_->Lock(500);
#endif
    }
}

void LongLinkTaskManager::__RunOnTimeout() {
    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    uint64_t cur_time = ::gettickcount();
    int socket_timeout_code = 0;
    bool istasktimeout = false;

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (first->running_id && 0 < first->transfer_profile.start_send_time) {
            if (0 == first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.first_pkg_timeout) {
                xerror2(TSF"task first-pkg timeout taskid:%_,  nStartSendTime=%_, nfirstpkgtimeout=%_",
                        first->task.taskid, first->transfer_profile.start_send_time / 1000, first->transfer_profile.first_pkg_timeout / 1000);
                socket_timeout_code = kEctLongFirstPkgTimeout;
                __SetLastFailedStatus(first);
            }

            if (0 < first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.last_receive_pkg_time >= ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval)) {
                xerror2(TSF"task pkg-pkg timeout, taskid:%_, nLastRecvTime=%_, pkg-pkg timeout=%_",
                        first->task.taskid, first->transfer_profile.last_receive_pkg_time / 1000, ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval) / 1000);
                socket_timeout_code = kEctLongPkgPkgTimeout;
            }
            
            if (cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.read_write_timeout) {
                xerror2(TSF"task read-write timeout, taskid:%_, , nStartSendTime=%_, nReadWriteTimeOut=%_",
                        first->task.taskid, first->transfer_profile.start_send_time / 1000, first->transfer_profile.read_write_timeout / 1000);
                socket_timeout_code = kEctLongReadWriteTimeout;
            }
        }

        if (cur_time - first->start_task_time >= first->task_timeout) {
            __SingleRespHandle(first, kEctLocal, kEctLocalTaskTimeout, kTaskFailHandleTaskTimeout, longlink_->Profile());
            istasktimeout = true;
        }

        first = next;
    }

    if (0 != socket_timeout_code) {
        dynamic_timeout_.CgiTaskStatistic("", kDynTimeTaskFailedPkgLen, 0);
        __BatchErrorRespHandle(kEctNetMsgXP, socket_timeout_code, kTaskFailHandleDefault, Task::kInvalidTaskID, longlink_->Profile());
        xassert2(fun_notify_network_err_);
        fun_notify_network_err_(__LINE__, kEctNetMsgXP, socket_timeout_code, longlink_->Profile().ip,  longlink_->Profile().port);
    } else if (istasktimeout) {
        __BatchErrorRespHandle(kEctNetMsgXP, kEctLongTaskTimeout, kTaskFailHandleDefault, Task::kInvalidTaskID, longlink_->Profile());
    }
}

void LongLinkTaskManager::__RunOnStartTask() {
    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    bool ismakesureauthruned = false;
    bool ismakesureauthsuccess = false;
    uint64_t curtime = ::gettickcount();

    bool canretry = curtime - lastbatcherrortime_ >= retry_interval_;
    bool canprint = true;
    int sent_count = 0;

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (first->running_id) {
            ++sent_count;
            first = next;
            continue;
        }

        //重试间隔, 不影响第一次发送的任务
        if (first->task.retry_count > first->remain_retry_count && !canretry) {
            xdebug2_if(canprint, TSF"retry interval:%0, curtime:%1, lastbatcherrortime_:%2, curtime-m_lastbatcherrortime:%3",
                       retry_interval_, curtime, lastbatcherrortime_, curtime - lastbatcherrortime_);
            
            canprint = false;
            first = next;
            continue;
        }

        // make sure login
        if (first->task.need_authed) {
            if (!ismakesureauthruned) {
                ismakesureauthruned = true;
                ismakesureauthsuccess = MakesureAuthed();
            }

            if (!ismakesureauthsuccess) {
                xinfo2_if(curtime % 3 == 0, TSF"makeSureAuth retsult=%0", ismakesureauthsuccess);
                first = next;
                continue;
            }
        }

        AutoBuffer bufreq;
        AutoBuffer buffer_extension;
        int error_code = 0;

        if (!first->antiavalanche_checked) {
			if (!Req2Buf(first->task.taskid, first->task.user_context, bufreq, buffer_extension, error_code, Task::kChannelLong)) {
				__SingleRespHandle(first, kEctEnDecode, error_code, kTaskFailHandleTaskEnd, longlink_->Profile());
				first = next;
				continue;
			}
			// 雪崩检测
			xassert2(fun_anti_avalanche_check_);
			if (!fun_anti_avalanche_check_(first->task, bufreq.Ptr(), (int)bufreq.Length())) {
				__SingleRespHandle(first, kEctLocal, kEctLocalAntiAvalanche, kTaskFailHandleTaskEnd, longlink_->Profile());
				first = next;
				continue;
			}
           first->antiavalanche_checked = true;
        }

        xassert2(first->antiavalanche_checked);
		if (!longlinkconnectmon_->MakeSureConnected()) {
            if (0 != first->task.channel_id) {
                __SingleRespHandle(first, kEctLocal, kEctLocalChannelID, kTaskFailHandleTaskEnd, longlink_->Profile());
            }
            
            first = next;
            continue;
		}

        if (0 != first->task.channel_id && longlink_->Profile().start_time != first->task.channel_id) {
            __SingleRespHandle(first, kEctLocal, kEctLocalChannelID, kTaskFailHandleTaskEnd, longlink_->Profile());
            first = next;
            continue;
        }
        
		if (0 == bufreq.Length()) {
			if (!Req2Buf(first->task.taskid, first->task.user_context, bufreq, buffer_extension, error_code, Task::kChannelLong)) {
				__SingleRespHandle(first, kEctEnDecode, error_code, kTaskFailHandleTaskEnd, longlink_->Profile());
				first = next;
				continue;
			}
			// 雪崩检测
			xassert2(fun_anti_avalanche_check_);
			if (!fun_anti_avalanche_check_(first->task, bufreq.Ptr(), (int)bufreq.Length())) {
				__SingleRespHandle(first, kEctLocal, kEctLocalAntiAvalanche, kTaskFailHandleTaskEnd, longlink_->Profile());
				first = next;
				continue;
			}
		}

		first->transfer_profile.loop_start_task_time = ::gettickcount();
        first->transfer_profile.first_pkg_timeout = __FirstPkgTimeout(first->task.server_process_cost, bufreq.Length(), sent_count, dynamic_timeout_.GetStatus());
        first->current_dyntime_status = (first->task.server_process_cost <= 0) ? dynamic_timeout_.GetStatus() : kEValuating;
        first->transfer_profile.read_write_timeout = __ReadWriteTimeout(first->transfer_profile.first_pkg_timeout);
        first->transfer_profile.send_data_size = bufreq.Length();
        first->running_id = longlink_->Send(bufreq, buffer_extension, first->task);

        if (!first->running_id) {
            xwarn2(TSF"task add into longlink readwrite fail cgi:%_, cmdid:%_, taskid:%_", first->task.cgi, first->task.cmdid, first->task.taskid);
            first = next;
            continue;
        }

        xinfo2(TSF"task add into longlink readwrite suc cgi:%_, cmdid:%_, taskid:%_, size:%_, timeout(firstpkg:%_, rw:%_, task:%_), retry:%_",
               first->task.cgi, first->task.cmdid, first->task.taskid, first->transfer_profile.send_data_size, first->transfer_profile.first_pkg_timeout / 1000,
               first->transfer_profile.read_write_timeout / 1000, first->task_timeout / 1000, first->remain_retry_count);

        if (first->task.send_only) {
            __SingleRespHandle(first, kEctOK, 0, kTaskFailHandleNoError, longlink_->Profile());
        }

        ++sent_count;
        first = next;
    }
}

bool LongLinkTaskManager::__SingleRespHandle(std::list<TaskProfile>::iterator _it, ErrCmdType _err_type, int _err_code, int _fail_handle, const ConnectProfile& _connect_profile) {
    xverbose_function();
    xassert2(kEctServer != _err_type);
    xassert2(_it != lst_cmd_.end());

    _it->transfer_profile.connect_profile = _connect_profile;
    
    if (kEctOK == _err_type) {
        retry_interval_ = 0;
        tasks_continuous_fail_count_ = 0;
    } else {
        ++tasks_continuous_fail_count_;
    }

    uint64_t curtime =  gettickcount();
    size_t receive_data_size = _it->transfer_profile.receive_data_size;
    size_t received_size = _it->transfer_profile.received_size;
    
    xassert2((kEctOK == _err_type) == (kTaskFailHandleNoError == _fail_handle), TSF"type:%_, handle:%_", _err_type, _fail_handle);

    if (0 >= _it->remain_retry_count || kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle || kTaskFailHandleTaskTimeout == _fail_handle) {
        xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end callback  long cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
        (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
        (TSF"cli(%_, %_, n:%_, sig:%_), ", _it->transfer_profile.external_ip, _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal)
        (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, receive_data_size-received_size? string_cast(received_size).str():"", receive_data_size-received_size? "/":"", receive_data_size, _connect_profile.conn_rtt, (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time), (curtime - _it->start_task_time), _it->remain_retry_count)
        (TSF"cgi:%_, taskid:%_, tid:%_", _it->task.cgi, _it->task.taskid, _connect_profile.tid);

        int cgi_retcode = fun_callback_(_err_type, _err_code, _fail_handle, _it->task, (unsigned int)(curtime - _it->start_task_time));
        int errcode = _err_code;

        if (!_it->task.send_only && _it->running_id) {
        	if (kEctOK == _err_type) {
				errcode = (cgi_retcode == 0 ? cgi_retcode : kEctServer);
			}
		}

        _it->end_task_time = ::gettickcount();
        _it->err_code = errcode;
        _it->err_type = _err_type;
        _it->PushHistory();
        ReportTaskProfile(*_it);

        lst_cmd_.erase(_it);
        return true;
    }

    xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end retry  long cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
    (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
    (TSF"cli(%_, %_, n:%_, sig:%_), ", _it->transfer_profile.external_ip, _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal)
    (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, receive_data_size-received_size? string_cast(received_size).str():"", receive_data_size-received_size? "/":"", receive_data_size, _connect_profile.conn_rtt, (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time), (curtime - _it->start_task_time), _it->remain_retry_count)
    (TSF"cgi:%_, taskid:%_, tid:%_", _it->task.cgi, _it->task.taskid, _connect_profile.tid);

    _it->remain_retry_count--;
    _it->PushHistory();
    _it->InitSendParam();
    
    return false;
}

void LongLinkTaskManager::__BatchErrorRespHandle(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, const ConnectProfile& _connect_profile, bool _callback_runing_task_only) {
    xassert2(kEctOK != _err_type);
    xassert2(kTaskFailHandleTaskTimeout != _fail_handle);

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (_callback_runing_task_only && !first->running_id) {
            first = next;
            continue;
        }
        
        if (_src_taskid == Task::kInvalidTaskID || _src_taskid == first->task.taskid)
            __SingleRespHandle(first, _err_type, _err_code, _fail_handle, _connect_profile);
        else
            __SingleRespHandle(first, _err_type, 0, _fail_handle, _connect_profile);

        first = next;
    }
    
    lastbatcherrortime_ = ::gettickcount();
    
    if (kEctLocal != _err_type &&  !lst_cmd_.empty()) {
        retry_interval_ = DEF_TASK_RETRY_INTERNAL;
    }
    
    if (kTaskFailHandleSessionTimeout == _fail_handle || kTaskFailHandleRetryAllTasks == _fail_handle) {
        longlink_->Disconnect(LongLink::kDecodeErr);
        MessageQueue::CancelMessage(asyncreg_.Get(), 0);
        retry_interval_ = 0;
    }
    
    if (kTaskFailHandleDefault == _fail_handle) {
        if (kEctDns != _err_type && kEctSocket != _err_type) {  // not longlink callback
            longlink_->Disconnect(LongLink::kDecodeErr);
        }
        MessageQueue::CancelMessage(asyncreg_.Get(), 0);
    }
    
    if (kEctNetMsgXP == _err_type) {
        longlink_->Disconnect(LongLink::kTaskTimeout);
        MessageQueue::CancelMessage(asyncreg_.Get(), 0);
    }
}

struct find_task {
  public:
    bool operator()(const TaskProfile& value) {return taskid == value.task.taskid;}

  public:
    uint32_t taskid;
};

std::list<TaskProfile>::iterator LongLinkTaskManager::__Locate(uint32_t _taskid) {
    if (Task::kInvalidTaskID == _taskid) return lst_cmd_.end();

    find_task find_functor;
    find_functor.taskid = _taskid;
    std::list<TaskProfile>::iterator it = std::find_if(lst_cmd_.begin(), lst_cmd_.end(), find_functor);

    return it;
}

void LongLinkTaskManager::__OnResponse(ErrCmdType _error_type, int _error_code, uint32_t _cmdid, uint32_t _taskid, AutoBuffer& _body, AutoBuffer& _extension, const ConnectProfile& _connect_profile) {
    move_wrapper<AutoBuffer> body(_body);
    move_wrapper<AutoBuffer> extension(_extension);
    RETURN_LONKLINK_SYNC2ASYNC_FUNC(boost::bind(&LongLinkTaskManager::__OnResponse, this, _error_type, _error_code, _cmdid, _taskid, body, extension, _connect_profile));
    // svr push notify
    
    if (kEctOK == _error_type && ::longlink_ispush(_cmdid, _taskid, body, extension))  {
        xinfo2(TSF"task push seq:%_, cmdid:%_, len:(%_, %_)", _taskid, _cmdid, body->Length(), extension->Length());
        OnPush(_connect_profile.start_time, _cmdid, _taskid, body, extension);
        return;
    }
    
    if (kEctOK != _error_type) {
        xwarn2(TSF"task error, taskid:%_, cmdid:%_, error_type:%_, error_code:%_", _taskid, _cmdid, _error_type, _error_code);
        __BatchErrorRespHandle(_error_type, _error_code, kTaskFailHandleDefault, 0, _connect_profile);
        return;
    }
    
    std::list<TaskProfile>::iterator it = __Locate(_taskid);
    
    if (lst_cmd_.end() == it) {
        xwarn2_if(Task::kInvalidTaskID != _taskid, TSF"task no found task:%0, cmdid:%1, ect:%2, errcode:%3",
                  _taskid, _cmdid, _error_type, _error_code);
        return;
    }
    
    it->transfer_profile.received_size = body->Length();
    it->transfer_profile.receive_data_size = body->Length();
    it->transfer_profile.last_receive_pkg_time = ::gettickcount();
    
    int err_code = 0;
    int handle_type = Buf2Resp(it->task.taskid, it->task.user_context, body, extension, err_code, Task::kChannelLong);
    
    switch(handle_type){
        case kTaskFailHandleNoError:
        {
            dynamic_timeout_.CgiTaskStatistic(it->task.cgi, (unsigned int)it->transfer_profile.send_data_size + (unsigned int)body->Length(), ::gettickcount() - it->transfer_profile.start_send_time);
            __SingleRespHandle(it, kEctOK, err_code, handle_type, _connect_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, kEctOK, err_code, _connect_profile.ip, _connect_profile.port);
        }
            break;
        case kTaskFailHandleSessionTimeout:
        {
            xassert2(fun_notify_retry_all_tasks);
            xwarn2(TSF"task decode error session timeout taskid:%_, cmdid:%_, cgi:%_", it->task.taskid, it->task.cmdid, it->task.cgi);
            fun_notify_retry_all_tasks(kEctEnDecode, err_code, handle_type, it->task.taskid);
        }
            break;
        case kTaskFailHandleRetryAllTasks:
        {
            xassert2(fun_notify_retry_all_tasks);
            xwarn2(TSF"task decode error retry all task taskid:%_, cmdid:%_, cgi:%_", it->task.taskid, it->task.cmdid, it->task.cgi);
            fun_notify_retry_all_tasks(kEctEnDecode, err_code, handle_type, it->task.taskid);
        }
            break;
        case kTaskFailHandleTaskEnd:
        {
            xwarn2(TSF"task decode error taskid:%_, cmdid:%_, handle_type:%_", it->task.taskid, it->task.cmdid, handle_type);
            __SingleRespHandle(it, kEctEnDecode, err_code, handle_type, _connect_profile);
        }
            break;
        case kTaskFailHandleDefault:
        {
            xerror2(TSF"task decode error taskid:%_, handle_type:%_, err_code:%_, body dump:%_", it->task.taskid, handle_type, err_code, xdump(body->Ptr(), body->Length()));
            __BatchErrorRespHandle(kEctEnDecode, err_code, handle_type, it->task.taskid, _connect_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, kEctEnDecode, err_code, _connect_profile.ip, _connect_profile.port);
        }
            break;
        default:
        {
			xassert2(false, TSF"task decode error fail_handle:%_, taskid:%_", handle_type, it->task.taskid);
			__BatchErrorRespHandle(kEctEnDecode, err_code, handle_type, it->task.taskid, _connect_profile);
			xassert2(fun_notify_network_err_);
			fun_notify_network_err_(__LINE__, kEctEnDecode, handle_type, _connect_profile.ip, _connect_profile.port);
			break;
		}
    }

}

void LongLinkTaskManager::__OnSend(uint32_t _taskid) {
    RETURN_LONKLINK_SYNC2ASYNC_FUNC(boost::bind(&LongLinkTaskManager::__OnSend, this, _taskid));
    xverbose_function();

    std::list<TaskProfile>::iterator it = __Locate(_taskid);

    if (lst_cmd_.end() != it) {
    	if (it->transfer_profile.first_start_send_time == 0)
    		it->transfer_profile.first_start_send_time = ::gettickcount();
        it->transfer_profile.start_send_time = ::gettickcount();
        xdebug2(TSF"taskid:%_, starttime:%_", it->task.taskid, it->transfer_profile.start_send_time / 1000);
    }
}

void LongLinkTaskManager::__OnRecv(uint32_t _taskid, size_t _cachedsize, size_t _totalsize) {
    RETURN_LONKLINK_SYNC2ASYNC_FUNC(boost::bind(&LongLinkTaskManager::__OnRecv, this, _taskid, _cachedsize, _totalsize));
    xverbose_function();
    std::list<TaskProfile>::iterator it = __Locate(_taskid);

    if (lst_cmd_.end() != it) {
        it->transfer_profile.received_size = _cachedsize;
        it->transfer_profile.receive_data_size = _totalsize;
        it->transfer_profile.last_receive_pkg_time = ::gettickcount();
        xdebug2(TSF"taskid:%_, cachedsize:%_, _totalsize:%_", it->task.taskid, _cachedsize, _totalsize);
    } else {
        xwarn2(TSF"not found taskid:%_ cachedsize:%_, _totalsize:%_", _taskid, _cachedsize, _totalsize);
    }
}

void LongLinkTaskManager::__SignalConnection(LongLink::TLongLinkStatus _connect_status) {
	if (LongLink::kConnected == _connect_status)
        __RunLoop();
}

