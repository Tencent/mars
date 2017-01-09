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

#include "boost/bind.hpp"

#include "mars/app/app.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/copy_wrapper.h"
#include "mars/comm/platform_comm.h"
#ifdef ANDROID
#include "mars/comm/android/wakeuplock.h"
#endif

#include "dynamic_timeout.h"
#include "net_channel_factory.h"

using namespace mars::stn;
using namespace mars::app;

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_SHORTLINK_SYNC2ASYNC_FUNC_TITLE(func, title) RETURN_SYNC2ASYNC_FUNC_TITLE(func, title, )

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
    xinfo_function();
    xinfo2(TSF"ShortLinkTaskManager messagequeue_id=%_", MessageQueue::Handler2Queue(asyncreg_.Get()));
}

ShortLinkTaskManager::~ShortLinkTaskManager() {
    xinfo_function();
    asyncreg_.CancelAndWait();
    xinfo2(TSF"lst_cmd_ count=%0", lst_cmd_.size());
    __Reset();
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

void ShortLinkTaskManager::OnSessionTimeout(int _err_code, uint32_t _src_taskid) {
    xverbose_function();
    __BatchErrorRespHandle(kEctEnDecode, _err_code, kTaskFailHandleSessionTimeout, _src_taskid);
    __RunLoop(); 
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
        wakeup_lock_->Lock(30 * 1000);
#endif
        MessageQueue::FasterMessage(asyncreg_.Get(),
                                    MessageQueue::Message((MessageQueue::MessageTitle_t)this, boost::bind(&ShortLinkTaskManager::__RunLoop, this)),
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
    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    uint64_t cur_time = ::gettickcount();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        ErrCmdType err_type = kEctLocal;
        int socket_timeout_code = 0;

        if (cur_time - first->start_task_time >= first->task_timeout) {
        	err_type = kEctLocal;
        	socket_timeout_code = kEctLocalTaskTimeout;
        } else if (first->running_id && 0 < first->transfer_profile.start_send_time && cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.read_write_timeout) {
            xerror2(TSF"task read-write timeout, taskid:%_, wworker:%_, nStartSendTime:%_, nReadWriteTimeOut:%_", first->task.taskid, (void*)first->running_id, first->transfer_profile.start_send_time / 1000, first->transfer_profile.read_write_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpReadWriteTimeout;
        } else if (first->running_id && 0 < first->transfer_profile.start_send_time && 0 == first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.first_pkg_timeout) {
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

    bool ismakesureauthruned = false;
    bool ismakesureauthsuccess = false;
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

        // make sure login
        if (first->task.need_authed) {

            if (!ismakesureauthruned) {
                ismakesureauthruned = true;
                ismakesureauthsuccess = MakesureAuthed();
            }

            if (!ismakesureauthsuccess) {
                xinfo2_if(curtime % 3 == 1, TSF"makeSureAuth retsult=%0", ismakesureauthsuccess);
                first = next;
                continue;
            }
        }

        AutoBuffer bufreq;
        int error_code = 0;

        if (!Req2Buf(first->task.taskid, first->task.user_context, bufreq, error_code, Task::kChannelShort)) {
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
		first->transfer_profile.read_write_timeout = __ReadWriteTimeout(first->transfer_profile.first_pkg_timeout);
		first->transfer_profile.send_data_size = bufreq.Length();

        first->use_proxy =  (first->remain_retry_count == 0 && first->task.retry_count > 0) ? !default_use_proxy_ : default_use_proxy_;
        ShortLinkInterface* worker = ShortLinkChannelFactory::Create(MessageQueue::Handler2Queue(asyncreg_.Get()), net_source_, first->task.shortlink_host_list, first->task.cgi, first->task.taskid, first->use_proxy);
        worker->OnSend = boost::bind(&ShortLinkTaskManager::__OnSend, this, _1);
        worker->OnRecv = boost::bind(&ShortLinkTaskManager::__OnRecv, this, _1, _2, _3);
        worker->OnResponse = boost::bind(&ShortLinkTaskManager::__OnResponse, this, _1, _2, _3, _4, _5, _6);
        first->running_id = (intptr_t)worker;

        xassert2(worker && first->running_id);
        if (!first->running_id) {
			xwarn2(TSF"task add into shortlink readwrite fail cgi:%_, cmdid:%_, taskid:%_", first->task.cgi, first->task.cmdid, first->task.taskid);
			first = next;
			continue;
		}

        worker->func_network_report = fun_notify_network_err_;
        worker->SendRequest(bufreq);

        xinfo2(TSF"task add into shortlink readwrite cgi:%_, cmdid:%_, taskid:%_, work:%_, size:%_, timeout(firstpkg:%_, rw:%_, task:%_), retry:%_, useProxy:%_",
               first->task.cgi, first->task.cmdid, first->task.taskid, (ShortLinkInterface*)first->running_id, first->transfer_profile.send_data_size, first->transfer_profile.first_pkg_timeout / 1000,
			   first->transfer_profile.read_write_timeout / 1000, first->task_timeout / 1000, first->remain_retry_count, first->use_proxy);
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

void ShortLinkTaskManager::__OnResponse(ShortLinkInterface* _worker, ErrCmdType _err_type, int _status, AutoBuffer& _body, bool _cancel_retry, ConnectProfile& _conn_profile) {
    copy_wrapper<AutoBuffer> body(_body);
    RETURN_SHORTLINK_SYNC2ASYNC_FUNC_TITLE(boost::bind(&ShortLinkTaskManager::__OnResponse, this, _worker, _err_type, _status, body, _cancel_retry, _conn_profile), _worker);

    xdebug2(TSF"worker=%0, _err_type=%1, _status=%2, _body.lenght=%3, _cancel_retry=%4", _worker, _err_type, _status, body.get().Length(), _cancel_retry);

    fun_shortlink_response_(_status);

    std::list<TaskProfile>::iterator it = __LocateBySeq((intptr_t)_worker);    // must used iter pWorker, not used aSelf. aSelf may be destroy already

    if (lst_cmd_.end() == it) {
    		xerror2(TSF"task no found task:%0, status:%1, worker:%2", _status, _worker);
		return;
    }

    if (_err_type != kEctOK) {
        if (_err_type == kEctSocket && _status == kEctSocketMakeSocketPrepared) {
            dynamic_timeout_.CgiTaskStatistic(it->task.cgi, kDynTimeTaskFailedPkgLen, 0);
            __SetLastFailedStatus(it);
        }
        __SingleRespHandle(it, _err_type, _status, kTaskFailHandleDefault, body.get().Length(), _conn_profile);
        return;

    }

    it->transfer_profile.received_size = body->Length();
	it->transfer_profile.receive_data_size = body->Length();
	it->transfer_profile.last_receive_pkg_time = ::gettickcount();
	if (_cancel_retry) {
		it->remain_retry_count > 0 ? it->remain_retry_count-- : it->remain_retry_count;
	}

	int err_code = 0;
	int handle_type = Buf2Resp(it->task.taskid, it->task.user_context, body, err_code, Task::kChannelShort);

	switch(handle_type){
		case kTaskFailHandleNoError:
		{
			dynamic_timeout_.CgiTaskStatistic(it->task.cgi, (unsigned int)it->transfer_profile.send_data_size + (unsigned int)body.get().Length(), ::gettickcount() - it->transfer_profile.start_send_time);
			__SingleRespHandle(it, kEctOK, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
			xassert2(fun_notify_network_err_);
			fun_notify_network_err_(__LINE__, kEctOK, err_code, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
		}
			break;
		case kTaskFailHandleSessionTimeout:
		{
			xwarn2(TSF"task decode error session timeout taskid:%_, cmdid:%_, cgi:%_", it->task.taskid, it->task.cmdid, it->task.cgi);
			fun_notify_session_timeout_(err_code, it->task.taskid);
		}
			break;
		case kTaskFailHandleTaskEnd:
		{
			__SingleRespHandle(it, kEctEnDecode, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
		}
			break;
		case kTaskFailHandleDefault:
		{
			xerror2(TSF"task decode error handle_type:%_, err_code:%_, pWorker:%_, taskid:%_ body dump:%_", handle_type, err_code, (void*)it->running_id, it->task.taskid, xdump(body->Ptr(), body->Length()));
			__SingleRespHandle(it, kEctEnDecode, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
			xassert2(fun_notify_network_err_);
			fun_notify_network_err_(__LINE__, kEctEnDecode, handle_type, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
		}
			break;
		default:
		{
			xassert2(false, TSF"task decode error fail_handle:%_, taskid:%_", handle_type, it->task.taskid);
			__SingleRespHandle(it, kEctEnDecode, err_code, handle_type, (unsigned int)it->transfer_profile.receive_data_size, _conn_profile);
			xassert2(fun_notify_network_err_);
			fun_notify_network_err_(__LINE__, kEctEnDecode, handle_type, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
			break;
		}

	}
}

void ShortLinkTaskManager::__OnSend(ShortLinkInterface* _worker) {
    RETURN_SHORTLINK_SYNC2ASYNC_FUNC_TITLE(boost::bind(&ShortLinkTaskManager::__OnSend, this, _worker), _worker);

    std::list<TaskProfile>::iterator it = __LocateBySeq((intptr_t)_worker);

    if (lst_cmd_.end() != it) {
    	if (it->transfer_profile.first_start_send_time == 0)
    	    		it->transfer_profile.first_start_send_time = ::gettickcount();
        it->transfer_profile.start_send_time = ::gettickcount();
        xdebug2(TSF"taskid:%_, worker:%_, nStartSendTime:%_", it->task.taskid, _worker, it->transfer_profile.start_send_time / 1000);
    }
}

void ShortLinkTaskManager::__OnRecv(ShortLinkInterface* _worker, unsigned int _cached_size, unsigned int _total_size) {
    RETURN_SHORTLINK_SYNC2ASYNC_FUNC_TITLE(boost::bind(&ShortLinkTaskManager::__OnRecv, this, _worker, _cached_size, _total_size), _worker);

    xverbose_function();
    std::list<TaskProfile>::iterator it = __LocateBySeq((intptr_t)_worker);

    if (lst_cmd_.end() != it) {
        it->transfer_profile.last_receive_pkg_time = ::gettickcount();
        it->transfer_profile.received_size = _cached_size;
        it->transfer_profile.receive_data_size = _total_size;
        xdebug2(TSF"worker:%_, last_recvtime:%_, cachedsize:%_, totalsize:%_", _worker, it->transfer_profile.last_receive_pkg_time / 1000, _cached_size, _total_size);
    } else {
        xwarn2(TSF"not found worker:%_", _worker);
    }
}

void ShortLinkTaskManager::RedoTasks() {
    xdebug_function();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (first->running_id)    __DeleteShortLink(first->running_id);

        first->InitSendParam();
        first->last_failed_dyntime_status = 0;

        first = next;
    }
    
    __RunLoop();
}

void ShortLinkTaskManager::__Reset() {
    xinfo_function();
    __BatchErrorRespHandle(kEctLocal, kEctLocalReset, kTaskFailHandleTaskEnd, 0);
}

void ShortLinkTaskManager::__BatchErrorRespHandle(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid) {
    xassert2(kEctOK != _err_type);
    xdebug2(TSF"ect=%0, errcode=%1", _err_type, _err_code);

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (_src_taskid == first->task.taskid)
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

    if (kEctOK == _err_type) {
        tasks_continuous_fail_count_ = 0;
        default_use_proxy_ = _it->use_proxy;
    } else {
        ++tasks_continuous_fail_count_;
    }

    uint64_t curtime =  gettickcount();
    _it->transfer_profile.connect_profile = _connect_profile;
    
    xassert2((kEctOK == _err_type) == (kTaskFailHandleNoError == _fail_handle), TSF"type:%_, handle:%_", _err_type, _fail_handle);

    if (0 >= _it->remain_retry_count || kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle || kTaskFailHandleTaskTimeout == _fail_handle) {
        xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end callback short cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
        (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
        (TSF"cli(%_, %_, n:%_, sig:%_), ", _it->transfer_profile.external_ip, _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal)
        (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, 0 != _resp_length ? _resp_length : _it->transfer_profile.receive_data_size, 0 != _resp_length ? "" : "/",
        		0 != _resp_length ? "" : TVariant(_it->transfer_profile.received_size).ToString(), _connect_profile.conn_rtt, (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time),
        				(curtime - _it->start_task_time), _it->remain_retry_count)
        (TSF"cgi:%_, taskid:%_, worker:%_", _it->task.cgi, _it->task.taskid, (ShortLinkInterface*)_it->running_id);

        int cgi_retcode = fun_callback_(_err_type, _err_code, _fail_handle, _it->task, (unsigned int)(curtime - _it->start_task_time));
        int errcode = _err_code;

        if (_it->running_id) {
            if (kEctOK == _err_type) {
                errcode = (cgi_retcode == 0 ? cgi_retcode : kEctServer);
            }
        }

        _it->end_task_time = ::gettickcount();
        _it->err_type = _err_type;
        _it->err_code = errcode;
        _it->PushHistory();
        ReportTaskProfile(*_it);

        __DeleteShortLink(_it->running_id);

        lst_cmd_.erase(_it);

        return true;
    }


    xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end retry short cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
    (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
    (TSF"cli(%_, n:%_, sig:%_), ", _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal)
    (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, 0 != _resp_length ? _resp_length : _it->transfer_profile.received_size,
    		0 != _resp_length ? "" : "/", 0 != _resp_length ? "" : TVariant(_it->transfer_profile.receive_data_size).ToString(), _connect_profile.conn_rtt,
    				(_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time), (curtime - _it->start_task_time), _it->remain_retry_count)
    (TSF"cgi:%_, taskid:%_, worker:%_", _it->task.cgi, _it->task.taskid,(void*) _it->running_id);

    _it->remain_retry_count--;

    __DeleteShortLink(_it->running_id);
    _it->PushHistory();
    _it->InitSendParam();

    _it->retry_start_time = ::gettickcount();
    // session timeout 应该立刻重试
    if (kTaskFailHandleSessionTimeout == _err_code) {
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

