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
#include "mars/comm/autobuffer.h"
#include "mars/comm/move_wrapper.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#ifdef ANDROID
#include "mars/comm/android/wakeuplock.h"
#endif

#include "config.h"
#include "dynamic_timeout.h"
#include "mars/app/app_manager.h"
#include "mars/stn/stn_manager.h"
#include "net_channel_factory.h"
#include "weak_network_logic.h"

using namespace mars::stn;
using namespace mars::app;
using namespace mars::comm;
using namespace mars::app;

#define USE_SHARED_STACK 1
constexpr size_t kThreadStackSize = 512 * 1024;
constexpr size_t kTotalCoStackSize = 256 * 1024;
constexpr size_t kCoStackSize = 64 * 1024;

#define AYNC_HANDLER asyncreg_.Get()
#define RETURN_SHORTLINK_SYNC2ASYNC_FUNC_TITLE(func, title) RETURN_SYNC2ASYNC_FUNC_TITLE(func, title, )

// boost::function<size_t (const std::string& _user_id, std::vector<std::string>& _host_list, bool _strict_match)>
// ShortLinkTaskManager::get_real_host_; boost::function<void (const int _error_type, const int _error_code, const int
// _use_ip_index)> ShortLinkTaskManager::task_connection_detail_; boost::function<int (TaskProfile& _profile)>
// ShortLinkTaskManager::choose_protocol_; boost::function<void (const TaskProfile& _profile)>
// ShortLinkTaskManager::on_timeout_or_remote_shutdown_; boost::function<void (uint32_t _version,
// mars::stn::TlsHandshakeFrom _from)> ShortLinkTaskManager::on_handshake_ready_; boost::function<bool (const
// std::vector<std::string> _host_list)> ShortLinkTaskManager::can_use_tls_; boost::function<bool (int _error_code)>
// ShortLinkTaskManager::should_intercept_result_;

ShortLinkTaskManager::ShortLinkTaskManager(boot::Context* _context,
                                           std::shared_ptr<NetSource> _netsource,
                                           DynamicTimeout& _dynamictimeout,
                                           MessageQueue::MessageQueue_t _messagequeueid)
: context_(_context)
, asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
, net_source_(_netsource)
, default_use_proxy_(true)
, tasks_continuous_fail_count_(0)
, dynamic_timeout_(_dynamictimeout)
#ifdef ANDROID
, wakeup_lock_(new WakeUpLock())
#endif
, owl_lst_looper_(owl::create_looper()) {
    xdebug_function(TSF "mars2");
    xinfo_function(TSF "handler:(%_,%_), ShortLinkTaskManager messagequeue_id=%_",
                   asyncreg_.Get().queue,
                   asyncreg_.Get().seq,
                   MessageQueue::Handler2Queue(asyncreg_.Get()));
    owl_looper_ = owl::create_looper(kThreadStackSize, [] {
        char stack[kTotalCoStackSize];
#if USE_SHARED_STACK
        owl::co_set_shared_stack(stack, sizeof(stack));
        // 共享栈模式协程的总内存占用为 kCoStackSize + max_concurrent_count * N，N
        // 为每个协程挂起时的用到的栈大小（一般为 2KB 左右） 因此共享栈模式下 max_concurrent_count 可以设置大一些
        owl::co_set_max_concurrent_count(1000);
#else
            // 为当前线程设置协程栈，协程栈将从 stack 中分配，每个协程的栈大小为 kCoStackSize
            owl::co_set_stack(stack, sizeof(stack), kCoStackSize);
        // 下一行代码不是必须的，co_set_stack 时会自动设置 max_concurrent_count
        // owl::co_set_max_concurrent_count(kTotalCoStackSize / kCoStackSize);
#endif
        // 开启消息循环
        owl::this_looper()->run_until_quit();
    });

    //    owl_looper_ = owl::create_looper("shortlink_task_manager");
    owl_scope_ = std::make_shared<owl::co_scope>("shortlink_task_manager");
    owl_scope_->options().exec = owl_looper_;

    owl::thread_pool_config conf = owl::thread_pool_config::get_default();
    conf.max_thread_count = 128;
    owl_task_looper = new owl::thread_pool(conf);
    owl_task_scope = new owl::async_scope(owl_task_looper);

    net_source_->DisableQUIC();
}

ShortLinkTaskManager::~ShortLinkTaskManager() {
    xinfo_function(TSF "mars2");
    already_release_manager_ = true;
    //    asyncreg_.CancelAndWait();
    xinfo2(TSF "lst_cmd_ count=%0", lst_cmd_.size());
    __BatchErrorRespHandle(kEctLocal, kEctLocalReset, kTaskFailHandleTaskEnd, Task::kInvalidTaskID, false);
#ifdef ANDROID
    delete wakeup_lock_;
#endif

    if (owl_scope_) {
        owl_scope_->cancel();
        owl_scope_->join();
        owl_looper_ = nullptr;
    }
    if (owl_looper_) {
        owl_looper_->quit();
        owl_looper_->join();
        owl_looper_ = nullptr;
    }
}

bool ShortLinkTaskManager::StartTask(const Task& _task, PrepareProfile _prepare_profile) {
    xverbose_function();
    if (_task.send_only) {
        xassert2(false);
        xerror2(TSF "taskid:%_, short link should have resp", _task.taskid);
        return false;
    }

    xinfo2(TSF "task is long-polling task:%_, cgi:%_, timeout:%_",
           _task.long_polling,
           _task.cgi,
           _task.long_polling_timeout);

    xdebug2(TSF "taskid:%0", _task.taskid);

    //    owl_scope_->co_launch([this, _task, _prepare_profile] {

    owl_task_scope->async([this, _task, _prepare_profile] {
        TaskProfile task(_task, _prepare_profile);
        task.link_type = Task::kChannelShort;
        CoJobTaskProfile co_task_profile(task);

        //    owl::co_options options = owl::co_options();
        //    options.priority = task.task.priority;
        //
        //    owl::co_job job = owl_scope_->co_launch(options, [this, &co_task_profile] {
        //        __CoLaunchStartTask(co_task_profile);
        //    });
        //    job->set_name(std::to_string(task.task.taskid));
        //    co_task_profile.setCoJob(job);

        lst_cmd_.push_back(co_task_profile);
        lst_cmd_.sort(__CompareTask);
        __RunLoop();
    });

    //    });
    return true;
}

bool ShortLinkTaskManager::StopTask(uint32_t _taskid) {
    xverbose_function();

    //    std::list<TaskProfile>::iterator first = lst_cmd_.begin();
    //    std::list<TaskProfile>::iterator last = lst_cmd_.end();
    //
    //    while (first != last) {
    //        if (_taskid == first->task.taskid) {
    //            xinfo2(TSF "find the task, taskid:%0", _taskid);
    //
    //            __DeleteShortLink(first->running_id);
    //            lst_cmd_.erase(first);
    //            return true;
    //        }
    //
    //        ++first;
    //    }
    //    owl_scope_->co_launch([this, _taskid] {
    owl_task_scope->async([this, _taskid] {
        std::list<CoJobTaskProfile>::iterator first = lst_cmd_.begin();
        std::list<CoJobTaskProfile>::iterator last = lst_cmd_.end();
        while (first != last) {
            if (_taskid == first->task_profile.task.taskid) {
                xinfo2(TSF "erase task cgi %_", first->task_profile.task.cgi);
                lst_cmd_.erase(first);

                //                if (first->job) {
                //                    if(!first->job->is_completed()){
                //                        first->job->cancel();
                //                        first->job = nullptr;
                //                    }
                //                }

                return true;
            }
            ++first;
        }
        //    });
    });
    return false;
}

bool ShortLinkTaskManager::HasTask(uint32_t _taskid) const {
    xverbose_function();

    //    std::list<TaskProfile>::const_iterator first = lst_cmd_.begin();
    //    std::list<TaskProfile>::const_iterator last = lst_cmd_.end();
    //
    //    while (first != last) {
    //        if (_taskid == first->task.taskid) {
    //            return true;
    //        }
    //        ++first;
    //    }

    std::list<CoJobTaskProfile>::const_iterator first = lst_cmd_.begin();
    std::list<CoJobTaskProfile>::const_iterator last = lst_cmd_.end();
    while (first != last) {
        if (_taskid == first->task_profile.task.taskid) {
            return true;
        }
        ++first;
    }

    return false;
}

void ShortLinkTaskManager::ClearTasks() {
    xverbose_function();

    xinfo2(TSF "cmd size:%0", lst_cmd_.size());
    owl_task_scope->cancel_all();

    for (std::list<CoJobTaskProfile>::iterator it = lst_cmd_.begin(); it != lst_cmd_.end(); ++it) {
        __DeleteShortLink(it->task_profile.running_id);
    }
    //
    //    lst_cmd_.clear();
    //    owl_scope_->co_launch([this] {
    //        for (std::list<CoJobTaskProfile>::iterator it = lst_cmd_.begin(); it != lst_cmd_.end(); ++it) {
    //            if (it->job) {
    //                if(!it->job->is_completed()){
    //                    it->job->cancel();
    //                    it->job = nullptr;
    //                }
    //            }
    //        }
    xinfo2(TSF "clear all task.");
    owl_task_scope->async([this] {
        xinfo2(TSF "erase clear task");
        lst_cmd_.clear();
    });

    //    });
}

unsigned int ShortLinkTaskManager::GetTasksContinuousFailCount() {
    return tasks_continuous_fail_count_;
}

void ShortLinkTaskManager::__RunLoop() {
    if (lst_cmd_.empty()) {
#ifdef ANDROID
        /*cancel the last wakeuplock*/
        if (context_->GetManager<AppManager>() != nullptr) {
            wakeup_lock_->Lock(context_->GetManager<AppManager>()->GetConfig<int>(kKeyShortLinkWakeupLockEmptyCMD,
                                                                                  kShortLinkWakeupLockEmptyCMD));
        } else {
            xinfo2(TSF "appmanager no exist.");
            wakeup_lock_->Lock(kShortLinkWakeupLockEmptyCMD);
        }
#endif
        return;
    }

    //    __RunOnTimeout();
    __RunOnStartTask();

    if (!lst_cmd_.empty()) {
#ifdef ANDROID
        if (context_->GetManager<AppManager>() != nullptr) {
            wakeup_lock_->Lock(context_->GetManager<AppManager>()->GetConfig<int>(kKeyShortLinkWakeupLockRunCMD,
                                                                                  kShortLinkWakeupLockRunCMD));
        } else {
            xinfo2(TSF "appmanager no exist.");
            wakeup_lock_->Lock(kShortLinkWakeupLockRunCMD);
        }
#endif
        //        MessageQueue::FasterMessage(asyncreg_.Get(),
        //                                    MessageQueue::Message((MessageQueue::MessageTitle_t)this,
        //                                                          boost::bind(&ShortLinkTaskManager::__RunLoop, this),
        //                                                          "ShortLinkTaskManager::__RunLoop"),
        //                                    MessageQueue::MessageTiming(DEF_TASK_RUN_LOOP_TIMING));

        //        owl_looper_->async_periodic(DEF_TASK_RUN_LOOP_TIMING, DEF_TASK_RUN_LOOP_TIMING, [this] {
        //            __RunOnTimeout();
        //            __RunOnStartTask();
        //        });
        owl_task_scope->async_after(DEF_TASK_RUN_LOOP_TIMING, [this] {
            __RunLoop();
        });

    } else {
#ifdef ANDROID
        /*cancel the last wakeuplock*/
        if (context_->GetManager<AppManager>() != nullptr) {
            wakeup_lock_->Lock(context_->GetManager<AppManager>()->GetConfig<int>(kKeyShortLinkWakeupLockEmptyCMD,
                                                                                  kShortLinkWakeupLockEmptyCMD));
        } else {
            xinfo2(TSF "appmanager no exist.");
            wakeup_lock_->Lock(kShortLinkWakeupLockEmptyCMD);
        }
#endif
    }
}

void ShortLinkTaskManager::__RunOnTimeout() {
    xverbose2(TSF "lst_cmd_ size=%0", lst_cmd_.size());

    //    owl_scope_->co_launch([this] {
    //    owl_task_scope->async([this]{
    socket_pool_.CleanTimeout();

    std::list<CoJobTaskProfile>::iterator first = lst_cmd_.begin();
    std::list<CoJobTaskProfile>::iterator last = lst_cmd_.end();

    uint64_t cur_time = ::gettickcount();

    while (first != last) {
        std::list<CoJobTaskProfile>::iterator next = first;
        ++next;

        ErrCmdType err_type = kEctLocal;
        int socket_timeout_code = 0;
        // xinfo2(TSF"task is long-polling task:%_,%_, cgi:%_,%_, timeout:%_, id %_",first->task.long_polling,
        // first->transfer_profile.task.long_polling, first->transfer_profile.task.cgi,first->task.cgi,
        // first->transfer_profile.task.long_polling_timeout, (void*)first->running_id);

        if (cur_time - first->task_profile.start_task_time >= first->task_profile.task_timeout) {
            err_type = kEctLocal;
            socket_timeout_code = kEctLocalTaskTimeout;
        } else if (first->task_profile.running_id && 0 < first->task_profile.transfer_profile.start_send_time
                   && cur_time - first->task_profile.transfer_profile.start_send_time
                          >= first->task_profile.transfer_profile.read_write_timeout) {
            xerror2(TSF "task read-write timeout, taskid:%_, wworker:%_, nStartSendTime:%_, nReadWriteTimeOut:%_",
                    first->task_profile.task.taskid,
                    (void*)first->task_profile.running_id,
                    first->task_profile.transfer_profile.start_send_time / 1000,
                    first->task_profile.transfer_profile.read_write_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpReadWriteTimeout;
        } else if (first->task_profile.running_id && first->task_profile.task.long_polling
                   && 0 < first->task_profile.transfer_profile.start_send_time
                   && 0 == first->task_profile.transfer_profile.last_receive_pkg_time
                   && cur_time - first->task_profile.transfer_profile.start_send_time
                          >= (uint64_t)first->task_profile.task.long_polling_timeout) {
            xerror2(TSF "task long-polling timeout, taskid:%_, wworker:%_, nStartSendTime:%_, nLongPollingTimeout:%_",
                    first->task_profile.task.taskid,
                    (void*)first->task_profile.running_id,
                    first->task_profile.transfer_profile.start_send_time / 1000,
                    first->task_profile.task.long_polling_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpLongPollingTimeout;
        } else if (first->task_profile.running_id && !first->task_profile.task.long_polling
                   && 0 < first->task_profile.transfer_profile.start_send_time
                   && 0 == first->task_profile.transfer_profile.last_receive_pkg_time
                   && cur_time - first->task_profile.transfer_profile.start_send_time
                          >= first->task_profile.transfer_profile.first_pkg_timeout) {
            xerror2(TSF "task first-pkg timeout taskid:%_, wworker:%_, nStartSendTime:%_, nfirstpkgtimeout:%_",
                    first->task_profile.task.taskid,
                    (void*)first->task_profile.running_id,
                    first->task_profile.transfer_profile.start_send_time / 1000,
                    first->task_profile.transfer_profile.first_pkg_timeout / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpFirstPkgTimeout;
        } else if (first->task_profile.running_id && 0 < first->task_profile.transfer_profile.start_send_time
                   && 0 < first->task_profile.transfer_profile.last_receive_pkg_time
                   && cur_time - first->task_profile.transfer_profile.last_receive_pkg_time
                          >= ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval)) {
            xerror2(TSF "task pkg-pkg timeout, taskid:%_, wworker:%_, nLastRecvTime:%_, pkg-pkg timeout:%_",
                    first->task_profile.task.taskid,
                    (void*)first->task_profile.running_id,
                    first->task_profile.transfer_profile.last_receive_pkg_time / 1000,
                    ((kMobile != getNetInfo()) ? kWifiPackageInterval : kGPRSPackageInterval) / 1000);
            err_type = kEctHttp;
            socket_timeout_code = kEctHttpPkgPkgTimeout;
        } else {
            // pass
        }

        if (0 != socket_timeout_code) {
            std::string ip = first->task_profile.running_id
                                 ? ((ShortLinkInterface*)first->task_profile.running_id)->Profile().ip
                                 : "";
            std::string host = first->task_profile.running_id
                                   ? ((ShortLinkInterface*)first->task_profile.running_id)->Profile().host
                                   : "";
            int port = first->task_profile.running_id
                           ? ((ShortLinkInterface*)first->task_profile.running_id)->Profile().port
                           : 0;
            dynamic_timeout_.CgiTaskStatistic(first->task_profile.task.cgi, kDynTimeTaskFailedPkgLen, 0);
            __OnRequestTimeout(reinterpret_cast<ShortLinkInterface*>(first->task_profile.running_id),
                               socket_timeout_code);
            // TODO cpan
            //__SetLastFailedStatus(first->task_profile);

            __SingleRespHandle(*first,
                               err_type,
                               socket_timeout_code,
                               err_type == kEctLocal ? kTaskFailHandleTaskTimeout : kTaskFailHandleDefault,
                               0,
                               first->task_profile.running_id
                                   ? ((ShortLinkInterface*)first->task_profile.running_id)->Profile()
                                   : ConnectProfile());
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__, err_type, socket_timeout_code, ip, host, port);
        }

        first = next;
    }
    //    });
    //    });
}

void ShortLinkTaskManager::__RunOnStartTask() {
    std::list<CoJobTaskProfile>::iterator first = lst_cmd_.begin();
    std::list<CoJobTaskProfile>::iterator last = lst_cmd_.end();

    uint64_t curtime = ::gettickcount();
    int sent_count = 0;

    while (first != last) {
        std::list<CoJobTaskProfile>::iterator next = first;
        ++next;

        if (first->task_profile.running_id) {
            xinfo2(TSF "running_id is no empty.");
            ++sent_count;
            first = next;
            continue;
        }

        //        if (first->job) {
        //            xinfo2(TSF "jobs is no empty.");
        //            ++sent_count;
        //            first = next;
        //            continue;
        //        }

        // retry time interval is 1 second, but when last connect is quic, retry now
        if (first->task_profile.retry_time_interval > curtime - first->task_profile.retry_start_time) {
            if (first->task_profile.history_transfer_profiles.empty()
                || first->task_profile.history_transfer_profiles.back().connect_profile.transport_protocol
                       != Task::kTransportProtocolQUIC) {
                xinfo2(TSF "retry interval, taskid:%0, task retry late task, wait:%1, retry:%2",
                       first->task_profile.task.taskid,
                       (curtime - first->task_profile.transfer_profile.loop_start_task_time) / 1000,
                       first->task_profile.history_transfer_profiles.empty() ? "false" : "true");
                first = next;
                continue;
            }
        }

        // proxy
        first->task_profile.use_proxy =
            (first->task_profile.remain_retry_count == 0 && first->task_profile.task.retry_count > 0)
                ? !default_use_proxy_
                : default_use_proxy_;

        //
        if (!first->task_profile.history_transfer_profiles.empty()) {
            // retry task, force use tcp.
            xassert2(!first->task_profile.task.shortlink_fallback_hostlist.empty());
            xwarn2(TSF "taskid:%_ retry, fallback to tcp with host.cnt %_",
                   first->task_profile.task.taskid,
                   first->task_profile.task.shortlink_fallback_hostlist.size());

            first->task_profile.task.transport_protocol = Task::kTransportProtocolTCP;
            first->task_profile.task.shortlink_host_list = first->task_profile.task.shortlink_fallback_hostlist;
        }

        // cpan OWL
        //__CoLaunchStartTask(*first);
        CoJobTaskProfile p = *first;
        owl::co_options options = owl::co_options();
        options.priority = first->task_profile.task.priority;

        //        owl::co_job job = owl_scope_->co_launch([this, p] {
        owl_task_scope->async([this, p] {
            __CoLaunchStartTask(p);
        });
        //        });

        // job->set_name(first->task_profile.task.cgi);
        //    co_task_profile.setCoJob(job);
        // first->setCoJob(job);

        first = next;

    }  // end of while
}

struct find_seq {
 public:
    bool operator()(const TaskProfile& _value) {
        return p_worker == (ShortLinkInterface*)_value.running_id;
    }

 public:
    ShortLinkInterface* p_worker;
};

void ShortLinkTaskManager::__OnResponse(CoJobTaskProfile& _co_task_profile,
                                        ErrCmdType _err_type,
                                        int _status,
                                        AutoBuffer& _body,
                                        AutoBuffer& _extension,
                                        bool _cancel_retry,
                                        ConnectProfile& _conn_profile) {
    //    co_main_scope() {
    // TODO cpan cgi profile 这里的body extension copy了一份
    //    owl_looper_->async([this, _worker, _err_type, _status, _body, _extension, _cancel_retry, _conn_profile] {
    xdebug2(TSF "taskid=%0, _err_type=%1, _status=%2, _body.lenght=%3, _cancel_retry=%4",
            _co_task_profile.task_profile.task.taskid,
            _err_type,
            _status,
            _body.Length(),
            _cancel_retry);

    fun_shortlink_response_(_status);

    if (_conn_profile.socket_fd != INVALID_SOCKET) {
        if (_err_type != kEctOK) {
            _conn_profile.closefunc(_conn_profile.socket_fd);
            if (_status != kEctSocketShutdown) {  // ignore server close error
                socket_pool_.Report(_conn_profile.is_reused_fd, false, false);
            }
        } else if (_conn_profile.ip_index >= 0 && _conn_profile.ip_index < (int)_conn_profile.ip_items.size()) {
            IPPortItem item = _conn_profile.ip_items[_conn_profile.ip_index];
            CacheSocketItem cache_item(item,
                                       _conn_profile.socket_fd,
                                       _conn_profile.keepalive_timeout,
                                       _conn_profile.closefunc,
                                       _conn_profile.createstream_func,
                                       _conn_profile.issubstream_func);
            if (!socket_pool_.AddCache(cache_item)) {
                _conn_profile.closefunc(cache_item.socket_fd);
            }
        } else {
            xassert2(false, "not match");
        }
    }

    if (_err_type != kEctOK) {
        if (_err_type == kEctSocket && _status == kEctSocketMakeSocketPrepared) {
            dynamic_timeout_.CgiTaskStatistic(_co_task_profile.task_profile.task.cgi, kDynTimeTaskFailedPkgLen, 0);
            // TODO cpan
            //__SetLastFailedStatus(it);
        }

        if (_err_type != kEctLocal && _err_type != kEctCanceld) {
            if (_conn_profile.transport_protocol == Task::kTransportProtocolQUIC) {
                // quic失败,临时屏蔽20分钟，直到下一次网络切换或者20分钟后再尝试.
                xwarn2(TSF "disable quic. err %_:%_", _err_type, _status);
                net_source_->DisableQUIC();

                //.increment retry count when first quic failed.
                if (_co_task_profile.task_profile.history_transfer_profiles.empty()) {
                    ++_co_task_profile.task_profile.remain_retry_count;
                }
            }
        }

        if (_err_type == kEctSocket) {
            _co_task_profile.task_profile.force_no_retry = _cancel_retry;
        }
        if (_status == kEctHandshakeMisunderstand) {
            _co_task_profile.task_profile.remain_retry_count++;
        }
        __SingleRespHandle(_co_task_profile, _err_type, _status, kTaskFailHandleDefault, _body.Length(), _conn_profile);
        return;
    }

    _co_task_profile.task_profile.transfer_profile.received_size = _body.Length();
    _co_task_profile.task_profile.transfer_profile.receive_data_size = _body.Length();
    _co_task_profile.task_profile.transfer_profile.last_receive_pkg_time = ::gettickcount();

    int err_code = 0;
    unsigned short server_sequence_id = 0;
    _co_task_profile.task_profile.transfer_profile.begin_buf2resp_time = gettickcount();
    int handle_type = context_->GetManager<StnManager>()->Buf2Resp(_co_task_profile.task_profile.task.taskid,
                                                                   _co_task_profile.task_profile.task.user_context,
                                                                   _co_task_profile.task_profile.task.user_id,
                                                                   _body,
                                                                   _extension,
                                                                   err_code,
                                                                   Task::kChannelShort,
                                                                   server_sequence_id);
    xinfo2_if(_co_task_profile.task_profile.task.priority >= 0, TSF "err_code %_ ", err_code);
    xinfo2(TSF "server_sequence_id:%_", server_sequence_id);
    _co_task_profile.task_profile.transfer_profile.end_buf2resp_time = gettickcount();
    _co_task_profile.task_profile.task.server_sequence_id = server_sequence_id;
    socket_pool_.Report(_conn_profile.is_reused_fd, true, handle_type == kTaskFailHandleNoError);
    if (should_intercept_result_ && should_intercept_result_(err_code)) {
        task_intercept_.AddInterceptTask(_co_task_profile.task_profile.task.cgi,
                                         std::string((const char*)_body.Ptr(), _body.Length()));
    }

    switch (handle_type) {
        case kTaskFailHandleNoError: {
            dynamic_timeout_.CgiTaskStatistic(
                _co_task_profile.task_profile.task.cgi,
                (unsigned int)_co_task_profile.task_profile.transfer_profile.send_data_size
                    + (unsigned int)_body.Length(),
                ::gettickcount() - _co_task_profile.task_profile.transfer_profile.start_send_time);
            __SingleRespHandle(_co_task_profile,
                               kEctOK,
                               err_code,
                               handle_type,
                               (unsigned int)_co_task_profile.task_profile.transfer_profile.receive_data_size,
                               _conn_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__,
                                    kEctOK,
                                    err_code,
                                    _conn_profile.ip,
                                    _conn_profile.host,
                                    _conn_profile.port);
        } break;
        case kTaskFailHandleSessionTimeout: {
            xassert2(fun_notify_retry_all_tasks);
            xwarn2(TSF "task decode error session timeout taskid:%_, cmdid:%_, cgi:%_",
                   _co_task_profile.task_profile.task.taskid,
                   _co_task_profile.task_profile.task.cmdid,
                   _co_task_profile.task_profile.task.cgi);
            fun_notify_retry_all_tasks(kEctEnDecode,
                                       err_code,
                                       handle_type,
                                       _co_task_profile.task_profile.task.taskid,
                                       _co_task_profile.task_profile.task.user_id);
        } break;
        case kTaskFailHandleRetryAllTasks: {
            xassert2(fun_notify_retry_all_tasks);
            xwarn2(TSF "task decode error retry all task taskid:%_, cmdid:%_, cgi:%_",
                   _co_task_profile.task_profile.task.taskid,
                   _co_task_profile.task_profile.task.cmdid,
                   _co_task_profile.task_profile.task.cgi);
            fun_notify_retry_all_tasks(kEctEnDecode,
                                       err_code,
                                       handle_type,
                                       _co_task_profile.task_profile.task.taskid,
                                       _co_task_profile.task_profile.task.user_id);
        } break;
        case kTaskFailHandleTaskEnd: {
            __SingleRespHandle(_co_task_profile,
                               kEctEnDecode,
                               err_code,
                               handle_type,
                               (unsigned int)_co_task_profile.task_profile.transfer_profile.receive_data_size,
                               _conn_profile);
        } break;
        case kTaskFailHandleDefault: {
            xerror2(TSF "task decode error handle_type:%_, err_code:%_, pWorker:%_, taskid:%_ body dump:%_",
                    handle_type,
                    err_code,
                    (void*)_co_task_profile.task_profile.running_id,
                    _co_task_profile.task_profile.task.taskid,
                    xlogger_memory_dump(_body.Ptr(), _body.Length()));
            __SingleRespHandle(_co_task_profile,
                               kEctEnDecode,
                               err_code,
                               handle_type,
                               (unsigned int)_co_task_profile.task_profile.transfer_profile.receive_data_size,
                               _conn_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__,
                                    kEctEnDecode,
                                    handle_type,
                                    _conn_profile.ip,
                                    _conn_profile.host,
                                    _conn_profile.port);
        } break;
        default: {
            xerror2(TSF "task decode error fail_handle:%_, taskid:%_, context id:%_",
                    handle_type,
                    _co_task_profile.task_profile.task.taskid,
                    _co_task_profile.task_profile.task.user_id);
            //#ifdef __APPLE__
            //            //.test only.
            //            const char* pbuffer = (const char*)_body.Ptr();
            //            for (size_t off = 0; off < _body.Length();){
            //                size_t len = std::min((size_t)512, _body.Length() - off);
            //                xerror2(TSF"[%_-%_] %_", off, off + len, xlogger_memory_dump(pbuffer + off, len));
            //                off += len;
            //            }
            //#endif
            __SingleRespHandle(_co_task_profile,
                               kEctEnDecode,
                               err_code,
                               handle_type,
                               (unsigned int)_co_task_profile.task_profile.transfer_profile.receive_data_size,
                               _conn_profile);
            xassert2(fun_notify_network_err_);
            fun_notify_network_err_(__LINE__,
                                    kEctEnDecode,
                                    handle_type,
                                    _conn_profile.ip,
                                    _conn_profile.host,
                                    _conn_profile.port);
            break;
        }
    }
    //    });
    //    };//end of scope
}

void ShortLinkTaskManager::__OnSend(TaskProfile& task_profile) {
    if (task_profile.transfer_profile.first_start_send_time == 0)
        task_profile.transfer_profile.first_start_send_time = ::gettickcount();
    task_profile.transfer_profile.start_send_time = ::gettickcount();
    xdebug2(TSF "taskid:%_, nStartSendTime:%_",
            task_profile.task.taskid,
            task_profile.transfer_profile.start_send_time / 1000);
}

void ShortLinkTaskManager::__OnRecv(TaskProfile& task_profile, unsigned int _cached_size, unsigned int _total_size) {
    xverbose_function();

    if (task_profile.transfer_profile.last_receive_pkg_time == 0) {
        // WeakNetworkLogic::Singleton::Instance()->OnPkgEvent(true, (int)(::gettickcount() -
        // task_profile.transfer_profile.start_send_time));
        net_source_->GetWeakNetworkLogic()->OnPkgEvent(
            true,
            (int)(::gettickcount() - task_profile.transfer_profile.start_send_time));
    } else {
        // WeakNetworkLogic::Singleton::Instance()->OnPkgEvent(false, (int)(::gettickcount() -
        // task_profile.transfer_profile.last_receive_pkg_time));
        net_source_->GetWeakNetworkLogic()->OnPkgEvent(
            false,
            (int)(::gettickcount() - task_profile.transfer_profile.last_receive_pkg_time));
    }
    task_profile.transfer_profile.last_receive_pkg_time = ::gettickcount();
    task_profile.transfer_profile.received_size = _cached_size;
    task_profile.transfer_profile.receive_data_size = _total_size;
    xdebug2(TSF "last_recvtime:%_, cachedsize:%_, totalsize:%_",

            task_profile.transfer_profile.last_receive_pkg_time / 1000,
            _cached_size,
            _total_size);
}

void ShortLinkTaskManager::RedoTasks() {
    xinfo_function();

    owl_task_scope->cancel_all();

    owl_task_scope->async([this] {
        std::list<CoJobTaskProfile>::iterator first = lst_cmd_.begin();
        std::list<CoJobTaskProfile>::iterator last = lst_cmd_.end();

        while (first != last) {
            std::list<CoJobTaskProfile>::iterator next = first;
            ++next;

            first->task_profile.last_failed_dyntime_status = 0;

            if (first->task_profile.running_id) {
                xinfo2(TSF "task redo, taskid:%_", first->task_profile.task.taskid);
                __SingleRespHandle(*first,
                                   kEctLocal,
                                   kEctLocalCancel,
                                   kTaskFailHandleDefault,
                                   0,
                                   ((ShortLinkInterface*)first->task_profile.running_id)->Profile());
            }

            first = next;
        }

        socket_pool_.Clear();
        __RunLoop();
    });
}

void ShortLinkTaskManager::TouchTasks() {
    owl_task_scope->async([this] {
        __RunLoop();
    });
}

void ShortLinkTaskManager::RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid) {
    xverbose_function();
    xinfo2(TSF "RetryTasks taskid %_ ", _src_taskid);

    owl_task_scope->async([this, _err_type, _err_code, _fail_handle, _src_taskid] {
        __BatchErrorRespHandle(_err_type, _err_code, _fail_handle, _src_taskid);
        __RunLoop();
    });
}

void ShortLinkTaskManager::__BatchErrorRespHandle(ErrCmdType _err_type,
                                                  int _err_code,
                                                  int _fail_handle,
                                                  uint32_t _src_taskid,
                                                  bool _callback_runing_task_only) {
    xassert2(kEctOK != _err_type);
    xdebug2(TSF "ect=%0, errcode=%1 taskid:=%2", _err_type, _err_code, _src_taskid);

    //    owl_task_scope->async([this, _err_type, _err_code, _fail_handle, _src_taskid, _callback_runing_task_only] {

    std::list<CoJobTaskProfile>::iterator first = lst_cmd_.begin();
    std::list<CoJobTaskProfile>::iterator last = lst_cmd_.end();

    while (first != last) {
        std::list<CoJobTaskProfile>::iterator next = first;
        ++next;

        if (_callback_runing_task_only && !first->task_profile.running_id) {
            first = next;
            continue;
        }

        if (_fail_handle == kTaskFailHandleSessionTimeout && !first->task_profile.task.need_authed) {
            first = next;
            continue;
        }

        xinfo2(TSF "axauth sessiontime id %_, cgi %_ taskid %_",
               _src_taskid,
               first->task_profile.task.cgi,
               first->task_profile.task.taskid);

        if (_fail_handle == kTaskFailHandleSessionTimeout && _src_taskid != 0
            && first->task_profile.task.taskid == _src_taskid
            && first->task_profile.allow_sessiontimeout_retry) {  // retry task when sessiontimeout
            xinfo2(TSF "axauth to timeout queue %_, cgi %_ ",
                   first->task_profile.task.taskid,
                   first->task_profile.task.cgi);
            first->task_profile.allow_sessiontimeout_retry = false;
            first->task_profile.remain_retry_count++;
            __DeleteShortLink(first->task_profile.running_id);
            first->task_profile.PushHistory();
            first->task_profile.InitSendParam();
            first = next;
            continue;
        }

        if (_src_taskid == Task::kInvalidTaskID || _src_taskid == first->task_profile.task.taskid)
            __SingleRespHandle(*first,
                               _err_type,
                               _err_code,
                               _fail_handle,
                               0,
                               first->task_profile.running_id
                                   ? ((ShortLinkInterface*)first->task_profile.running_id)->Profile()
                                   : ConnectProfile());
        else
            __SingleRespHandle(*first,
                               _err_type,
                               0,
                               _fail_handle,
                               0,
                               first->task_profile.running_id
                                   ? ((ShortLinkInterface*)first->task_profile.running_id)->Profile()
                                   : ConnectProfile());

        first = next;
    }
    //    });
}

// std::list<TaskProfile>::iterator ShortLinkTaskManager::__LocateBySeq(intptr_t _running_id) {
//    if (!_running_id)
//        return lst_cmd_.end();
//
//    find_seq find_functor;
//    find_functor.p_worker = (ShortLinkInterface*)_running_id;
//    std::list<TaskProfile>::iterator it = std::find_if(lst_cmd_.begin(), lst_cmd_.end(), find_functor);
//
//    return it;
//}

// std::list<TaskProfile>::iterator ShortLinkTaskManager::__LocateByTask(Task& _task) {
//    for (std::list<TaskProfile>::iterator it = lst_cmd_.begin(); it != lst_cmd_.end(); ++it) {
//        if (it->task.taskid == _task.taskid) {
//            return it;
//        }
//    }
//    return lst_cmd_.end();
//}

void ShortLinkTaskManager::__DeleteShortLink(intptr_t& _running_id) {
    if (!_running_id)
        return;

    ShortLinkInterface* p_shortlink = (ShortLinkInterface*)_running_id;
    //    // p_shortlink->func_add_weak_net_info = NULL;
    //    // p_shortlink->func_weak_net_report = NULL;
    ShortLinkChannelFactory::Destory(p_shortlink);
    //    MessageQueue::CancelMessage(asyncreg_.Get(), p_shortlink);
    p_shortlink = NULL;
}

ConnectProfile ShortLinkTaskManager::GetConnectProfile(uint32_t _taskid) const {
    std::list<CoJobTaskProfile>::const_iterator first = lst_cmd_.begin();
    std::list<CoJobTaskProfile>::const_iterator last = lst_cmd_.end();

    while (first != last) {
        if ((first->task_profile.running_id) && _taskid == first->task_profile.task.taskid) {
            return ((ShortLinkInterface*)(first->task_profile.running_id))->Profile();
        }
        ++first;
    }
    return ConnectProfile();
}

SOCKET ShortLinkTaskManager::__OnGetCacheSocket(const IPPortItem& _address) {
    return socket_pool_.GetSocket(_address);
}

void ShortLinkTaskManager::__OnHandshakeCompleted(uint32_t _version, mars::stn::TlsHandshakeFrom _from) {
    xinfo2(TSF "receive tls version: %_", _version);
    if (already_release_manager_) {
        xinfo2(TSF "mars2 shortlink_task_manager had released. ignore handshake event.");
        return;
    }
    if (on_handshake_ready_) {
        on_handshake_ready_(_version, _from);
    }
}

void ShortLinkTaskManager::__OnRequestTimeout(ShortLinkInterface* _worker, int _errorcode) {
    if (kEctLocalTaskTimeout != _errorcode && _worker) {
        _worker->OnNetTimeout();
    }
}

void ShortLinkTaskManager::__OnAddWeakNetInfo(bool _connect_timeout, struct tcp_info& _info) {
    if (already_release_manager_) {
        xinfo2(TSF "mars2 longlink_task_manager had released. ignore add weaknet info.");
        return;
    }
    if (add_weaknet_info_) {
        add_weaknet_info_(_connect_timeout, _info);
    }
}

bool ShortLinkTaskManager::__CompareTask(const CoJobTaskProfile& _first, const CoJobTaskProfile& _second) {
    return _first.task_profile.task.priority < _second.task_profile.task.priority;
}

void ShortLinkTaskManager::__CoLaunchStartTask(CoJobTaskProfile _task_profile) {
    Task task = _task_profile.task_profile.task;
    std::vector<std::string> hosts = task.shortlink_host_list;
    ShortlinkConfig config(_task_profile.task_profile.use_proxy, /*use_tls=*/true);
#ifndef DISABLE_QUIC_PROTOCOL
    if (!task.quic_host_list.empty() && (task.transport_protocol & Task::kTransportProtocolQUIC)
        && 0 == _task_profile.task_profile.err_code) {
        //.task允许重试且允许走quic，任务也没有出错（首次连接？）,则走quic.
        if (task.retry_count > 0 && net_source_->CanUseQUIC()) {
            config.use_proxy = false;
            config.use_quic = true;
            config.quic.alpn = "h1";
            config.quic.enable_0rtt = true;
            TimeoutSource source;
            config.quic.conn_timeout_ms = net_source_->GetQUICConnectTimeoutMs(task.cgi, &source);
            xinfo2_if(source != TimeoutSource::kClientDefault,
                      TSF "taskid:%_ qctimeout %_ source %_",
                      task.taskid,
                      config.quic.conn_timeout_ms,
                      source);
            hosts = task.quic_host_list;

            _task_profile.task_profile.transfer_profile.connect_profile.quic_conn_timeout_ms =
                config.quic.conn_timeout_ms;
            _task_profile.task_profile.transfer_profile.connect_profile.quic_conn_timeout_source = source;
        } else {
            xwarn2(TSF "taskid:%_ quic disabled or cant retry %_",
                   _task_profile.task_profile.task.taskid,
                   task.retry_count);
        }
    }
#endif
    size_t realhost_cnt = hosts.size();
    if (get_real_host_) {
        _task_profile.task_profile.transfer_profile.begin_first_get_host_time = gettickcount();
        realhost_cnt = get_real_host_(task.user_id, hosts, /*_strict_match=*/config.use_quic);
        _task_profile.task_profile.transfer_profile.end_first_get_host_time = gettickcount();

    } else {
        xwarn2(TSF "mars2 get_real_host_ is null.");
    }

    if (realhost_cnt == 0 && config.use_quic) {
        xwarn2(TSF "taskid:%_ no quic hosts.", _task_profile.task_profile.task.taskid);
        //.使用quic但拿到的host为空（一般是svr部署问题），则回退到tcp.
        config = ShortlinkConfig(_task_profile.task_profile.use_proxy, /*use_tls=*/true);
        hosts = task.shortlink_host_list;
        _task_profile.task_profile.transfer_profile.begin_retry_get_host_time = gettickcount();
        realhost_cnt = get_real_host_(task.user_id, hosts, /*_strict_match=*/false);
        _task_profile.task_profile.transfer_profile.end_retry_get_host_time = gettickcount();
    }

    _task_profile.task_profile.task.shortlink_host_list = hosts;

    if (hosts.empty()) {
        // continue;
        return;
    }
    std::string host = hosts.front();
    //#ifndef DISABLE_QUIC_PROTOCOL
    //    if (config.use_quic) {
    //        config.quic.hostname = host;
    //    }
    //#endif
    xinfo2_if(!_task_profile.task_profile.task.long_polling,
              TSF "need auth cgi %_ , host %_ need auth %_",
              _task_profile.task_profile.task.cgi,
              host,
              _task_profile.task_profile.task.need_authed);
    // make sure login
    if (_task_profile.task_profile.task.need_authed) {
        _task_profile.task_profile.transfer_profile.begin_make_sure_auth_time = gettickcount();
        bool ismakesureauthsuccess =
            context_->GetManager<StnManager>()->MakesureAuthed(host, _task_profile.task_profile.task.user_id);
        _task_profile.task_profile.transfer_profile.end_make_sure_auth_time = gettickcount();
        xinfo2_if(!_task_profile.task_profile.task.long_polling && _task_profile.task_profile.task.priority >= 0,
                  TSF "auth result %_ host %_",
                  ismakesureauthsuccess,
                  host);

        if (!ismakesureauthsuccess) {
            // TODO cpan
            // xinfo2_if(curtime % 3 == 1, TSF "makeSureAuth retsult=%0", ismakesureauthsuccess);
            // first = next;
            // continue;
            return;
        }
    }

    bool use_tls = true;
    if (can_use_tls_) {
        use_tls = !can_use_tls_(hosts);
        xdebug2(TSF "cgi can use tls: %_, host: %_", use_tls, hosts[0]);
    }
    config.use_tls = use_tls;

    AutoBuffer bufreq;
    AutoBuffer buffer_extension;
    int error_code = 0;

    // client_sequence_id 在buf2resp这里生成,防止重试sequence_id一样
    _task_profile.task_profile.task.client_sequence_id = context_->GetManager<StnManager>()->GenSequenceId();
    xinfo2(TSF "client_sequence_id:%_", _task_profile.task_profile.task.client_sequence_id);

    _task_profile.task_profile.transfer_profile.begin_req2buf_time = gettickcount();
    if (!context_->GetManager<StnManager>()->Req2Buf(_task_profile.task_profile.task.taskid,
                                                     _task_profile.task_profile.task.user_context,
                                                     _task_profile.task_profile.task.user_id,
                                                     bufreq,
                                                     buffer_extension,
                                                     error_code,
                                                     Task::kChannelShort,
                                                     host,
                                                     _task_profile.task_profile.task.client_sequence_id)) {
        _task_profile.task_profile.transfer_profile.end_req2buf_time = gettickcount();
        __SingleRespHandle(_task_profile,
                           kEctEnDecode,
                           error_code,
                           kTaskFailHandleTaskEnd,
                           0,
                           _task_profile.task_profile.running_id
                               ? ((ShortLinkInterface*)_task_profile.task_profile.running_id)->Profile()
                               : ConnectProfile());
        // first = next;
        // continue;
        return;
    }
    _task_profile.task_profile.transfer_profile.end_req2buf_time = gettickcount();

    //雪崩检测
    xassert2(fun_anti_avalanche_check_);

    if (!fun_anti_avalanche_check_(_task_profile.task_profile.task, bufreq.Ptr(), (int)bufreq.Length())) {
        __SingleRespHandle(_task_profile,
                           kEctLocal,
                           kEctLocalAntiAvalanche,
                           kTaskFailHandleTaskEnd,
                           0,
                           _task_profile.task_profile.running_id
                               ? ((ShortLinkInterface*)_task_profile.task_profile.running_id)->Profile()
                               : ConnectProfile());
        // first = next;
        // continue;
        return;
    }

    std::string intercept_data;
    if (task_intercept_.GetInterceptTaskInfo(_task_profile.task_profile.task.cgi, intercept_data)) {
        xwarn2(TSF "task has been intercepted");
        AutoBuffer body;
        AutoBuffer extension;
        int err_code = 0;
        unsigned short server_sequence_id = 0;
        body.Write(intercept_data.data(), intercept_data.length());
        _task_profile.task_profile.transfer_profile.received_size = body.Length();
        _task_profile.task_profile.transfer_profile.receive_data_size = body.Length();
        _task_profile.task_profile.transfer_profile.last_receive_pkg_time = ::gettickcount();
        int handle_type = context_->GetManager<StnManager>()->Buf2Resp(_task_profile.task_profile.task.taskid,
                                                                       _task_profile.task_profile.task.user_context,
                                                                       _task_profile.task_profile.task.user_id,
                                                                       body,
                                                                       extension,
                                                                       err_code,
                                                                       Task::kChannelShort,
                                                                       server_sequence_id);
        xinfo2(TSF "server_sequence_id:%_", server_sequence_id);
        _task_profile.task_profile.task.server_sequence_id = server_sequence_id;
        ConnectProfile profile;
        __SingleRespHandle(_task_profile,
                           kEctEnDecode,
                           err_code,
                           handle_type,
                           (unsigned int)_task_profile.task_profile.transfer_profile.receive_data_size,
                           profile);
        // first = next;
        // continue;
        return;
    }

    int sent_count = 0;
    _task_profile.task_profile.transfer_profile.loop_start_task_time = ::gettickcount();
    _task_profile.task_profile.transfer_profile.first_pkg_timeout =
        __FirstPkgTimeout(_task_profile.task_profile.task.server_process_cost,
                          bufreq.Length(),
                          sent_count,
                          dynamic_timeout_.GetStatus());
    _task_profile.task_profile.current_dyntime_status =
        (_task_profile.task_profile.task.server_process_cost <= 0) ? dynamic_timeout_.GetStatus() : kEValuating;
    if (_task_profile.task_profile.transfer_profile.task.long_polling) {
        _task_profile.task_profile.transfer_profile.read_write_timeout =
            __ReadWriteTimeout(_task_profile.task_profile.transfer_profile.task.long_polling_timeout);
    } else {
        _task_profile.task_profile.transfer_profile.read_write_timeout =
            __ReadWriteTimeout(_task_profile.task_profile.transfer_profile.first_pkg_timeout);
    }
    _task_profile.task_profile.transfer_profile.send_data_size = bufreq.Length();

    ShortLinkInterface* worker = ShortLinkChannelFactory::Create(context_,
                                                                 MessageQueue::Handler2Queue(asyncreg_.Get()),
                                                                 net_source_,
                                                                 _task_profile,
                                                                 config);
    worker->func_host_filter = get_real_host_strict_match_;
    worker->func_add_weak_net_info =
        std::bind(&ShortLinkTaskManager::__OnAddWeakNetInfo, this, std::placeholders::_1, std::placeholders::_2);
    // worker->OnSend.set(boost::bind(&ShortLinkTaskManager::__OnSend, this, _1), worker, AYNC_HANDLER);
    // worker->OnRecv.set(boost::bind(&ShortLinkTaskManager::__OnRecv, this, _1, _2, _3), worker, AYNC_HANDLER);
    //            worker->OnResponse.set(boost::bind(&ShortLinkTaskManager::__OnResponse, this, _1, _2, _3, _4,
    //            _5, _6, _7),
    //                                   worker,
    //                                   AYNC_HANDLER);
    worker->OnSend = std::bind(&ShortLinkTaskManager::__OnSend, this, std::placeholders::_1);
    worker->OnRecv = std::bind(&ShortLinkTaskManager::__OnRecv,
                               this,
                               std::placeholders::_1,
                               std::placeholders::_2,
                               std::placeholders::_3);
    worker->OnResponse = std::bind(&ShortLinkTaskManager::__OnResponse,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2,
                                   std::placeholders::_3,
                                   std::placeholders::_4,
                                   std::placeholders::_5,
                                   std::placeholders::_6,
                                   std::placeholders::_7);

    worker->GetCacheSocket = boost::bind(&ShortLinkTaskManager::__OnGetCacheSocket, this, _1);
    worker->OnHandshakeCompleted = boost::bind(&ShortLinkTaskManager::__OnHandshakeCompleted, this, _1, _2);

    if (!debug_host_.empty()) {
        worker->SetDebugHost(debug_host_);
    }

    _task_profile.task_profile.running_id = (intptr_t)worker;

    xassert2(worker && _task_profile.task_profile.running_id);
    if (!_task_profile.task_profile.running_id) {
        xwarn2(TSF "task add into shortlink readwrite fail cgi:%_, cmdid:%_, taskid:%_",
               _task_profile.task_profile.task.cgi,
               _task_profile.task_profile.task.cmdid,
               _task_profile.task_profile.task.taskid);
        // first = next;
        // continue;
        return;
    }

    worker->func_network_report.set(fun_notify_network_err_);
    if (choose_protocol_) {
        worker->SetUseProtocol(choose_protocol_(_task_profile.task_profile));
    }

    /*work:%_,*/
    xinfo2_if(_task_profile.task_profile.task.priority >= 0,
              TSF
              "task add into shortlink readwrite cgi:%_, cmdid:%_, taskid:%_, size:%_, "
              "timeout(firstpkg:%_, rw:%_, task:%_), retry:%_, long-polling:%_, useProxy:%_, tls:%_",
              _task_profile.task_profile.task.cgi,
              _task_profile.task_profile.task.cmdid,
              _task_profile.task_profile.task.taskid,
              //              (ShortLinkInterface*)_task_profile.task_profile.running_id,
              _task_profile.task_profile.transfer_profile.send_data_size,
              _task_profile.task_profile.transfer_profile.first_pkg_timeout / 1000,
              _task_profile.task_profile.transfer_profile.read_write_timeout / 1000,
              _task_profile.task_profile.task_timeout / 1000,
              _task_profile.task_profile.remain_retry_count,
              _task_profile.task_profile.task.long_polling,
              _task_profile.task_profile.use_proxy,
              use_tls);

    //这里跑完就完了,所以log要在前面打印
    worker->SendRequest(bufreq, buffer_extension);
}

bool ShortLinkTaskManager::__SingleRespHandle(CoJobTaskProfile& co_task_profile,
                                              ErrCmdType _err_type,
                                              int _err_code,
                                              int _fail_handle,
                                              size_t _resp_length,
                                              const ConnectProfile& _connect_profile) {
    xverbose_function();
    xassert2(kEctServer != _err_type);

    if (kEctOK == _err_type) {
        tasks_continuous_fail_count_ = 0;
        default_use_proxy_ = co_task_profile.task_profile.use_proxy;
    } else {
        ++tasks_continuous_fail_count_;
    }

    uint64_t curtime = gettickcount();
    co_task_profile.task_profile.transfer_profile.connect_profile = _connect_profile;

    xassert2((kEctOK == _err_type) == (kTaskFailHandleNoError == _fail_handle),
             TSF "type:%_, handle:%_",
             _err_type,
             _fail_handle);

    if (co_task_profile.task_profile.force_no_retry || 0 >= co_task_profile.task_profile.remain_retry_count
        || kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle
        || kTaskFailHandleTaskTimeout == _fail_handle) {
        xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn,
              TSF "task end callback short cmdid:%_, err(%_, %_, %_), ",
              co_task_profile.task_profile.task.cmdid,
              _err_type,
              _err_code,
              _fail_handle)(TSF "svr(%_:%_, %_, %_), ",
                            _connect_profile.ip,
                            _connect_profile.port,
                            IPSourceTypeString[_connect_profile.ip_type],
                            _connect_profile.host)(TSF "cli(%_, %_, %_, n:%_, sig:%_), ",
                                                   co_task_profile.task_profile.transfer_profile.external_ip,
                                                   _connect_profile.local_ip,
                                                   _connect_profile.connection_identify,
                                                   _connect_profile.net_type,
                                                   _connect_profile.disconn_signal)(
            TSF "cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ",
            co_task_profile.task_profile.transfer_profile.send_data_size,
            0 != _resp_length ? _resp_length : co_task_profile.task_profile.transfer_profile.receive_data_size,
            0 != _resp_length ? "" : "/",
            0 != _resp_length ? "" : string_cast(co_task_profile.task_profile.transfer_profile.received_size).str(),
            _connect_profile.conn_rtt,
            (co_task_profile.task_profile.transfer_profile.start_send_time == 0
                 ? 0
                 : curtime - co_task_profile.task_profile.transfer_profile.start_send_time),
            (curtime - co_task_profile.task_profile.start_task_time),
            co_task_profile.task_profile.remain_retry_count)(
            TSF "cgi:%_, taskid:%_, worker:%_, context id:%_",
            co_task_profile.task_profile.task.cgi,
            co_task_profile.task_profile.task.taskid,
            (ShortLinkInterface*)co_task_profile.task_profile.running_id,
            co_task_profile.task_profile.task.user_id);

        if (_err_type != kEctOK && _err_type != kEctServer) {
            xinfo_trace(TSF "cgi trace error: (%_, %_), cost:%_, rtt:%_, svr:(%_, %_, %_)",
                        _err_type,
                        _err_code,
                        (curtime - co_task_profile.task_profile.start_task_time),
                        _connect_profile.conn_rtt,
                        _connect_profile.ip,
                        _connect_profile.port,
                        IPSourceTypeString[_connect_profile.ip_type]);
        }

        if (task_connection_detail_) {
            task_connection_detail_(_err_type, _err_code, _connect_profile.ip_index);
        }

        int cgi_retcode = fun_callback_(_err_type,
                                        _err_code,
                                        _fail_handle,
                                        co_task_profile.task_profile.task,
                                        (unsigned int)(curtime - co_task_profile.task_profile.start_task_time));
        int errcode = _err_code;

        if (co_task_profile.task_profile.running_id) {
            if (kEctOK == _err_type) {
                errcode = cgi_retcode;
            }
        }

        co_task_profile.task_profile.end_task_time = ::gettickcount();
        co_task_profile.task_profile.err_type = _err_type;
        co_task_profile.task_profile.transfer_profile.error_type = _err_type;
        co_task_profile.task_profile.err_code = errcode;
        co_task_profile.task_profile.transfer_profile.error_code = _err_code;

        co_task_profile.task_profile.PushHistory();
        if (on_timeout_or_remote_shutdown_) {
            on_timeout_or_remote_shutdown_(co_task_profile.task_profile);
        }
        co_task_profile.task_profile.is_weak_network = net_source_->GetWeakNetworkLogic()->IsCurrentNetworkWeak();
        int64_t span = 0;
        co_task_profile.task_profile.is_last_valid_connect_fail =
            net_source_->GetWeakNetworkLogic()->IsLastValidConnectFail(span);
        /* mars2
        ReportTaskProfile(*_it);
        */
        context_->GetManager<StnManager>()->ReportTaskProfile(co_task_profile.task_profile);
        // WeakNetworkLogic::Singleton::Instance()->OnTaskEvent(_task_profile);
        net_source_->GetWeakNetworkLogic()->OnTaskEvent(co_task_profile.task_profile);
        __DeleteShortLink(co_task_profile.task_profile.running_id);

        // TODO cpan

        //__EraseCMD(task_profile);
        // lst_cmd_.erase();

        // lst_cmd_.erase(<#const_iterator p#>) (task_profile);

        owl_task_scope->async([this, co_task_profile] {
            std::list<CoJobTaskProfile>::iterator first = lst_cmd_.begin();
            std::list<CoJobTaskProfile>::iterator last = lst_cmd_.end();
            while (first != last) {
                if (co_task_profile.task_profile.task.taskid == first->task_profile.task.taskid) {
                    xinfo2(TSF "cpan erase task cgi:%_", co_task_profile.task_profile.task.cgi);
                    // first->job= nullptr;

                    lst_cmd_.erase(first);

                    break;
                }
                ++first;
            }
        });
        return true;
    }

    xlog2(kEctOK == _err_type ? kLevelInfo : kLevelWarn,
          TSF "task end retry short cmdid:%_, err(%_, %_, %_), ",
          co_task_profile.task_profile.task.cmdid,
          _err_type,
          _err_code,
          _fail_handle)(TSF "svr(%_:%_, %_, %_), ",
                        _connect_profile.ip,
                        _connect_profile.port,
                        IPSourceTypeString[_connect_profile.ip_type],
                        _connect_profile.host)(TSF "cli(%_, %_, %_, n:%_, sig:%_), ",
                                               co_task_profile.task_profile.transfer_profile.external_ip,
                                               _connect_profile.local_ip,
                                               _connect_profile.connection_identify,
                                               _connect_profile.net_type,
                                               _connect_profile.disconn_signal)(
        TSF "cost(s:%_, r:%_%_%_, c:%_, rw:%_), all:%_, retry:%_, ",
        co_task_profile.task_profile.transfer_profile.send_data_size,
        0 != _resp_length ? _resp_length : co_task_profile.task_profile.transfer_profile.received_size,
        0 != _resp_length ? "" : "/",
        0 != _resp_length ? "" : string_cast(co_task_profile.task_profile.transfer_profile.receive_data_size).str(),
        _connect_profile.conn_rtt,
        (co_task_profile.task_profile.transfer_profile.start_send_time == 0
             ? 0
             : curtime - co_task_profile.task_profile.transfer_profile.start_send_time),
        (curtime - co_task_profile.task_profile.start_task_time),
        co_task_profile.task_profile.remain_retry_count)(TSF "cgi:%_, taskid:%_, worker:%_",
                                                         co_task_profile.task_profile.task.cgi,
                                                         co_task_profile.task_profile.task.taskid,
                                                         (void*)co_task_profile.task_profile.running_id);

    co_task_profile.task_profile.remain_retry_count--;
    co_task_profile.task_profile.transfer_profile.error_type = _err_type;
    co_task_profile.task_profile.transfer_profile.error_code = _err_code;
    co_task_profile.task_profile.err_type = _err_type;
    co_task_profile.task_profile.err_code = _err_code;
    __DeleteShortLink(co_task_profile.task_profile.running_id);
    co_task_profile.task_profile.PushHistory();
    if (on_timeout_or_remote_shutdown_) {
        on_timeout_or_remote_shutdown_(co_task_profile.task_profile);
    }
    co_task_profile.task_profile.InitSendParam();

    co_task_profile.task_profile.retry_start_time = ::gettickcount();
    // session timeout 应该立刻重试
    if (kTaskFailHandleSessionTimeout == _fail_handle) {
        co_task_profile.task_profile.retry_start_time = 0;
    }
    // .quic失败立刻换tcp重试.
    if (_connect_profile.transport_protocol == Task::kTransportProtocolQUIC) {
        xwarn2(TSF "taskid:%_ quic failed, retry with tcp immediately.", co_task_profile.task_profile.task.taskid);
        co_task_profile.task_profile.retry_start_time = 0;
    }

    // TODO cpan 重试间隔
    co_task_profile.task_profile.retry_time_interval = DEF_TASK_RETRY_INTERNAL;

    return false;
}
