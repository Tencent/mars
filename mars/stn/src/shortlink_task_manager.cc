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
 * shortlink_task_manager.cc
 *
 *  Created on: 2012-8-24
 *      Author: zhouzhijie
 */

#include "shortlink_task_manager.h"

#include <algorithm>
#include <set>

#include "boost/bind.hpp"

#include "mars/app/app.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/move_wrapper.h"
#include "mars/comm/platform_comm.h"
#ifdef ANDROID
#include "mars/comm/android/wakeuplock.h"
#endif

#include "dynamic_timeout.h"
#include "net_channel_factory.h"
#include "weak_network_logic.h"

using namespace mars::stn;
using namespace mars::app;

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_SHORTLINK_SYNC2ASYNC_FUNC_TITLE(func, title) RETURN_SYNC2ASYNC_FUNC_TITLE(func, title, )

boost::function<void (const std::string& _user_id, std::vector<std::string>& _host_list)> ShortLinkTaskManager::get_real_host_;
boost::function<void (const int _error_type, const int _error_code, const int _use_ip_index)> ShortLinkTaskManager::task_connection_detail_;

ShortLinkTaskManager::ShortLinkTaskManager(NetSource& _netsource, DynamicTimeout& _dynamictimeout, MessageQueue::MessageQueue_t _messagequeueid)
    : asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
    , net_source_(_netsource)
    , default_use_proxy_(true)
    , tasks_continuous_fail_count_(0)
    , dynamic_timeout_(_dynamictimeout)
#ifdef ANDROID
    , wakeup_lock_(new WakeUpLock())
#endif
{
    xinfo_function(TSF"handler:(%_,%_)", asyncreg_.Get().queue, asyncreg_.Get().seq);
    xinfo2(TSF"ShortLinkTaskManager messagequeue_id=%_", MessageQueue::Handler2Queue(asyncreg_.Get()));
}

ShortLinkTaskManager::~ShortLinkTaskManager() {
    xinfo_function();
    asyncreg_.CancelAndWait();
    xinfo2(TSF"lst_cmd_ count=%0", lst_cmd_.size());
    __BatchErrorRespHandle(kEctLocal, kEctLocalReset, kTaskFailHandleTaskEnd, Task::kInvalidTaskID, false);
#ifdef ANDROID
    delete wakeup_lock_;
#endif
}

bool ShortLinkTaskManager::StartTask(const Task& _task) {
    xverbose_function();

    if (_task.send_only) {
        xassert2(false);
        xerror2(TSF"taskid:%_, short link should have resp", _task.taskid);
        return false;
    }


	xinfo2(TSF"task is long-polling task:%_, cgi:%_, timeout:%_",_task.long_polling, _task.cgi, _task.long_polling_timeout);

    xdebug2(TSF"taskid:%0", _task.taskid);

    TaskProfile task(_task);
    task.link_type = Task::kChannelShort;

    lst_cmd_.push_back(task);
    lst_cmd_.sort(__CompareTask);

    __RunLoop();
    return true;
}

bool ShortLinkTaskManager::StopTask(uint32_t _taskid) {
    xverbose_function();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        if (_taskid == first->task.taskid) {
            xinfo2(TSF"find the task, taskid:%0", _taskid);

            __DeleteShortLink(first->running_id);
            lst_cmd_.erase(first);
            return true;
        }

        ++first;
    }

    return false;
}

bool ShortLinkTaskManager::HasTask(uint32_t _taskid) const {
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

void ShortLinkTaskManager::ClearTasks() {
    xverbose_function();

    xinfo2(TSF"cmd size:%0", lst_cmd_.size());

    for (std::list<TaskProfile>::iterator it = lst_cmd_.begin(); it != lst_cmd_.end(); ++it) {
        __DeleteShortLink(it->running_id);
    }

    lst_cmd_.clear();
}

unsigned int ShortLinkTaskManager::GetTasksContinuousFailCount() {
    return tasks_continuous_fail_count_;
}


void ShortLinkTaskManager::__RunLoop() {
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
        wakeup_lock_->Lock(60 * 1000);
#endif
        MessageQueue::FasterMessage(asyncreg_.Get(),
                                    MessageQueue::Message((MessageQueue::MessageTitle_t)this, boost::bind(&ShortLinkTaskManager::__RunLoop, this), "ShortLinkTaskManager::__RunLoop"),
                                    MessageQueue::MessageTiming(1000));
    } else {
#ifdef ANDROID
        /*cancel the last wakeuplock*/
        wakeup_lock_->Lock(500);
#endif
    }
}

void ShortLinkTaskManager::__RunOnTimeout() {
    xverbose2(TSF"lst_cmd_ size=%0", lst_cmd_.size());
    socket_pool_.CleanTimeout();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    uint64_t cur_time = ::gettickcount();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        ErrCmdType err_type = kEctLocal;
        int socket_timeout_code = 0;
	    // xinfo2(TSF"task is long-polling task:%_,%_, cgi:%_,%_, timeout:%_, id %_",first->task.long_polling, first->transfer_profile.task.long_polling, first->transfer_profile.task.cgi,first->task.cgi, first->transfer_profile.task.long_polling_timeout, (void*)first->running_id);

        if (cur_time - first->start_task_time >= first->task_timeout) {
            err_type = kEctLocal;
            socket_timeout_code = kEctLocalTaskTimeout;
        } else if (first->running_id && 0 < first->transfer_profile.start_send_time && cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.read_write_timeout) {
            xerror2(TSF"task read-write timeout, taskid:%_, wworker:%_, nStartSendTime:%_, nReadWriteTimeOut:%_", first->task.taskid, (void*)first->running_id, first->transfer_profile.start_send_time / 1000, first->transfer_profile.read_write_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpReadWriteTimeout;
        } else if (first->running_id && first->task.long_polling && 0 < first->transfer_profile.start_send_time && 0 == first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.start_send_time >= (uint64_t)first->task.long_polling_timeout) {
            xerror2(TSF"task long-polling timeout, taskid:%_, wworker:%_, nStartSendTime:%_, nLongPollingTimeout:%_", first->task.taskid, (void*)first->running_id, first->transfer_profile.start_send_time / 1000, first->task.long_polling_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpLongPollingTimeout;
        } else if (first->running_id && !first->task.long_polling && 0 < first->transfer_profile.start_send_time && 0 == first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.first_pkg_timeout) {
            xerror2(TSF"task first-pkg timeout taskid:%_, wworker:%_, nStartSendTime:%_, nfirstpkgtimeout:%_", first->task.taskid, (void*)first->running_id, first->transfer_profile.start_send_time / 1000, first->transfer_profile.first_pkg_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpFirstPkgTimeout;
        } else if (first->running_id && 0 < first->transfer_profile.start_send_time && 0 < first->transfer_profile.last_receive_pkg_time &&
                cur_time - first->transfer_profile.last_receive_pkg_time >= ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval)) {
            xerror2(TSF"task pkg-pkg timeout, taskid:%_, wworker:%_, nLastRecvTime:%_, pkg-pkg timeout:%_",
                    first->task.taskid, (void*)first->running_id, first->transfer_profile.last_receive_pkg_time / 1000, ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval) / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpPkgPkgTimeout;
        } else {
            // pass
        }

        if (0 != socket_timeout_code) {
            std::string ip = first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile().ip : "";
            std::string host = first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile().host : "";
            int port = first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile().port : 0;
            dynamic_timeout_.CgiTaskStatistic(first->task.cgi, kDynTimeTaskFailedPkgLen, 0);
            __SetLastFailedStatus(first);
            __SingleRespHandle(first, err_type, socket_timeout_code, err_type == kEctLocal ? kTaskFailHandleTaskTimeout : kTaskFailHandleDefault, 0, first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile() : ConnectProfile());
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, err_type, socket_timeout_code, ip, host, port);
        }

        first = next;
    }
}

void ShortLinkTaskManager::__RunOnStartTask() {
    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    uint64_t curtime = ::gettickcount();
    int sent_count = 0;

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (first->running_id) {
            ++sent_count;
            first = next;
            continue;
        }

        //重试间隔
        if (first->retry_time_interval > curtime - first->retry_start_time) {
            xdebug2(TSF"retry interval, taskid:%0, task retry late task, wait:%1", first->task.taskid, (curtime - first->transfer_profile.loop_start_task_time) / 1000);
            first = next;
            continue;
        }

        Task task = first->task;
        if (get_real_host_) {
            get_real_host_(task.user_id, task.shortlink_host_list);
        }
        std::string host = task.shortlink_host_list.front();
        xinfo2(TSF"host ip to callback is %_ ",host);

        xinfo2(TSF"need auth cgi %_ , host %_ need auth %_ , long-polling %_", first->task.cgi, host, first->task.need_authed, first->task.long_polling);
        // make sure login
        if (first->task.need_authed) {
            bool ismakesureauthsuccess = MakesureAuthed(host, first->task.user_id);
            xinfo2(TSF"auth result %_ host %_", ismakesureauthsuccess, host);

            if (!ismakesureauthsuccess) {
                xinfo2_if(curtime % 3 == 1, TSF"makeSureAuth retsult=%0", ismakesureauthsuccess);
                first = next;
                continue;
            }
        }

        AutoBuffer bufreq;
        AutoBuffer buffer_extension;
        int error_code = 0;

        if (!Req2Buf(first->task.taskid, first->task.user_context, first->task.user_id, bufreq, buffer_extension, error_code, Task::kChannelShort, host)) {
            __SingleRespHandle(first, kEctEnDecode, error_code, kTaskFailHandleTaskEnd, 0, first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile() : ConnectProfile());
            first = next;
            continue;
        }

        //雪崩检测
        xassert2(fun_anti_avalanche_check_);

        if (!fun_anti_avalanche_check_(first->task, bufreq.Ptr(), (int)bufreq.Length())) {
            __SingleRespHandle(first, kEctLocal, kEctLocalAntiAvalanche, kTaskFailHandleTaskEnd, 0, first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile() : ConnectProfile());
            first = next;
            continue;
        }

        first->transfer_profile.loop_start_task_time = ::gettickcount();
        first->transfer_profile.first_pkg_timeout = __FirstPkgTimeout(first->task.server_process_cost, bufreq.Length(), sent_count, dynamic_timeout_.GetStatus());
        first->current_dyntime_status = (first->task.server_process_cost <= 0) ? dynamic_timeout_.GetStatus() : kEValuating;
        if (first->transfer_profile.task.long_polling) {
            xinfo2(TSF"this task is long-polling %_ ", first->transfer_profile.task.cgi);
            first->transfer_profile.read_write_timeout = __ReadWriteTimeout(first->transfer_profile.task.long_polling_timeout);
        } else {
            xinfo2(TSF"this task is not long-polling %_ ", first->transfer_profile.task.cgi);
            first->transfer_profile.read_write_timeout = __ReadWriteTimeout(first->transfer_profile.first_pkg_timeout);
        }
        first->transfer_profile.send_data_size = bufreq.Length();

        first->use_proxy =  (first->remain_retry_count == 0 && first->task.retry_count > 0) ? !default_use_proxy_ : default_use_proxy_;
        ShortLinkInterface* worker = ShortLinkChannelFactory::Create(MessageQueue::Handler2Queue(asyncreg_.Get()), net_source_, first->task, first->use_proxy);
        worker->OnSend.set(boost::bind(&ShortLinkTaskManager::__OnSend, this, _1), worker, AYNC_HANDLER);
        worker->OnRecv.set(boost::bind(&ShortLinkTaskManager::__OnRecv, this, _1, _2, _3), worker, AYNC_HANDLER);
        worker->OnResponse.set(boost::bind(&ShortLinkTaskManager::__OnResponse, this, _1, _2, _3, _4, _5, _6, _7), worker, AYNC_HANDLER);
        worker->GetCacheSocket = boost::bind(&ShortLinkTaskManager::__OnGetCacheSocket, this, _1);
        first->running_id = (intptr_t)worker;

        xassert2(worker && first->running_id);
        if (!first->running_id) {
            xwarn2(TSF"task add into shortlink readwrite fail cgi:%_, cmdid:%_, taskid:%_", first->task.cgi, first->task.cmdid, first->task.taskid);
            first = next;
            continue;
        }

        worker->func_network_report.set(fun_notify_network_err_);
        worker->SendRequest(bufreq, buffer_extension);

        xinfo2(TSF"task add into shortlink readwrite cgi:%_, cmdid:%_, taskid:%_, work:%_, size:%_, timeout(firstpkg:%_, rw:%_, task:%_), retry:%_, long-polling:%_, useProxy:%_",
               first->task.cgi, first->task.cmdid, first->task.taskid, (ShortLinkInterface*)first->running_id, first->transfer_profile.send_data_size, first->transfer_profile.first_pkg_timeout / 1000,
               first->transfer_profile.read_write_timeout / 1000, first->task_timeout / 1000, first->remain_retry_count, first->task.long_polling, first->use_proxy);
        ++sent_count;
        first = next;
    }
}

struct find_seq {
  public:
    bool operator()(const TaskProfile& _value) {return p_worker == (ShortLinkInterface*)_value.running_id;}

  public:
    ShortLinkInterface* p_worker;
};

void ShortLinkTaskManager::__OnResponse(ShortLinkInterface* _worker, ErrCmdType _err_type, int _status, AutoBuffer& _body, AutoBuffer& _extension, bool _cancel_retry, ConnectProfile& _conn_profile) {

    xdebug2(TSF"worker=%0, _err_type=%1, _status=%2, _body.lenght=%3, _cancel_retry=%4", _worker, _err_type, _status, _body.Length(), _cancel_retry);

    fun_shortlink_response_(_status);

    std::list<TaskProfile>::iterator it = __LocateBySeq((intptr_t)_worker);    // must used iter pWorker, not used aSelf. aSelf may be destroy already

    if (lst_cmd_.end() == it) {
        xerror2(TSF"task no found: status:%_, worker:%_", _status, _worker);
        return;
    }
    
    if(_worker->IsKeepAlive() && _conn_profile.socket_fd != INVALID_SOCKET) {
        if(_err_type != kEctOK) {
            socket_close(_conn_profile.socket_fd);
            socket_pool_.Report(_conn_profile.is_reused_fd, false, false);
        } else if(_conn_profile.ip_index >=0 && _conn_profile.ip_index < (int)_conn_profile.ip_items.size()) {
            IPPortItem item = _conn_profile.ip_items[_conn_profile.ip_index];
            CacheSocketItem cache_item(item, _conn_profile.socket_fd, _conn_profile.keepalive_timeout);
            if(!socket_pool_.AddCache(cache_item)) {
                socket_close(cache_item.socket_fd);
            }
        } else {
            xassert2(false, "not match");
        }
    }

    if (_err_type != kEctOK) {
        if (_err_type == kEctSocket && _status == kEctSocketMakeSocketPrepared) {
            dynamic_timeout_.CgiTaskStatistic(it->task.cgi, kDynTimeTaskFailedPkgLen, 0);
            __SetLastFailedStatus(it);
        }

        if (_err_type == kEctSocket) {
            it->force_no_retry = _cancel_retry;
        }
        __SingleRespHandle(it, _err_type, _status, kTaskFailHandleDefault, _body.Length(), _conn_profile);
        return;

    }

    it->transfer_profile.received_size = _body.Length();
    it->transfer_profile.receive_data_size = _body.Length();
    it->transfer_profile.last_receive_pkg_time = ::gettickcount();

    int err_code = 0;
    int handle_type = Buf2Resp(it->task.taskid, it->task.user_context, it->task.user_id, _body, _extension, err_code, Task::kChannelShort);
    xinfo2(TSF"err_code %_ ",err_code);
    socket_pool_.Report(_conn_profile.is_reused_fd, true, handle_type==kTaskFailHandleNoError);

    switch(handle_type){
        case kTaskFailHandleNoError:
        {
            dynamic_timeout_.CgiTaskStatistic(it->task.cgi, (unsigned int)it->transfer_profile.send_data_size + (unsigned int)_body.Length(), ::gettickcount() - it->transfer_profile.start_send_time);
            __SingleRespHandle(it, kEctOK, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, kEctOK, err_code, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
        }
            break;
        case kTaskFailHandleSessionTimeout:
        {
            xassert2(fun_notify_retry_all_tasks);
            xwarn2(TSF"task decode error session timeout taskid:%_, cmdid:%_, cgi:%_", it->task.taskid, it->task.cmdid, it->task.cgi);
            fun_notify_retry_all_tasks(kEctEnDecode, err_code, handle_type, it->task.taskid, it->task.user_id);
        }
            break;
        case kTaskFailHandleRetryAllTasks:
        {
            xassert2(fun_notify_retry_all_tasks);
            xwarn2(TSF"task decode error retry all task taskid:%_, cmdid:%_, cgi:%_", it->task.taskid, it->task.cmdid, it->task.cgi);
            fun_notify_retry_all_tasks(kEctEnDecode, err_code, handle_type, it->task.taskid, it->task.user_id);
        }
            break;
        case kTaskFailHandleTaskEnd:
        {
            __SingleRespHandle(it, kEctEnDecode, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
        }
            break;
        case kTaskFailHandleDefault:
        {
            xerror2(TSF"task decode error handle_type:%_, err_code:%_, pWorker:%_, taskid:%_ body dump:%_", handle_type, err_code, (void*)it->running_id, it->task.taskid, xdump(_body.Ptr(), _body.Length()));
            __SingleRespHandle(it, kEctEnDecode, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, kEctEnDecode, handle_type, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
        }
            break;
        default:
        {
            xassert2(false, TSF"task decode error fail_handle:%_, taskid:%_, context id:%_", handle_type, it->task.taskid, it->task.user_id);
            __SingleRespHandle(it, kEctEnDecode, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, kEctEnDecode, handle_type, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
            break;
        }

    }
}

void ShortLinkTaskManager::__OnSend(ShortLinkInterface* _worker) {
    
    std::list<TaskProfile>::iterator it = __LocateBySeq((intptr_t)_worker);

    if (lst_cmd_.end() != it) {
        if (it->transfer_profile.first_start_send_time == 0)
            it->transfer_profile.first_start_send_time = ::gettickcount();
        it->transfer_profile.start_send_time = ::gettickcount();
        xdebug2(TSF"taskid:%_, worker:%_, nStartSendTime:%_", it->task.taskid, _worker, it->transfer_profile.start_send_time / 1000);
    }
}

void ShortLinkTaskManager::__OnRecv(ShortLinkInterface* _worker, unsigned int _cached_size, unsigned int _total_size) {

    xverbose_function();
    std::list<TaskProfile>::iterator it = __LocateBySeq((intptr_t)_worker);

    if (lst_cmd_.end() != it) {
        if(it->transfer_profile.last_receive_pkg_time == 0)
            WeakNetworkLogic::Singleton::Instance()->OnPkgEvent(true, (int)(::gettickcount() - it->transfer_profile.start_send_time));
        else
            WeakNetworkLogic::Singleton::Instance()->OnPkgEvent(false, (int)(::gettickcount() - it->transfer_profile.last_receive_pkg_time));
        it->transfer_profile.last_receive_pkg_time = ::gettickcount();
        it->transfer_profile.received_size = _cached_size;
        it->transfer_profile.receive_data_size = _total_size;
        xdebug2(TSF"worker:%_, last_recvtime:%_, cachedsize:%_, totalsize:%_", _worker, it->transfer_profile.last_receive_pkg_time / 1000, _cached_size, _total_size);
    } else {
        xwarn2(TSF"not found worker:%_", _worker);
    }
}

void ShortLinkTaskManager::RedoTasks() {
    xinfo_function();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        first->last_failed_dyntime_status = 0;

        if (first->running_id) {
            xinfo2(TSF "task redo, taskid:%_", first->task.taskid);
            __SingleRespHandle(first, kEctLocal, kEctLocalCancel, kTaskFailHandleDefault, 0, ((ShortLinkInterface*)first->running_id)->Profile());
        }

        first = next;
    }
    
    socket_pool_.Clear();
    __RunLoop();
}

void ShortLinkTaskManager::RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid) {
    xverbose_function();
    xinfo2(TSF"RetryTasks taskid %_ ", _src_taskid);
    __BatchErrorRespHandle(_err_type, _err_code, _fail_handle, _src_taskid);
    __RunLoop(); 
}

void ShortLinkTaskManager::__BatchErrorRespHandle(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, bool _callback_runing_task_only) {
    xassert2(kEctOK != _err_type);
    xdebug2(TSF"ect=%0, errcode=%1 taskid:=%2", _err_type, _err_code, _src_taskid);

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;
        
        if (_callback_runing_task_only && !first->running_id) {
            first = next;
            continue;
        }
        
        if (_fail_handle == kTaskFailHandleSessionTimeout && !first->task.need_authed) {
            first = next;
            continue;
        }

        xinfo2(TSF"axauth sessiontime id %_, cgi %_ taskid %_", _src_taskid, first->task.cgi, first->task.taskid);

        if (_fail_handle == kTaskFailHandleSessionTimeout && _src_taskid != 0 && first->task.taskid == _src_taskid && first->allow_sessiontimeout_retry) { //retry task when sessiontimeout
            xinfo2(TSF"axauth to timeout queue %_, cgi %_ ", first->task.taskid, first->task.cgi);
            first->allow_sessiontimeout_retry = false;
            first->remain_retry_count++;
            __DeleteShortLink(first->running_id);
            first->PushHistory();
            first->InitSendParam();
            first = next;
            continue;
        }
        
        if (_src_taskid == Task::kInvalidTaskID || _src_taskid == first->task.taskid)
            __SingleRespHandle(first, _err_type, _err_code, _fail_handle, 0, first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile() : ConnectProfile());
        else
            __SingleRespHandle(first, _err_type, 0, _fail_handle, 0, first->running_id ? ((ShortLinkInterface*)first->running_id)->Profile() : ConnectProfile());

        first = next;
    }
}

bool ShortLinkTaskManager::__SingleRespHandle(std::list<TaskProfile>::iterator _it, ErrCmdType _err_type, int _err_code, int _fail_handle, size_t _resp_length, const ConnectProfile& _connect_profile) {
    xverbose_function();
    xassert2(kEctServer != _err_type);
    xassert2(_it != lst_cmd_.end());
    
    if(_it == lst_cmd_.end()) return false;

    if (kEctOK == _err_type) {
        tasks_continuous_fail_count_ = 0;
        default_use_proxy_ = _it->use_proxy;
    } else {
        ++tasks_continuous_fail_count_;
    }

    uint64_t curtime =  gettickcount();
    _it->transfer_profile.connect_profile = _connect_profile;
    
    xassert2((kEctOK == _err_type) == (kTaskFailHandleNoError == _fail_handle), TSF"type:%_, handle:%_", _err_type, _fail_handle);

    if (_it->force_no_retry || 0 >= _it->remain_retry_count || kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle || kTaskFailHandleTaskTimeout == _fail_handle) {
        xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end callback short cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
        (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
        (TSF"cli(%_, %_, n:%_, sig:%_), ", _it->transfer_profile.external_ip, _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal)
        (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, 0 != _resp_length ? _resp_length : _it->transfer_profile.receive_data_size, 0 != _resp_length ? "" : "/",
                0 != _resp_length ? "" : string_cast(_it->transfer_profile.received_size).str(), _connect_profile.conn_rtt, (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time),
                        (curtime - _it->start_task_time), _it->remain_retry_count)
        (TSF"cgi:%_, taskid:%_, worker:%_, context id:%_", _it->task.cgi, _it->task.taskid, (ShortLinkInterface*)_it->running_id, _it->task.user_id);

        if(_err_type != kEctOK && _err_type != kEctServer) {
            xinfo_trace(TSF"cgi trace error: (%_, %_), cost:%_, rtt:%_, svr:(%_, %_, %_)", _err_type, _err_code, (curtime - _it->start_task_time), _connect_profile.conn_rtt,
                        _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type]);
        }

        if (task_connection_detail_) {
            task_connection_detail_(_err_type, _err_code, _connect_profile.ip_index);
        }

        int cgi_retcode = fun_callback_(_err_type, _err_code, _fail_handle, _it->task, (unsigned int)(curtime - _it->start_task_time));
        int errcode = _err_code;

        if (_it->running_id) {
            if (kEctOK == _err_type) {
                errcode = cgi_retcode;
            }
        }

        _it->end_task_time = ::gettickcount();
        _it->err_type = _err_type;
        _it->transfer_profile.error_type = _err_type;
        _it->err_code = errcode;
        _it->transfer_profile.error_code = _err_code;
        _it->PushHistory();
        ReportTaskProfile(*_it);
        WeakNetworkLogic::Singleton::Instance()->OnTaskEvent(*_it);

        __DeleteShortLink(_it->running_id);

        lst_cmd_.erase(_it);

        return true;
    }


    xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end retry short cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
    (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
    (TSF"cli(%_, n:%_, sig:%_), ", _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal)
    (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, 0 != _resp_length ? _resp_length : _it->transfer_profile.received_size,
            0 != _resp_length ? "" : "/", 0 != _resp_length ? "" : string_cast(_it->transfer_profile.receive_data_size).str(), _connect_profile.conn_rtt,
                    (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time), (curtime - _it->start_task_time), _it->remain_retry_count)
    (TSF"cgi:%_, taskid:%_, worker:%_", _it->task.cgi, _it->task.taskid,(void*) _it->running_id);

    _it->remain_retry_count--;
    _it->transfer_profile.error_type = _err_type;
    _it->transfer_profile.error_code = _err_code;

    __DeleteShortLink(_it->running_id);
    _it->PushHistory();
    _it->InitSendParam();

    _it->retry_start_time = ::gettickcount();
    // session timeout 应该立刻重试
    if (kTaskFailHandleSessionTimeout == _fail_handle) {
        _it->retry_start_time = 0;
    }

    _it->retry_time_interval = DEF_TASK_RETRY_INTERNAL;

    return false;
}

std::list<TaskProfile>::iterator ShortLinkTaskManager::__LocateBySeq(intptr_t _running_id) {
    if (!_running_id) return lst_cmd_.end();

    find_seq find_functor;
    find_functor.p_worker = (ShortLinkInterface*)_running_id;
    std::list<TaskProfile>::iterator it = std::find_if(lst_cmd_.begin(), lst_cmd_.end(), find_functor);

    return it;
}

void ShortLinkTaskManager::__DeleteShortLink(intptr_t& _running_id) {
    if (!_running_id) return;
    ShortLinkInterface* p_shortlink = (ShortLinkInterface*)_running_id;
    ShortLinkChannelFactory::Destory(p_shortlink);
    MessageQueue::CancelMessage(asyncreg_.Get(), p_shortlink);
    p_shortlink = NULL;
}

ConnectProfile ShortLinkTaskManager::GetConnectProfile(uint32_t _taskid) const{
    std::list<TaskProfile>::const_iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::const_iterator last = lst_cmd_.end();
    
    while (first != last) {
        if (_taskid == first->task.taskid) {
            return ((ShortLinkInterface*)(first->running_id))->Profile();
        }
        ++first;
    }
    return ConnectProfile();
}

SOCKET ShortLinkTaskManager::__OnGetCacheSocket(const IPPortItem& _address) {
    return socket_pool_.GetSocket(_address);
}

