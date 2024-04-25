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
#include "mars/stn/stn_manager.h"

using namespace mars::stn;
using namespace mars::comm;

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_LONKLINK_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )
#define RETURN_LONKLINK_SYNC2ASYNC_FUNC_TITLE(func, title) RETURN_SYNC2ASYNC_FUNC_TITLE(func, title, )

//boost::function<size_t (const std::string& _user_id, std::vector<std::string>& _host_list, bool _strict_match)> LongLinkTaskManager::get_real_host_;
//boost::function<void (uint32_t _version, mars::stn::TlsHandshakeFrom _from)> LongLinkTaskManager::on_handshake_ready_;
//boost::function<bool (int _error_code)> LongLinkTaskManager::should_intercept_result_;

static int longlink_id = 0;
//std::set<std::string> LongLinkTaskManager::forbid_tls_host_;

LongLinkTaskManager::LongLinkTaskManager(mars::boot::Context* _context, std::shared_ptr<NetSource> _netsource, ActiveLogic& _activelogic, DynamicTimeout& _dynamictimeout, MessageQueue::MessageQueue_t  _messagequeue_id)
    : context_(_context)
    , asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeue_id))
    , lastbatcherrortime_(0)
    , retry_interval_(0)
    , tasks_continuous_fail_count_(0)
    // , longlink_(LongLinkChannelFactory::Create(_messagequeue_id, _netsource))
    // , longlinkconnectmon_(new LongLinkConnectMonitor(_activelogic, *longlink_, _messagequeue_id))
    , dynamic_timeout_(_dynamictimeout)
    , netsource_(_netsource)
    , active_logic_(_activelogic)
#ifdef ANDROID
    , wakeup_lock_(new WakeUpLock())
#endif
#ifndef _WIN32
    , meta_mutex_(true)
#endif
    , cellular_network_manager_(new CellularNetworkManager(context_))
{
    xdebug_function(TSF"mars2");
    xinfo_function(TSF"handler:(%_,%_)", asyncreg_.Get().queue, asyncreg_.Get().seq);
}

LongLinkTaskManager::~LongLinkTaskManager() {
    xinfo_function(TSF "mars2");
    already_release_manager_ = true;
    asyncreg_.CancelAndWait();
    __BatchErrorRespHandle("", kEctLocal, kEctLocalReset, kTaskFailHandleTaskEnd, Task::kInvalidTaskID, false);

    MetaScopedLock lock(meta_mutex_);
    for(auto kv : longlink_metas_){
        ReleaseLongLink(kv.second);
    }
    longlink_metas_.clear();

#ifdef ANDROID
    delete wakeup_lock_;
#endif
}

bool LongLinkTaskManager::StartTask(const Task& _task, int _channel) {
    xverbose_function();
    xdebug2(TSF"taskid=%0", _task.taskid);

    TaskProfile task(_task);
    task.link_type = _channel;

    if(_channel == Task::kChannelMinorLong) {   //already fixed host redirect, getted real host
        xassert2(!_task.minorlong_host_list.empty());
        task.channel_name = _task.minorlong_host_list.empty() ? "" : _task.minorlong_host_list.front();
    } else {
        task.channel_name = _task.channel_name;
    }

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

            auto longlink = GetLongLink(first->channel_name);
            if(longlink == nullptr) {
                xwarn2(TSF"longlink nullptr name:%_", first->channel_name);
                return false;
            }

	        longlink->Channel()->Stop(first->task.taskid);
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
    MetaScopedLock lock(meta_mutex_);
    for(auto item : longlink_metas_) {
        item.second->Channel()->Disconnect(LongLinkErrCode::kReset);
        MessageQueue::CancelMessage(asyncreg_.Get(), longlink_id_[item.second->Config().name]);
    }
    lock.unlock();
    
    MessageQueue::CancelMessage(asyncreg_.Get(), 0);
    lst_cmd_.clear();
}

unsigned int LongLinkTaskManager::GetTaskCount(const std::string& _name) {
    unsigned int count = 0;
    for(auto item : lst_cmd_) {
        if(item.channel_name == _name)
            count += 1;
    }
    return count;
}

unsigned int LongLinkTaskManager::GetTasksContinuousFailCount() {
    return tasks_continuous_fail_count_;
}

void LongLinkTaskManager::TouchTasks() {
    __RunLoop();
}

void LongLinkTaskManager::RedoTasks() {
    xinfo_function();
    MetaScopedLock lock(meta_mutex_);
    for(auto longlink : longlink_metas_) {
        longlink.second->Checker()->CancelConnect();
        longlink.second->Channel()->Disconnect(LongLinkErrCode::kReset);
        longlink.second->Channel()->SvrTrigOff();
        longlink.second->Channel()->MakeSureConnected();
    }

    __RedoTasks("", false);
}

void LongLinkTaskManager::__RedoTasks(const std::string& _name, bool need_lock_link) {
    xinfo_function(TSF"channel name:%_", _name);

    if (lst_cmd_.empty()) {
        xerror2(TSF"task list is empty!");
    }
    
    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();
    
    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;
        
        xinfo2(TSF"task channel name:%_, cgi:%_", first->channel_name, first->task.cgi);
        if(!_name.empty() && first->channel_name != _name) {
            first = next;
            continue;
        }
        first->last_failed_dyntime_status = 0;
        
        auto link_name = (_name.empty() ? first->channel_name : _name);
        auto longlink = need_lock_link ? GetLongLink(link_name) : GetLongLinkNoLock(link_name);
        if (!longlink){
            xerror2(TSF"didn't find longlink or running id is empty, task channel name:%_, running id:%_", first->channel_name, first->running_id);
            __DumpLongLinkChannelInfo();
        }
        if (first->running_id) {
            xinfo2(TSF "task redo, taskid:%_", first->task.taskid);
            __SingleRespHandle(first, kEctLocal, kEctLocalCancel, kTaskFailHandleDefault, __GetConnectionProfile(longlink));
        }
        
        first = next;
    }
    
    retry_interval_ = 0;
    
    if (_name.empty()) {
        MessageQueue::CancelMessage(asyncreg_.Get());
    } else {
        MessageQueue::CancelMessage(asyncreg_.Get(), longlink_id_[_name]);
    }
    __RunLoop();
}

void LongLinkTaskManager::RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id) {
    xverbose_function();
    __BatchErrorRespHandleByUserId(_user_id, _err_type, _err_code, _fail_handle, _src_taskid);
    __RunLoop();
}


void LongLinkTaskManager::__RunLoop() {
    xinfo_function();
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
                                  MessageQueue::Message((MessageQueue::MessageTitle_t)this, boost::bind(&LongLinkTaskManager::__RunLoop, this), "LongLinkTaskManager::__RunLoop"),
                                  MessageQueue::MessageTiming(1000));
    } else {
#ifdef ANDROID
        /*cancel the last wakeuplock*/
        wakeup_lock_->Lock(500);
#endif
    }
}

void LongLinkTaskManager::__RunOnTimeout() {
    xdebug_function();

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    uint64_t cur_time = ::gettickcount();
    std::map<std::string, std::pair<int, uint32_t> > batchMap;

    while (first != last) {
        int socket_timeout_code = 0;
        uint32_t src_taskid = Task::kInvalidTaskID;

        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (first->running_id && 0 < first->transfer_profile.start_send_time) {
            if (0 == first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.first_pkg_timeout) {
                xerror2(TSF"task first-pkg timeout taskid:%_,  nStartSendTime=%_, nfirstpkgtimeout=%_",
                        first->task.taskid, first->transfer_profile.start_send_time / 1000, first->transfer_profile.first_pkg_timeout / 1000);
                socket_timeout_code = kEctLongFirstPkgTimeout;
                src_taskid = first->task.taskid;
                batchMap[first->channel_name] = std::make_pair(socket_timeout_code, src_taskid);
                __SetLastFailedStatus(first);
            }

            if (0 < first->transfer_profile.last_receive_pkg_time && cur_time - first->transfer_profile.last_receive_pkg_time >= ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval)) {
                xerror2(TSF"task pkg-pkg timeout, taskid:%_, nLastRecvTime=%_, pkg-pkg timeout=%_",
                        first->task.taskid, first->transfer_profile.last_receive_pkg_time / 1000, ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval) / 1000);
                socket_timeout_code = kEctLongPkgPkgTimeout;
                src_taskid = first->task.taskid;
                batchMap[first->channel_name] = std::make_pair(socket_timeout_code, src_taskid);
            }
            
            if (cur_time - first->transfer_profile.start_send_time >= first->transfer_profile.read_write_timeout) {
                xerror2(TSF"task read-write timeout, taskid:%_, , nStartSendTime=%_, nReadWriteTimeOut=%_",
                        first->task.taskid, first->transfer_profile.start_send_time / 1000, first->transfer_profile.read_write_timeout / 1000);
                socket_timeout_code = kEctLongReadWriteTimeout;
                src_taskid = first->task.taskid;
                batchMap[first->channel_name] = std::make_pair(socket_timeout_code, src_taskid);
            }
        }

        if (cur_time - first->start_task_time >= first->task_timeout) {
            xerror2(TSF"task timeout, taskid:%_, nStartSendTime=%_, cur_time=%_, timeout:%_",
                    first->task.taskid, first->transfer_profile.start_send_time / 1000, cur_time / 1000, first->task_timeout / 1000);
            if(batchMap.find(first->channel_name) == batchMap.end()) {
                socket_timeout_code = kEctLongTaskTimeout;
                src_taskid = first->task.taskid;
                batchMap[first->channel_name] = std::make_pair(socket_timeout_code, src_taskid);
            }
            
            auto longlink = GetLongLink(first->channel_name);
            __SingleRespHandle(first, kEctLocal, kEctLocalTaskTimeout, kTaskFailHandleTaskTimeout, __GetConnectionProfile(longlink));
        }

        first = next;
    }

    for(auto item : batchMap) {
        if(item.second.first == kEctLongTaskTimeout) {
            __BatchErrorRespHandle(item.first, kEctNetMsgXP, kEctLocalTaskTimeout, kTaskFailHandleDefault, item.second.second);
        } else {
	        xassert2(fun_notify_network_err_);
            auto longlink = GetLongLink(item.first);
            if (longlink){
                auto longlink_channel = longlink->Channel();
                if(longlink_channel){
                    fun_notify_network_err_(item.first, __LINE__, kEctNetMsgXP, item.second.first, longlink_channel->Profile().ip,  longlink_channel->Profile().port);
                }
            }

            __BatchErrorRespHandle(item.first, kEctNetMsgXP, item.second.first, kTaskFailHandleDefault, item.second.second);
        }
    }
}

void LongLinkTaskManager::__RunOnStartTask() {
    xdebug_function();
    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

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

        Task task = first->task;
        if (get_real_host_) {
            get_real_host_(task.user_id, task.longlink_host_list, /*_strict_match=*/false);
        }
        std::string host = "";
        if (!task.longlink_host_list.empty()) {
            host = task.longlink_host_list.front();
        }
        xinfo2(TSF"host ip to callback is %_, task's channel name:%_", host, first->channel_name);

        // make sure login
        if (first->task.need_authed) {
            bool ismakesureauthsuccess = context_->GetManager<StnManager>()->MakesureAuthed(host, first->task.user_id);
            xinfo2(TSF"makesureauth host:%_, auth result:%_, cgi:%_, channal name:%_", host, ismakesureauthsuccess,first->task.cgi, first->channel_name);
            if (!ismakesureauthsuccess) {
                xinfo2_if(curtime % 3 == 0, TSF"makeSureAuth retsult=%0", ismakesureauthsuccess);
                first = next;
                continue;
            }
        }

        AutoBuffer bufreq;
        AutoBuffer buffer_extension;
        int error_code = 0;

        auto longlink = GetLongLink(first->channel_name);
	    if(longlink == nullptr) {
		    xerror2(TSF"longlink nullptr:%_", first->channel_name);
		    first = next;
		    continue;
	    }

        auto longlink_channel = longlink->Channel();

        if (!first->antiavalanche_checked) {
			if (!context_->GetManager<StnManager>()->Req2Buf(first->task.taskid, first->task.user_context, first->task.user_id, bufreq, buffer_extension, error_code, longlink->Config().link_type, host)) {
				__SingleRespHandle(first, kEctEnDecode, error_code, kTaskFailHandleTaskEnd, longlink_channel->Profile());
				first = next;
				continue;
			}
			// 雪崩检测
			xassert2(fun_anti_avalanche_check_);
			if (!fun_anti_avalanche_check_(first->task, bufreq.Ptr(), (int)bufreq.Length())) {
				__SingleRespHandle(first, kEctLocal, kEctLocalAntiAvalanche, kTaskFailHandleTaskEnd, longlink_channel->Profile());
				first = next;
				continue;
			}
           first->antiavalanche_checked = true;
        }

        xassert2(first->antiavalanche_checked);
        longlink->Channel()->SvrTrigOff();
		if (!longlink->Monitor()->MakeSureConnected()) {
            if (0 != first->task.channel_id) {
                __SingleRespHandle(first, kEctLocal, kEctLocalChannelID, kTaskFailHandleTaskEnd, longlink_channel->Profile());
            }
            
            first = next;
            continue;
		}

        if (0 != first->task.channel_id && longlink_channel->Profile().start_time != first->task.channel_id) {
            __SingleRespHandle(first, kEctLocal, kEctLocalChannelID, kTaskFailHandleTaskEnd, longlink_channel->Profile());
            first = next;
            continue;
        }
        
		if (0 == bufreq.Length()) {

			if (!context_->GetManager<StnManager>()->Req2Buf(first->task.taskid, first->task.user_context, first->task.user_id, bufreq, buffer_extension, error_code, longlink->Config().link_type, host)) {
				__SingleRespHandle(first, kEctEnDecode, error_code, kTaskFailHandleTaskEnd, longlink_channel->Profile());
				first = next;
				continue;
			}
			// 雪崩检测
			xassert2(fun_anti_avalanche_check_);
			if (!fun_anti_avalanche_check_(first->task, bufreq.Ptr(), (int)bufreq.Length())) {
				__SingleRespHandle(first, kEctLocal, kEctLocalAntiAvalanche, kTaskFailHandleTaskEnd, longlink_channel->Profile());
				first = next;
				continue;
			}
		}

        std::string intercept_data;
        if (task_intercept_.GetInterceptTaskInfo(first->task.cgi, intercept_data)) {
            xwarn2(TSF"task has been intercepted");
            AutoBuffer body;
            AutoBuffer extension;
            int err_code = 0;
            body.Write(intercept_data.data(), intercept_data.length());
            first->transfer_profile.received_size = body.Length();
            first->transfer_profile.receive_data_size = body.Length();
            first->transfer_profile.last_receive_pkg_time = ::gettickcount();
            int handle_type = context_->GetManager<StnManager>()->Buf2Resp(first->task.taskid, first->task.user_context, first->task.user_id, body, extension, err_code, longlink->Config().link_type);
            ConnectProfile profile;
            __SingleRespHandle(first, kEctEnDecode, err_code, handle_type, profile);
            first = next;
            continue;
        }

		first->transfer_profile.loop_start_task_time = ::gettickcount();
        first->transfer_profile.first_pkg_timeout = __FirstPkgTimeout(first->task.server_process_cost, bufreq.Length(), sent_count, dynamic_timeout_.GetStatus());
        first->current_dyntime_status = (first->task.server_process_cost <= 0) ? dynamic_timeout_.GetStatus() : kEValuating;
        first->transfer_profile.read_write_timeout = __ReadWriteTimeout(first->transfer_profile.first_pkg_timeout);
        first->transfer_profile.send_data_size = bufreq.Length();
        first->running_id = longlink_channel->Send(bufreq, buffer_extension, first->task);

        if (!first->running_id) {
            xwarn2(TSF"task add into longlink readwrite fail cgi:%_, cmdid:%_, taskid:%_", first->task.cgi, first->task.cmdid, first->task.taskid);
            first = next;
            continue;
        }

        xinfo2(TSF"task add into longlink readwrite suc cgi:%_, cmdid:%_, taskid:%_, size:%_, channel name:%_, timeout(firstpkg:%_, rw:%_, task:%_), retry:%_, curtime:%_, start_send_time:%_, sendonly:%_", first->task.cgi, first->task.cmdid, first->task.taskid, first->transfer_profile.send_data_size, first->channel_name,  first->transfer_profile.first_pkg_timeout / 1000,
               first->transfer_profile.read_write_timeout / 1000, first->task_timeout / 1000, first->remain_retry_count, curtime, first->start_task_time, first->task.send_only);

        if (first->task.send_only) {
            __SingleRespHandle(first, kEctOK, 0, kTaskFailHandleNoError, longlink_channel->Profile());
        }

        ++sent_count;
        first = next;
    }
}

bool LongLinkTaskManager::__SingleRespHandle(std::list<TaskProfile>::iterator _it, ErrCmdType _err_type, int _err_code, int _fail_handle, const ConnectProfile& _connect_profile) {
    xinfo_function();
    xassert2(kEctServer != _err_type);
    xassert2(_it != lst_cmd_.end());

    if(_it == lst_cmd_.end()) {
        xerror2(TSF"TaskProfile is in the end");
        return false;
    }
    
    _it->transfer_profile.connect_profile = _connect_profile;
    _it->link_type = _connect_profile.link_type;

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
        (TSF"cli(%_, %_, n:%_, sig:%_, link:%_), ", _it->transfer_profile.external_ip, _connect_profile.local_ip, _connect_profile.net_type,  _connect_profile.disconn_signal, ChannelTypeString[_connect_profile.link_type])
        (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, receive_data_size-received_size? string_cast(received_size).str():"", receive_data_size-received_size? "/":"", receive_data_size, _connect_profile.conn_rtt, (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time), (curtime - _it->start_task_time), _it->remain_retry_count)
        (TSF"cgi:%_, taskid:%_, tid:%_, context id:%_", _it->task.cgi, _it->task.taskid, _connect_profile.tid, _it->task.user_id);

        if(_err_type != kEctOK && _err_type != kEctServer) {
            xinfo_trace(TSF"cgi trace error: (%_, %_), cost:%_, rtt:%_, svr:(%_, %_, %_)", _err_type, _err_code, (curtime - _it->start_task_time), _connect_profile.conn_rtt,
                    _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type]);
        }

        int cgi_retcode = fun_callback_(_err_type, _err_code, _fail_handle, _it->task, (unsigned int)(curtime - _it->start_task_time));
        int errcode = _err_code;

        if (!_it->task.send_only && _it->running_id) {
        	if (kEctOK == _err_type) {
				errcode = cgi_retcode;
			}
		}

        _it->end_task_time = ::gettickcount();
        _it->err_code = errcode;
        _it->err_type = _err_type;
        _it->transfer_profile.error_type = _err_type;
        _it->transfer_profile.error_code = _err_code;
        _it->PushHistory();
        _it->is_weak_network = netsource_->GetWeakNetworkLogic()->IsCurrentNetworkWeak();
        int64_t span = 0;
        _it->is_last_valid_connect_fail = netsource_->GetWeakNetworkLogic()->IsLastValidConnectFail(span);
        context_->GetManager<StnManager>()->ReportTaskProfile(*_it);
        //WeakNetworkLogic::Singleton::Instance()->OnTaskEvent(*_it);
        netsource_->GetWeakNetworkLogic()->OnTaskEvent(*_it);
        cellular_network_manager_->OnTaskEvent(*_it);

        lst_cmd_.erase(_it);
        return true;
    }

    xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn, TSF"task end retry  long cmdid:%_, err(%_, %_, %_), ", _it->task.cmdid, _err_type, _err_code, _fail_handle)
    (TSF"svr(%_:%_, %_, %_), ", _connect_profile.ip, _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host)
    (TSF"cli(%_, %_, n:%_, sig:%_, link:%_), ", _it->transfer_profile.external_ip, _connect_profile.local_ip, _connect_profile.net_type, _connect_profile.disconn_signal, ChannelTypeString[_connect_profile.link_type])
    (TSF"cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ", _it->transfer_profile.send_data_size, receive_data_size-received_size? string_cast(received_size).str():"", receive_data_size-received_size? "/":"", receive_data_size, _connect_profile.conn_rtt, (_it->transfer_profile.start_send_time == 0 ? 0 : curtime - _it->transfer_profile.start_send_time), (curtime - _it->start_task_time), _it->remain_retry_count)
    (TSF"cgi:%_, taskid:%_, tid:%_", _it->task.cgi, _it->task.taskid, _connect_profile.tid);

    _it->remain_retry_count--;
    _it->transfer_profile.error_type = _err_type;
    _it->transfer_profile.error_code = _err_code;
    _it->PushHistory();
    _it->InitSendParam();
    
    return false;
}

void LongLinkTaskManager::__BatchErrorRespHandleByUserId(const std::string& _user_id, ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, bool _callback_runing_task_only) {
    std::list<TaskProfile> temp = lst_cmd_;
    auto first = temp.begin();
    while(first != temp.end()) {
        if(first->task.user_id != _user_id) {
            ++first;
            continue;
        }

        __BatchErrorRespHandle(first->channel_name, _err_type, _err_code, _fail_handle, _src_taskid, _callback_runing_task_only);
        ++first;
    }
}

void LongLinkTaskManager::__BatchErrorRespHandle(const std::string _name, ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, bool _callback_runing_task_only) {
    xinfo2(TSF"batch error, channel name:%_, src id:%_ callback:%_, errcode:%_, errtype:%_, fail handle:%_", _name, _src_taskid, _callback_runing_task_only, _err_code, _err_type, _fail_handle);
    xassert2(kEctOK != _err_type);
    xassert2(kTaskFailHandleTaskTimeout != _fail_handle);

    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    std::list<TaskProfile>::iterator last = lst_cmd_.end();

    if (lst_cmd_.empty()) {
        xerror2(TSF"task list is empty!");
    }

    while (first != last) {
        std::list<TaskProfile>::iterator next = first;
        ++next;

        if (_callback_runing_task_only && !first->running_id) {
            xwarn2(TSF"runnng id %_", first->running_id);
            first = next;
            continue;
        }

        if(!_name.empty() && first->channel_name != _name) {
            xwarn2(TSF"task channel name:%_", first->channel_name);
            first = next;
            continue;
        }

        auto longlink = GetLongLink(first->channel_name);
        if (!longlink){
            xerror2(TSF"didn't find longlink, task channel name:%_", first->channel_name);
            __DumpLongLinkChannelInfo();
        }
        
        auto profile = __GetConnectionProfile(longlink);
        if (_src_taskid == Task::kInvalidTaskID || _src_taskid == first->task.taskid)
            __SingleRespHandle(first, _err_type, _err_code, _fail_handle, profile);
        else
            __SingleRespHandle(first, _err_type, 0, _fail_handle, profile);

        first = next;
    }

    lastbatcherrortime_ = ::gettickcount();

    if (kEctLocal != _err_type &&  !lst_cmd_.empty()) {
        retry_interval_ = DEF_TASK_RETRY_INTERNAL;
    }

    if (kTaskFailHandleSessionTimeout == _fail_handle || kTaskFailHandleRetryAllTasks == _fail_handle) {
        __Disconnect(_name, LongLinkErrCode::kDecodeErr);
        MessageQueue::CancelMessage(asyncreg_.Get(), longlink_id_[_name]);
        retry_interval_ = 0;
    }

    if (kTaskFailHandleDefault == _fail_handle) {
        if (kEctDns != _err_type && kEctSocket != _err_type && kEctCanceld != _err_type) {  // not longlink callback
            __Disconnect(_name, LongLinkErrCode::kDecodeErr);
        }
        MessageQueue::CancelMessage(asyncreg_.Get(), longlink_id_[_name]);
    }

    if (kEctNetMsgXP == _err_type) {
        __Disconnect(_name, LongLinkErrCode::kTaskTimeout);
        MessageQueue::CancelMessage(asyncreg_.Get(), longlink_id_[_name]);
    }
}

void LongLinkTaskManager::__Disconnect(const std::string& _name, LongLinkErrCode::TDisconnectInternalCode code) {
	auto longlink = GetLongLink(_name);
    if(longlink != nullptr) {
	    longlink->Channel()->Disconnect(code);
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

void LongLinkTaskManager::__OnResponse(const std::string& _name, ErrCmdType _error_type, int _error_code, uint32_t _cmdid, uint32_t _taskid, AutoBuffer& _body, AutoBuffer& _extension, const ConnectProfile& _connect_profile) {
    move_wrapper<AutoBuffer> body(_body);
    move_wrapper<AutoBuffer> extension(_extension);
    RETURN_LONKLINK_SYNC2ASYNC_FUNC_TITLE(boost::bind(&LongLinkTaskManager::__OnResponse, this, _name, _error_type, _error_code, _cmdid, _taskid, body, extension, _connect_profile), longlink_id_[_name]);
    // svr push notify
    
    auto longlink_meta = GetLongLink(_name);
    if(longlink_meta == nullptr) {
        xwarn2(TSF"longlink response but longlink destroyed name:%_", _name);
        return;
    }
    
    auto longlink = longlink_meta->Channel();
    if (kEctOK == _error_type && longlink->Encoder().longlink_ispush(_cmdid, _taskid, body, extension))  {
        xinfo2(TSF"task push seq:%_, cmdid:%_, len:(%_, %_)", _taskid, _cmdid, body->Length(), extension->Length());
        
        if (fun_on_push_)
            fun_on_push_(_name, _cmdid, _taskid, body, extension);
        else
            xassert2(false);
        return;
    }
    
    std::list<TaskProfile>::iterator it = __Locate(_taskid);

    if (kEctOK != _error_type) {
        xwarn2(TSF"task error, taskid:%_, cmdid:%_, error_type:%_, error_code:%_", _taskid, _cmdid, _error_type, _error_code);
        if (_error_code == kEctHandshakeMisunderstand && it != lst_cmd_.end()) {
            it->remain_retry_count ++;
        }
        __BatchErrorRespHandle(_name, _error_type, _error_code, kTaskFailHandleDefault, 0);
        return;
    }

    
    if (lst_cmd_.end() == it) {
        xwarn2_if(Task::kInvalidTaskID != _taskid, TSF"task no found task:%0, cmdid:%1, ect:%2, errcode:%3",
                  _taskid, _cmdid, _error_type, _error_code);
        return;
    }
    
    it->transfer_profile.received_size = body->Length();
    it->transfer_profile.receive_data_size = body->Length();
    it->transfer_profile.last_receive_pkg_time = ::gettickcount();
    
    int err_code = 0;
    int handle_type = context_->GetManager<StnManager>()->Buf2Resp(it->task.taskid, it->task.user_context, it->task.user_id, body, extension, err_code, longlink_meta->Config().link_type);
    if (should_intercept_result_ && should_intercept_result_(err_code))  {
        task_intercept_.AddInterceptTask(it->task.cgi, std::string((const char*)body->Ptr(), body->Length()));
    }
    
    switch(handle_type){
        case kTaskFailHandleNoError:
        {
            dynamic_timeout_.CgiTaskStatistic(it->task.cgi, (unsigned int)it->transfer_profile.send_data_size + (unsigned int)body->Length(), ::gettickcount() - it->transfer_profile.start_send_time);
            __SingleRespHandle(it, kEctOK, err_code, handle_type, _connect_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(_name, __LINE__, kEctOK, err_code, _connect_profile.ip, _connect_profile.port);
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
            xwarn2(TSF"task decode error taskid:%_, cmdid:%_, handle_type:%_", it->task.taskid, it->task.cmdid, handle_type);
            __SingleRespHandle(it, kEctEnDecode, err_code, handle_type, _connect_profile);
        }
            break;
        case kTaskFailHandleDefault:
        {
            xerror2(TSF"task decode error taskid:%_, handle_type:%_, err_code:%_, body dump:%_", it->task.taskid, handle_type, err_code, xlogger_memory_dump(body->Ptr(), body->Length()));
            __BatchErrorRespHandle(it->channel_name, kEctEnDecode, err_code, handle_type, it->task.taskid);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(_name, __LINE__, kEctEnDecode, err_code, _connect_profile.ip, _connect_profile.port);
        }
            break;
        case kTaskSlientHandleTaskEnd:
        {
            xinfo2(TSF"task slient error taskid:%_, svr(%_:%_, %_, %_), handle_type:%_, err_code:%_, body dump:%_", it->task.taskid, _connect_profile.ip,
                    _connect_profile.port, IPSourceTypeString[_connect_profile.ip_type], _connect_profile.host, handle_type, err_code,
                    xlogger_memory_dump(body->Ptr(), std::min<size_t>(body->Length(), 1024)));

            lst_cmd_.erase(it);
        }
            break;
        default:
        {
			xassert2(false, TSF"task decode error fail_handle:%_, taskid:%_, context id:%_", handle_type, it->task.taskid, it->task.user_id);
			__BatchErrorRespHandle(it->channel_name, kEctEnDecode, err_code, handle_type, it->task.taskid);
			xassert2(fun_notify_network_err_);
			fun_notify_network_err_(_name, __LINE__, kEctEnDecode, handle_type, _connect_profile.ip, _connect_profile.port);
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
        if(it->transfer_profile.last_receive_pkg_time == 0)
            //WeakNetworkLogic::Singleton::Instance()->OnPkgEvent(true, (int)(::gettickcount() - it->transfer_profile.start_send_time));
            netsource_->GetWeakNetworkLogic()->OnPkgEvent(true, (int)(::gettickcount() - it->transfer_profile.start_send_time));
        else
            //WeakNetworkLogic::Singleton::Instance()->OnPkgEvent(false, (int)(::gettickcount() - it->transfer_profile.last_receive_pkg_time));
            netsource_->GetWeakNetworkLogic()->OnPkgEvent(false, (int)(::gettickcount() - it->transfer_profile.last_receive_pkg_time));
        it->transfer_profile.received_size = _cachedsize;
        it->transfer_profile.receive_data_size = _totalsize;
        it->transfer_profile.last_receive_pkg_time = ::gettickcount();
        xdebug2(TSF"taskid:%_, cachedsize:%_, _totalsize:%_", it->task.taskid, _cachedsize, _totalsize);
    } else {
        xwarn2(TSF"not found taskid:%_ cachedsize:%_, _totalsize:%_", _taskid, _cachedsize, _totalsize);
    }
}

void LongLinkTaskManager::__SignalConnection(LongLink::TLongLinkStatus _connect_status, const std::string& _channel_id) {
	if (LongLink::kConnected == _connect_status)
        __RunLoop();
}

std::shared_ptr<LongLinkMetaData> LongLinkTaskManager::GetLongLink(const std::string& _name) {
    MetaScopedLock lock(meta_mutex_);
    return GetLongLinkNoLock(_name);
}

std::shared_ptr<LongLinkMetaData> LongLinkTaskManager::GetLongLinkNoLock(const std::string& _name) {
    for (auto& item : longlink_metas_) {
        if (item.first == _name) {
            return item.second;
        }
    }
    return nullptr;
}


void LongLinkTaskManager::FixMinorRealhost(Task& _task) {
    if (get_real_host_) {
        get_real_host_(_task.user_id, _task.minorlong_host_list, /*_strict_match=*/false);
    }
}

bool LongLinkTaskManager::AddMinorLink(const std::vector<std::string>& _hosts) {
    if(_hosts.empty()) {
        return false;
    }
    std::string host = "";
    host = _hosts.front();

    LonglinkConfig defaultConfig(host, DEFAULT_LONGLINK_GROUP, false);
    defaultConfig.is_keep_alive = true;
    defaultConfig.host_list = _hosts;
    defaultConfig.link_type = Task::kChannelMinorLong;
    defaultConfig.longlink_encoder = &gDefaultLongLinkEncoder;

    return AddLongLink(defaultConfig);
}

bool LongLinkTaskManager::IsMinorAvailable(const Task& _task) {
    if(_task.minorlong_host_list.empty()) {
        return false;
    }
    std::string host = _task.minorlong_host_list.front();
    size_t linkCnt = 0;
    for(auto& item : longlink_metas_) {
        if(item.first == host) {
            linkCnt += 1;
            if(item.second->Channel()->ConnectStatus() != LongLink::kConnected || GetTaskCount(host) > 0)
                continue;
            return true;
        }
    }
    if((int)linkCnt >= _task.max_minorlinks)
        return false;
    else
        return true;
}

std::shared_ptr<LongLinkMetaData> LongLinkTaskManager::DefaultLongLink() {
        MetaScopedLock lock(meta_mutex_);
        for(auto& item : longlink_metas_) {
            if(item.second->Config().IsMain()) {
                return item.second;
            }
        }
        return nullptr;
    }

void LongLinkTaskManager::OnNetworkChange() {
    MetaScopedLock lock(meta_mutex_);
    for(auto& item : longlink_metas_) {
        if(item.second->Monitor()->NetworkChange()) {
            __RedoTasks(item.first, false);
        }
    }
}

#ifdef __APPLE__
void LongLinkTaskManager::__ResetLongLink(const std::string& _name) {
    ASYNC_BLOCK_START
    auto longlink = GetLongLink(_name);
    if(longlink) {
	    longlink->Channel()->Disconnect(LongLinkErrCode::kNetworkChange);
	    __RedoTasks(_name);
    }
    
    ASYNC_BLOCK_END
}
#endif

ConnectProfile LongLinkTaskManager::GetConnectProfile(uint32_t _taskid) {
    for(auto item : lst_cmd_) {
        if(item.task.taskid == _taskid) {
            auto longlink = GetLongLink(item.channel_name);
            if(longlink) {
                return longlink->Channel()->Profile();
            }
        }
    }
    return ConnectProfile();
}

bool LongLinkTaskManager::AddLongLink(LonglinkConfig& _config) {
    auto longlink = GetLongLink(_config.name);
    if(longlink != nullptr) {
        xwarn2(TSF"already have longlink name:%_", _config.name);
        return false;
    }
    
    MetaScopedLock lock(meta_mutex_);
    longlink_id ++;
    if (longlink_id == INT_MAX) {
        longlink_id = 1;
    }
    longlink_id_[_config.name] = longlink_id;
    xinfo2(TSF"new longlink name:%_, id:%_", _config.name, longlink_id);
    _config.need_tls = !__ForbidUseTls(_config.host_list);
    longlink_metas_[_config.name] = std::make_shared<LongLinkMetaData>(context_, _config, netsource_, active_logic_, asyncreg_.Get().queue);
    longlink = GetLongLinkNoLock(_config.name);
    longlink->Channel()->OnSend = boost::bind(&LongLinkTaskManager::__OnSend, this, _1);
    longlink->Channel()->OnRecv = boost::bind(&LongLinkTaskManager::__OnRecv, this, _1, _2, _3);
    longlink->Channel()->OnResponse = boost::bind(&LongLinkTaskManager::__OnResponse, this, _1, _2, _3, _4, _5, _6, _7, _8);
    longlink->Channel()->SignalConnection.connect(boost::bind(&LongLinkTaskManager::__SignalConnection, this, _1, _2));
    longlink->Channel()->OnHandshakeCompleted = boost::bind(&LongLinkTaskManager::__OnHandshakeCompleted, this, _1, _2);
#ifdef ANDROID
    longlink->Channel()->OnNoopAlarmSet = boost::bind(&LongLinkConnectMonitor::OnHeartbeatAlarmSet, longlink->Monitor(), _1);
    longlink->Channel()->OnNoopAlarmReceived = boost::bind(&LongLinkConnectMonitor::OnHeartbeatAlarmReceived, longlink->Monitor(), _1);
#endif
#ifdef __APPLE__
    longlink->Monitor()->fun_longlink_reset_ = boost::bind(&LongLinkTaskManager::__ResetLongLink, this, _config.name);
#endif
                              
    return true;
}

void LongLinkTaskManager::ReleaseLongLink(const std::string _name) {
    xinfo_function(TSF"release longlink:%_", _name);
    MetaScopedLock lock(meta_mutex_);
    auto longlink = GetLongLinkNoLock(_name);
    if(longlink == nullptr)
        return;

    auto task = lst_cmd_.begin();
    while(task != lst_cmd_.end()) {
		if(task->channel_name == _name) {
            if (__SingleRespHandle(task, kEctLocal, kEctLocalLongLinkReleased, kTaskFailHandleTaskEnd, longlink->Channel()->Profile())){
                // task erased
                task = lst_cmd_.begin();
            }else{
                task++;
            }
		} else {
			task++;
		}
    }

    longlink_metas_.erase(_name);
    longlink->Channel()->SignalConnection.disconnect_all_slots();
    longlink->Monitor()->DisconnectAllSlot();
    longlink.reset();
    lock.unlock();
    {
    // MessageQueue::AsyncInvoke([&,longlink] () {
//        longlink->Channel()->OnSend = NULL;
//        longlink->Channel()->OnRecv = NULL;
//        longlink->Channel()->OnResponse = NULL;
//#ifdef __APPLE__
//        longlink->Monitor()->fun_longlink_reset_ = NULL;
//#endif
        xinfo2(TSF"destroy long link %_ ", _name);
    // }, AYNC_HANDLER);
    }
}

void LongLinkTaskManager::ReleaseLongLink(std::shared_ptr<LongLinkMetaData> _linkmeta){
    std::string name = _linkmeta->Config().name;
    xinfo_function(TSF"release longlink:%_", name);

    auto task = lst_cmd_.begin();
    while(task != lst_cmd_.end()) {
        if(task->task.channel_name == name) {
            if (__SingleRespHandle(task, kEctLocal, kEctLocalLongLinkReleased, kTaskFailHandleTaskEnd, _linkmeta->Channel()->Profile())){
                // task erased
                task = lst_cmd_.begin();
            }else{
                task++;
            }
        } else {
            task++;
        }
    }

    _linkmeta->Channel()->SignalConnection.disconnect_all_slots();
    _linkmeta->Monitor()->DisconnectAllSlot();
}

bool LongLinkTaskManager::DisconnectByTaskId(uint32_t _taskid, LongLinkErrCode::TDisconnectInternalCode _code) {
    for(auto item : lst_cmd_) {
        if(item.task.taskid == _taskid) {
            auto longlink = GetLongLink(item.channel_name);
            if(longlink) {
                longlink->Channel()->Disconnect(_code);
                return true;
            }
        }
    }
    return false;
}

void LongLinkTaskManager::AddForbidTlsHost(const std::vector<std::string>& _host) {
	ScopedLock lock(mutex_);
    if (_host.empty()) {
        return;
    }
    std::string host_str = "";
    for (std::string h : _host) {
        if (h.find("long") == std::string::npos) {
            continue;
        }
        host_str += h;
        host_str += "," ;
        forbid_tls_host_.insert(h);
    }
    xinfo2(TSF"forbid tls hosts: %_", host_str);
}

bool LongLinkTaskManager::__ForbidUseTls(const std::vector<std::string>& _host_list) {
	ScopedLock lock(mutex_);
    xinfo2(TSF"host size: %_, %_", _host_list.size(), forbid_tls_host_.size());
    if (_host_list.empty() || forbid_tls_host_.empty()) {
        return false;
    }
    for (size_t i=0; i<_host_list.size(); i++) {
        xinfo2(TSF"host: %_", _host_list[i]);
        if (forbid_tls_host_.find(_host_list[i]) != forbid_tls_host_.end()) {
            return true;
        }
    }
    return false;
}

void LongLinkTaskManager::__DumpLongLinkChannelInfo() {
    MetaScopedLock lock(meta_mutex_);
    for(auto& item : longlink_metas_) {
        xinfo2(TSF"longlink channel name:%_, null:%_", item.first, item.second == nullptr);
    }
}

void LongLinkTaskManager::__OnHandshakeCompleted(uint32_t _version, mars::stn::TlsHandshakeFrom _from) {
    xinfo2(TSF"receive tls version: %_", _version);
    if (already_release_manager_) {
        xinfo2(TSF "mars2 longlink_task_manager had released. ignore handshake event.");
        return;
    }
    if (on_handshake_ready_) {
        on_handshake_ready_(_version, _from);
    }
}

ConnectProfile LongLinkTaskManager::__GetConnectionProfile(std::shared_ptr<LongLinkMetaData> longlink){
    ConnectProfile profile;
    profile.disconn_errtype = kEctLocal;
    profile.conn_errcode = kEctLocalLongLinkUnAvailable;
    
    if (longlink && longlink->Channel()){
        profile = longlink->Channel()->Profile();
    }
    
    return profile;
}
