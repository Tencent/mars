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
 * longlink.cc
 *
 *  Created on: 2014-2-27
 *      Author: yerungui
 */

#include "longlink.h"

#include <algorithm>

#include "boost/bind.hpp"

#include "mars/app/app.h"
#include "mars/baseevent/active_logic.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/socket/complexconnect.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/baseevent/baseprjevent.h"

#if defined(__ANDROID__) || defined(__APPLE__)
#include "mars/comm/socket/getsocktcpinfo.h"
#endif

#include "mars/stn/config.h"

#include "proto/longlink_packer.h"
#include "smart_heartbeat.h"

#define AYNC_HANDLER  asyncreg_.Get()
#define STATIC_RETURN_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

using namespace mars::stn;
using namespace mars::app;

namespace {
class LongLinkConnectObserver : public MComplexConnect {
  public:
    LongLinkConnectObserver(LongLink& _longlink, const std::vector<IPPortItem>& _iplist): longlink_(_longlink), ip_items_(_iplist) {
    	memset(connecting_index_, 0, sizeof(connecting_index_));
    };

    virtual void OnCreated(unsigned int _index, const socket_address& _addr, SOCKET _socket) {}
    virtual void OnConnect(unsigned int _index, const socket_address& _addr, SOCKET _socket)  {
    	connecting_index_[_index] = 1;
    }
    virtual void OnConnected(unsigned int _index, const socket_address& _addr, SOCKET _socket, int _error, int _rtt) {
        if (0 == _error) {
            if (!OnShouldVerify(_index, _addr)) {
                connecting_index_[_index] = 0;
            }
        } else {
            xwarn2(TSF"index:%_, connnet fail host:%_, iptype:%_", _index, ip_items_[_index].str_host, ip_items_[_index].source_type);
            xassert2(longlink_.fun_network_report_);

            if (longlink_.fun_network_report_) {
                longlink_.fun_network_report_(__LINE__, kEctSocket, _error, _addr.ip(), _addr.port());
            }
        }
    }

    virtual bool OnShouldVerify(unsigned int _index, const socket_address& _addr) {
        return longlink_complexconnect_need_verify();
    }
    
    virtual bool OnVerifySend(unsigned int _index, const socket_address& _addr, SOCKET _socket, AutoBuffer& _buffer_send) {
        AutoBuffer body;
        longlink_noop_req_body(body);
        longlink_pack(longlink_noop_cmdid(), Task::kNoopTaskID, body.Ptr(), body.Length(), _buffer_send);
        return true;
    }

    virtual bool OnVerifyRecv(unsigned int _index, const socket_address& _addr, SOCKET _socket, const AutoBuffer& _buffer_recv) {
        uint32_t cmdid = 0;
        uint32_t  taskid = Task::kInvalidTaskID;
        size_t pack_len = 0;
        AutoBuffer bufferbody;
        int ret = longlink_unpack(_buffer_recv, cmdid, taskid, pack_len, bufferbody);

        if (LONGLINK_UNPACK_OK != ret) {
            xerror2(TSF"0>ret, index:%_, sock:%_, %_, ret:%_, cmdid:%_, taskid:%_, pack_len:%_, recv_len:%_", _index, _socket, _addr.url(), ret, cmdid, taskid, pack_len, _buffer_recv.Length());
            return false;
        }

        if (Task::kNoopTaskID != taskid) {
            xwarn2(TSF"index:%_, sock:%_, %_, ret:%_, cmdid:%_, taskid:%_, pack_len:%_, recv_len:%_", _index, _socket, _addr.url(), ret, cmdid, taskid, pack_len, _buffer_recv.Length());

        }

        if (longlink_noop_resp_cmdid() != cmdid) {
            xwarn2(TSF"index:%_, sock:%_, %_, ret:%_, cmdid:%_, taskid:%_, pack_len:%_, recv_len:%_", _index, _socket, _addr.url(), ret, cmdid, taskid, pack_len, _buffer_recv.Length());
        }

        connecting_index_[_index] = 0;
        return true;
    }
    char connecting_index_[32];

  private:
    LongLinkConnectObserver(const LongLinkConnectObserver&);
    LongLinkConnectObserver& operator=(const LongLinkConnectObserver&);

  public:
    LongLink& longlink_;
    const std::vector<IPPortItem>& ip_items_;
};

}

LongLink::LongLink(NetSource& _netsource, MessageQueue::MessageQueue_t _messagequeueid)
    : asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
    , netsource_(_netsource)
    , thread_(boost::bind(&LongLink::__Run, this), XLOGGER_TAG "::lonklink")
#ifdef ANDROID
    , smartheartbeat_(new SmartHeartbeat)
#else
    , smartheartbeat_(NULL)
#endif
	, connectstatus_(kConnectIdle)
	, disconnectinternalcode_(kNone)
{}

LongLink::~LongLink() {
    Disconnect(kReset);
    asyncreg_.CancelAndWait();
    if (NULL != smartheartbeat_) {
    	delete smartheartbeat_, smartheartbeat_=NULL;
    }
}

bool LongLink::Send(const unsigned char* _pbuf, size_t _len, uint32_t _cmdid, uint32_t _taskid, const std::string& _task_info) {
    ScopedLock lock(mutex_);

    if (kConnected != connectstatus_) return false;

    return __Send(_pbuf, _len, _cmdid, _taskid, _task_info);
}

bool LongLink::SendWhenNoData(const unsigned char* _pbuf, size_t _len, uint32_t _cmdid, uint32_t _taskid) {
    ScopedLock lock(mutex_);

    if (kConnected != connectstatus_) return false;
    if (!lstsenddata_.empty()) return false;

    return __Send(_pbuf, _len, _cmdid, _taskid, "");
}

bool LongLink::Stop(uint32_t _taskid) {
    ScopedLock lock(mutex_);

    for (std::list<LongLinkSendData>::iterator it = lstsenddata_.begin(); it != lstsenddata_.end(); ++it) {
        if (_taskid == it->taskid && 0 == it->data.Pos()) {
            lstsenddata_.erase(it);
            return true;
        }
    }

    return false;
}

bool LongLink::__Send(const unsigned char* _pbuf, size_t _len, uint32_t _cmdid, uint32_t _taskid, const std::string& _task_info) {
    lstsenddata_.push_back(LongLinkSendData());

    lstsenddata_.back().cmdid = _cmdid;
    lstsenddata_.back().taskid = _taskid;
    longlink_pack(_cmdid, _taskid, _pbuf, _len, lstsenddata_.back().data);
    lstsenddata_.back().data.Seek(0, AutoBuffer::ESeekStart);
    lstsenddata_.back().task_info = _task_info;

    readwritebreak_.Break();
    return true;
}

bool LongLink::MakeSureConnected(bool* _newone) {
    if (_newone) *_newone = false;

    ScopedLock lock(mutex_);

    if (kConnected == ConnectStatus()) return true;

    bool newone = false;
    thread_.start(&newone);

    if (newone) {
        connectstatus_ = kConnectIdle;
        conn_profile_.Reset();
        identifychecker_.Reset();
        disconnectinternalcode_ = kNone;
        readwritebreak_.Clear();
        connectbreak_.Clear();
    }

    if (_newone) *_newone = newone;

    return false;
}

void LongLink::Disconnect(TDisconnectInternalCode _scene) {
    xinfo2(TSF"_scene:%_", _scene);
    
    ScopedLock lock(mutex_);
    lstsenddata_.clear();

    if (!thread_.isruning()) return;

    disconnectinternalcode_ = _scene;

    bool recreate = false;

    if (!readwritebreak_.Break() || !connectbreak_.Break()) {
        xassert2(false, "breaker fail");
        connectbreak_.Close();
        readwritebreak_.Close();
        recreate = true;
    }
    lock.unlock();
    
    dns_util_.Cancel();
    thread_.join();

    if (recreate) {
        connectbreak_.ReCreate();
        readwritebreak_.ReCreate();
    }
}

bool LongLink::__NoopReq(XLogger& _log, Alarm& _alarm, bool need_active_timeout) {
    AutoBuffer buffer;
    uint32_t req_cmdid = 0;
    bool suc = false;
    
    if (identifychecker_.GetIdentifyBuffer(buffer, req_cmdid)) {
        suc = Send((const unsigned char*)buffer.Ptr(), (int)buffer.Length(), req_cmdid, Task::kLongLinkIdentifyCheckerTaskID);
        identifychecker_.SetSeq(Task::kLongLinkIdentifyCheckerTaskID);
        xinfo2(TSF"start noop synccheck taskid:%0, cmdid:%1, ", Task::kLongLinkIdentifyCheckerTaskID, req_cmdid) >> _log;
    } else {
        AutoBuffer body;
        longlink_noop_req_body(body);
        suc = SendWhenNoData((const unsigned char*) body.Ptr(), body.Length(), longlink_noop_cmdid(), Task::kNoopTaskID);
        xinfo2(TSF"start noop taskid:%0, cmdid:%1, ", Task::kNoopTaskID, longlink_noop_cmdid()) >> _log;
    }
    
    if (suc) {
        _alarm.Cancel();
        _alarm.Start(need_active_timeout ? (2* 1000) : (10 * 1000));
    } else {
        xerror2("send noop fail");
    }
    
    return suc;
}

bool LongLink::__NoopResp(uint32_t _cmdid, uint32_t _taskid, AutoBuffer& _buf, Alarm& _alarm, ConnectProfile& _profile) {
    bool is_noop = false;
    
    if (identifychecker_.IsIdentifyResp(_taskid)) {
        xinfo2(TSF"end noop synccheck");
        is_noop = true;
        if (identifychecker_.OnIdentifyResp(_buf)) {
            fun_network_report_(__LINE__, kEctOK, 0, _profile.ip, _profile.port);
        }
    }
    
    if (_cmdid == longlink_noop_resp_cmdid() && Task::kNoopTaskID == _taskid) {
        longlink_noop_resp_body(_buf);
        xinfo2(TSF"end noop");
        is_noop = true;
    }
    else {
        xassert2(_cmdid != longlink_noop_resp_cmdid() && Task::kNoopTaskID != _taskid);
    }
    
    if (is_noop) {
        _alarm.Cancel();
        __NotifySmartHeartbeatHeartResult(true, false, _profile);
#ifdef ANDROID
        wakelock_.Lock(500);
#endif
    }
    
    return is_noop;
}

void LongLink::__RunResponseError(ErrCmdType _error_type, int _error_code, ConnectProfile& _profile, bool _networkreport) {
    ScopedLock lock(mutex_);
    lstsenddata_.clear();
    lock.unlock();

    AutoBuffer buf;
    OnResponse(_error_type, _error_code, 0, Task::kInvalidTaskID, buf, _profile);
    xassert2(fun_network_report_);

    if (_networkreport && fun_network_report_) fun_network_report_(__LINE__, _error_type, _error_code, _profile.ip, _profile.port);
}

LongLink::TLongLinkStatus LongLink::ConnectStatus() const {
    return connectstatus_;
}

void LongLink::__ConnectStatus(TLongLinkStatus _status) {
    if (_status == connectstatus_) return;
    xinfo2(TSF"connect status from:%0 to:%1, nettype:%_", connectstatus_, _status, ::getNetInfo());
    connectstatus_ = _status;
    __NotifySmartHeartbeatConnectStatus(connectstatus_);
    STATIC_RETURN_SYNC2ASYNC_FUNC(boost::bind(boost::ref(SignalConnection), connectstatus_));
}

void LongLink::__UpdateProfile(const ConnectProfile& _conn_profile) {
    STATIC_RETURN_SYNC2ASYNC_FUNC(boost::bind(&LongLink::__UpdateProfile, this, _conn_profile));
    conn_profile_ = _conn_profile;
    
    if (0 != conn_profile_.disconn_time) broadcast_linkstatus_signal_(conn_profile_);
}


void LongLink::__OnAlarm() {
    readwritebreak_.Break();
#ifdef ANDROID
    __NotifySmartHeartbeatJudgeMIUIStyle();
    wakelock_.Lock(3 * 1000);
#endif
}

void LongLink::__Run() {
    // sync to MakeSureConnected data reset
    {
        ScopedLock lock(mutex_);
    }
    
    uint64_t cur_time = gettickcount();
    xinfo_function(TSF"LongLink Rebuild span:%_, net:%_", conn_profile_.disconn_time != 0 ? cur_time - conn_profile_.disconn_time : 0, getNetInfo());
    
    ConnectProfile conn_profile;
    conn_profile.start_time = cur_time;
    conn_profile.conn_reason = conn_profile_.disconn_errcode;
    getCurrNetLabel(conn_profile.net_type);
    conn_profile.tid = xlogger_tid();
    __UpdateProfile(conn_profile);
    
#ifdef ANDROID
    wakelock_.Lock(30 * 1000);
#endif
    SOCKET sock = __RunConnect(conn_profile);
#ifdef ANDROID
    wakelock_.Lock(1000);
#endif
    
    if (INVALID_SOCKET == sock) {
        conn_profile.disconn_time = ::gettickcount();
        conn_profile.disconn_signal = ::getSignal(::getNetInfo() == kWifi);
        __UpdateProfile(conn_profile);
        return;
    }
    
    ErrCmdType errtype = kEctOK;
    int errcode = 0;
    __RunReadWrite(sock, errtype, errcode, conn_profile);
    
    socket_close(sock);
    
    conn_profile.disconn_time = ::gettickcount();
    conn_profile.disconn_errtype = errtype;
    conn_profile.disconn_errcode = errcode;
    conn_profile.disconn_signal = ::getSignal(::getNetInfo() == kWifi);
    
    __ConnectStatus(kDisConnected);
    __UpdateProfile(conn_profile);

    if (kEctOK != errtype) __RunResponseError(errtype, errcode, conn_profile);
    
#ifdef ANDROID
    wakelock_.Lock(1000);
#endif
}

SOCKET LongLink::__RunConnect(ConnectProfile& _conn_profile) {
    
    __ConnectStatus(kConnecting);
    _conn_profile.dns_time = ::gettickcount();
     __UpdateProfile(_conn_profile);
    
    std::vector<IPPortItem> ip_items;
    std::vector<socket_address> vecaddr;

    netsource_.GetLongLinkItems(ip_items, dns_util_);
    xinfo2(TSF"task socket dns ip:%_", NetSource::DumpTable(ip_items));
    
    bool isnat64 = ELocalIPStack_IPv6 == local_ipstack_detect();
    
    for (unsigned int i = 0; i < ip_items.size(); ++i) {
        vecaddr.push_back(socket_address(ip_items[i].str_ip.c_str(), ip_items[i].port).v4tov6_address(isnat64));
    }
    
    if (vecaddr.empty()) {
        xerror2("task socket close sock:-1 vecaddr empty");
        __ConnectStatus(kConnectFailed);
        __RunResponseError(kEctDns, kEctDnsMakeSocketPrepared, _conn_profile);
        return INVALID_SOCKET;
    }
    
    _conn_profile.ip_items = ip_items;
    _conn_profile.host = ip_items[0].str_host;
    _conn_profile.ip_type = ip_items[0].source_type;
    _conn_profile.ip = ip_items[0].str_ip;
    _conn_profile.port = ip_items[0].port;
    _conn_profile.nat64 = isnat64;
    _conn_profile.dns_endtime = ::gettickcount();
    __UpdateProfile(_conn_profile);
    
    // set the first ip info to the profiler, after connect, the ip info will be overwrriten by the real one
    
    LongLinkConnectObserver connect_observer(*this, ip_items);
    ComplexConnect com_connect(kLonglinkConnTimeout, kLonglinkConnInteral, kLonglinkConnInteral, kLonglinkConnMax);
    SOCKET sock = com_connect.ConnectImpatient(vecaddr, connectbreak_, &connect_observer);
    
    _conn_profile.conn_time = gettickcount();
    _conn_profile.conn_errcode = com_connect.ErrorCode();
    _conn_profile.conn_rtt = com_connect.IndexRtt();
    _conn_profile.conn_cost = com_connect.TotalCost();
    _conn_profile.tryip_count = com_connect.TryCount();
    __UpdateProfile(_conn_profile);
    
    if (INVALID_SOCKET == sock) {
        xwarn2(TSF"task socket connect fail sock:-1, costtime:%0", com_connect.TotalCost());
        
        __ConnectStatus(kConnectFailed);
        
        if (kNone == disconnectinternalcode_) __RunResponseError(kEctSocket, kEctSocketMakeSocketPrepared, _conn_profile, false);
        
        
        return INVALID_SOCKET;
    }
    
    xassert2(0 <= com_connect.Index() && (unsigned int)com_connect.Index() < ip_items.size());
    
    if (fun_network_report_) {
        for (int i = 0; i < com_connect.Index(); ++i) {
            if (1 == connect_observer.connecting_index_[i])
                fun_network_report_(__LINE__, kEctSocket, SOCKET_ERRNO(ETIMEDOUT), ip_items[i].str_ip, ip_items[i].port);
        }
    }
    
    _conn_profile.ip_index = com_connect.Index();
    _conn_profile.host = ip_items[com_connect.Index()].str_host;
    _conn_profile.ip_type = ip_items[com_connect.Index()].source_type;
    _conn_profile.ip = ip_items[com_connect.Index()].str_ip;
    _conn_profile.port = ip_items[com_connect.Index()].port;
    _conn_profile.local_ip = socket_address::getsockname(sock).ip();
    
    xinfo2(TSF"task socket connect suc sock:%_, host:%_, ip:%_, port:%_, iptype:%_, costtime:%_, rtt:%_, totalcost:%_, index:%_, net:%_",
           sock, _conn_profile.host, _conn_profile.ip, _conn_profile.port, IPSourceTypeString[_conn_profile.ip_type], com_connect.TotalCost(), com_connect.IndexRtt(), com_connect.IndexTotalCost(), com_connect.Index(), ::getNetInfo());
    __ConnectStatus(kConnected);
    __UpdateProfile(_conn_profile);
    
    xerror2_if(0 != socket_disable_nagle(sock, 1), TSF"socket_disable_nagle sock:%0, %1(%2)", sock, socket_errno, socket_strerror(socket_errno));
    
    //    struct linger so_linger;
    //    so_linger.l_onoff = 1;
    //    so_linger.l_linger = 0;
    
    //    xerror2_if(0 != setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger)),
    //               TSF"SO_LINGER sock:%0, %1(%2)", sock, socket_errno, socket_strerror(socket_errno));
    
    return sock;
}

void LongLink::__RunReadWrite(SOCKET _sock, ErrCmdType& _errtype, int& _errcode, ConnectProfile& _profile) {
    AutoBuffer bufrecv;
    
    bool first_noop_sent = false;
    
    Alarm alarmnoopinterval(boost::bind(&LongLink::__OnAlarm, this), false);
    Alarm alarmnooptimeout(boost::bind(&LongLink::__OnAlarm, this), false);
    
    std::map <unsigned int, std::string> sent_taskids;
    std::vector<LongLinkNWriteData> nsent_datas;
    bool is_noop = false;
    xgroup2_define(close_log);
    
    while (true) {
        if (!alarmnoopinterval.IsWaiting()) {
            if (first_noop_sent && alarmnoopinterval.Status() != Alarm::kOnAlarm) {
                xassert2(false, "noop interval alarm not running");
            }
            
            xgroup2_define(noop_xlog);
            uint64_t last_noop_interval = alarmnoopinterval.After();
            uint64_t last_noop_actual_interval = (alarmnoopinterval.Status() == Alarm::kOnAlarm) ? alarmnoopinterval.ElapseTime() : 0;
            bool has_late_toomuch = (last_noop_actual_interval >= (15*60*1000));
            
            if (__NoopReq(noop_xlog, alarmnooptimeout, has_late_toomuch)) {
                is_noop = true;
                __NotifySmartHeartbeatHeartReq(_profile, last_noop_interval, last_noop_actual_interval);
            }
            
            first_noop_sent = true;
            
            uint64_t noop_interval = __GetNextHeartbeatInterval();
            xinfo2(TSF" last:(%_,%_), next:%_", last_noop_interval, last_noop_actual_interval, noop_interval) >> noop_xlog;
            alarmnoopinterval.Cancel();
            alarmnoopinterval.Start((int)noop_interval);
        }
        
        if (is_noop && (alarmnooptimeout.Status() == Alarm::kInit || alarmnooptimeout.Status() == Alarm::kCancel)) {
            xassert2(false, "noop but alarmnooptimeout not running, take as noop timeout");
            _errtype = kEctSocket;
            _errcode = kEctSocketRecvErr;
            goto End;
        }
        
        SocketSelect sel(readwritebreak_, true);
        sel.PreSelect();
        sel.Read_FD_SET(_sock);
        sel.Exception_FD_SET(_sock);
        
        ScopedLock lock(mutex_);
        
        if (!lstsenddata_.empty()) sel.Write_FD_SET(_sock);
        
        lock.unlock();
        
        int retsel = sel.Select(10 * 60 * 1000);
        
        if (kNone != disconnectinternalcode_) {
            xwarn2(TSF"task socket close sock:%0, user disconnect:%1, nread:%_, nwrite:%_", _sock, disconnectinternalcode_, socket_nread(_sock), socket_nwrite(_sock)) >> close_log;
            goto End;
        }
        
        if (0 > retsel) {
            xfatal2(TSF"task socket close sock:%0, 0 > retsel, errno:%_, nread:%_, nwrite:%_", _sock, sel.Errno(), socket_nread(_sock), socket_nwrite(_sock)) >> close_log;
            _errtype = kEctSocket;
            _errcode = sel.Errno();
            goto End;
        }
        
        if (sel.IsException()) {
            xerror2(TSF"task socket close sock:%0, socketselect excptoin:%1(%2), nread:%_, nwrite:%_", _sock, socket_errno, socket_strerror(socket_errno), socket_nread(_sock), socket_nwrite(_sock)) >> close_log;
            _errtype = kEctSocket;
            _errcode = socket_errno;
            goto End;
        }
        
        if (sel.Exception_FD_ISSET(_sock)) {
            int error = socket_error(_sock);
            xerror2(TSF"task socket close sock:%0, excptoin:%1(%2), nread:%_, nwrite:%_", _sock, error, socket_strerror(error), socket_nread(_sock), socket_nwrite(_sock)) >> close_log;
            _errtype = kEctSocket;
            _errcode = error;
            goto End;
        }
        
        if (is_noop && alarmnooptimeout.Status() == Alarm::kOnAlarm) {
            xerror2(TSF"task socket close sock:%0, noop timeout, nread:%_, nwrite:%_", _sock, socket_nread(_sock), socket_nwrite(_sock)) >> close_log;
            _errtype = kEctSocket;
            _errcode = kEctSocketRecvErr;
            goto End;
        }
        
        lock.lock();
        if (socket_nwrite(_sock) == 0 && !nsent_datas.empty()) {
            nsent_datas.clear();
        }
        
        if (sel.Write_FD_ISSET(_sock) && !lstsenddata_.empty()) {
            xgroup2_define(xlog_group);
            xinfo2(TSF"task socket send sock:%0, ", _sock) >> xlog_group;
            
#ifndef WIN32
            iovec* vecwrite = (iovec*)calloc(lstsenddata_.size(), sizeof(iovec));
            unsigned int offset = 0;
            
            for (std::list<LongLinkSendData>::iterator it = lstsenddata_.begin(); it != lstsenddata_.end(); ++it) {
                vecwrite[offset].iov_base = it->data.PosPtr();
                vecwrite[offset].iov_len = it->data.PosLength();
                
                ++offset;
            }
            
            ssize_t writelen = writev(_sock, vecwrite, (int)lstsenddata_.size());
            
            free(vecwrite);
#else
            ssize_t writelen = ::send(_sock, lstsenddata_.begin()->data.PosPtr(), lstsenddata_.begin()->data.PosLength(), 0);
#endif
            
            if (0 == writelen || (0 > writelen && !IS_NOBLOCK_SEND_ERRNO(socket_errno))) {
                int error = socket_error(_sock);
                
                _errtype = kEctSocket;
                _errcode = error;
                xerror2(TSF"sock:%0, send:%1(%2)", _sock, error, socket_strerror(error)) >> xlog_group;
                goto End;
            }
            
            if (0 > writelen) writelen = 0;
            
            unsigned long long noop_interval = __GetNextHeartbeatInterval();
            alarmnoopinterval.Cancel();
            alarmnoopinterval.Start((int)noop_interval);
            
            
            xinfo2(TSF"all send:%_, count:%_, ", writelen, lstsenddata_.size()) >> xlog_group;
            
            GetSignalOnNetworkDataChange()(XLOGGER_TAG, writelen, 0);
            
            std::list<LongLinkSendData>::iterator it = lstsenddata_.begin();
            
            while (it != lstsenddata_.end() && 0 < writelen) {
                if (0 == it->data.Pos()) OnSend(it->taskid);
                
                if ((size_t)writelen >= it->data.PosLength()) {
                    xinfo2(TSF"sub send taskid:%_, cmdid:%_, %_, len(S:%_, %_/%_), ", it->taskid, it->cmdid, it->task_info, it->data.PosLength(), it->data.PosLength(), it->data.Length()) >> xlog_group;
                    writelen -= it->data.PosLength();
                    if (!it->task_info.empty()) sent_taskids[it->taskid] = it->task_info;
                    LongLinkNWriteData nwrite(it->taskid, it->data.PosLength(), it->cmdid, it->task_info);
                    nsent_datas.push_back(nwrite);
                    
                    it = lstsenddata_.erase(it);
                } else {
                    xinfo2(TSF"sub send taskid:%_, cmdid:%_, %_, len(S:%_, %_/%_), ", it->taskid, it->cmdid, it->task_info, writelen, it->data.PosLength(), it->data.Length()) >> xlog_group;
                    it->data.Seek(writelen, AutoBuffer::ESeekCur);
                    writelen = 0;
                }
            }
        }
        
        lock.unlock();
        
        if (sel.Read_FD_ISSET(_sock)) {
            bufrecv.AllocWrite(64 * 1024, false);
            ssize_t recvlen = recv(_sock, bufrecv.PosPtr(), 64 * 1024, 0);
            
            if (0 == recvlen) {
                _errtype = kEctSocket;
                _errcode = kEctSocketShutdown;
                xwarn2(TSF"task socket close sock:%0, remote disconnect", _sock) >> close_log;
                goto End;
            }
            
            if (0 > recvlen && !IS_NOBLOCK_READ_ERRNO(socket_errno)) {
                _errtype = kEctSocket;
                _errcode = socket_errno;
                xerror2(TSF"task socket close sock:%0, recv len: %1 errno:%2(%3)", _sock, recvlen, socket_errno, socket_strerror(socket_errno)) >> close_log;
                goto End;
            }
            
            if (0 > recvlen) recvlen = 0;
            
            GetSignalOnNetworkDataChange()(XLOGGER_TAG, 0, recvlen);
            
            bufrecv.Length(bufrecv.Pos() + recvlen, bufrecv.Length() + recvlen);
            xinfo2(TSF"task socket recv sock:%_, recv len:%_, buff len:%_", _sock, recvlen, bufrecv.Length());
            
            while (0 < bufrecv.Length()) {
                uint32_t cmdid = 0;
                uint32_t taskid = Task::kInvalidTaskID;
                size_t packlen = 0;
                AutoBuffer body;
                
                int unpackret = longlink_unpack(bufrecv, cmdid, taskid, packlen, body);
                
                if (LONGLINK_UNPACK_FALSE == unpackret) {
                    xerror2(TSF"task socket recv sock:%0, unpack error dump:%1", _sock, xdump(bufrecv.Ptr(), bufrecv.Length()));
                    _errtype = kEctNetMsgXP;
                    _errcode = kEctNetMsgXPHandleBufferErr;
                    goto End;
                }
                
                xinfo2(TSF"task socket recv sock:%_, pack recv %_ taskid:%_, cmdid:%_, %_, packlen:(%_/%_)", _sock, LONGLINK_UNPACK_CONTINUE == unpackret ? "continue" : "finish", taskid, cmdid, sent_taskids[taskid], LONGLINK_UNPACK_CONTINUE == unpackret ? bufrecv.Length() : packlen, packlen);
                lastrecvtime_.gettickcount();
                
                if (LONGLINK_UNPACK_CONTINUE == unpackret) {
                    OnRecv(taskid, bufrecv.Length(), packlen);
                    break;
                } else {
                    
                    sent_taskids.erase(taskid);
                    
                    bufrecv.Move(-(int)(packlen));
                    
                    if (__NoopResp(cmdid, taskid, body, alarmnooptimeout, _profile)) {
                        xdebug2(TSF"noopresp span:%0", alarmnooptimeout.ElapseTime());
                        is_noop = false;
                    } else {
                        OnResponse(kEctOK, 0, cmdid, taskid, body, _profile);
                    }
                }
            }
        }
    }
    
    
End:
    if (is_noop) __NotifySmartHeartbeatHeartResult(false, false, _profile);
        
    std::string netInfo;
    getCurrNetLabel(netInfo );
    xinfo2(TSF", net_type:%_", netInfo) >> close_log;
    
    int nwrite_size = socket_nwrite(_sock);
    int nread_size = socket_nread(_sock);
    if (nwrite_size > 0 && !nsent_datas.empty()) {
        xinfo2(TSF", info nwrite:%_ ", nwrite_size) >> close_log;
        ssize_t maxnwrite = 0;
        for (std::vector<LongLinkNWriteData>::reverse_iterator it = nsent_datas.rbegin(); it != nsent_datas.rend(); ++it) {
            if (nwrite_size <= (maxnwrite + it->writelen)) {
                xinfo2(TSF"taskid:%_, cmdid:%_, taskid:%_ ; ", it->taskid, it->cmdid, it->taskid) >> close_log;
                break;
            } else {
                maxnwrite += it->writelen;
                xinfo2(TSF"taskid:%_, cmdid:%_, task_info:%_ ; ", it->taskid, it->cmdid, it->task_info) >> close_log;
            }
        }
    }
    nsent_datas.clear();
    
    if (nread_size > 0 && _errtype != kEctNetMsgXP && _errcode != kEctNetMsgXPHandleBufferErr) {
        xinfo2(TSF", info nread:%_ ", nread_size) >> close_log;
        AutoBuffer bufrecv;
        bufrecv.AllocWrite(64 * 1024, false);
        ssize_t recvlen = recv(_sock, bufrecv.PosPtr(), 64 * 1024, 0);
        
        xinfo2_if(recvlen <= 0, TSF", recvlen:%_ error:%_ %_", recvlen, socket_errno, socket_strerror(socket_errno)) >> close_log;
        if (recvlen > 0) {
			bufrecv.Length(bufrecv.Pos() + recvlen, bufrecv.Length() + recvlen);

			while (0 < bufrecv.Length()) {
				uint32_t cmdid = 0;
				uint32_t taskid = Task::kInvalidTaskID;
				size_t packlen = 0;
				AutoBuffer body;

				int unpackret = longlink_unpack(bufrecv, cmdid, taskid, packlen, body);
				xinfo2(TSF"taskid:%_, cmdid:%_, task_info:%_; ", taskid, cmdid, sent_taskids[taskid]) >> close_log;
				if (LONGLINK_UNPACK_CONTINUE == unpackret || LONGLINK_UNPACK_FALSE == unpackret) {
					break;
				} else {
					sent_taskids.erase(taskid);
					bufrecv.Move(-(int)(packlen));
				}
			}
        }
    }
    
#if defined(__ANDROID__) || defined(__APPLE__)
    struct tcp_info _info;
    if (getsocktcpinfo(_sock, &_info) == 0) {
    	char tcp_info_str[1024] = {0};
        xinfo2(TSF"task socket close getsocktcpinfo:%_", tcpinfo2str(&_info, tcp_info_str, sizeof(tcp_info_str))) >> close_log;
    }
#endif
}

void LongLink::__NotifySmartHeartbeatHeartReq(ConnectProfile& _profile, uint64_t _internal, uint64_t _actual_internal) {
    if (longlink_noop_interval() > 0) {
        return;
    }
    
    if (!smartheartbeat_) return;
    
	NoopProfile noop_profile;
	noop_profile.noop_internal = _internal;
	noop_profile.noop_actual_internal = _actual_internal;
	noop_profile.noop_starttime = ::gettickcount();
	_profile.noop_profiles.push_back(noop_profile);

    smartheartbeat_->OnHeartbeatStart();
}

void LongLink::__NotifySmartHeartbeatHeartResult(bool _succes, bool _fail_of_timeout, ConnectProfile& _profile) {
    if (longlink_noop_interval() > 0) {
        return;
    }
    
    if (!smartheartbeat_) return;
    
	if (!_profile.noop_profiles.empty()) {
		NoopProfile& noop_profile = _profile.noop_profiles.back();
		noop_profile.noop_cost = ::gettickcount() - noop_profile.noop_starttime;
        noop_profile.success = _succes;
	}

	if (smartheartbeat_) smartheartbeat_->OnHeartResult(_succes, _fail_of_timeout);
}

void LongLink::__NotifySmartHeartbeatJudgeMIUIStyle() {
    if (longlink_noop_interval() > 0) {
        return;
    }
    
    if (!smartheartbeat_) return;
	smartheartbeat_->JudgeMIUIStyle();
}

void LongLink::__NotifySmartHeartbeatConnectStatus(TLongLinkStatus _status) {
    if (longlink_noop_interval() > 0) {
        return;
    }
    
    if (!smartheartbeat_) return;

    switch (_status) {
    case kConnected:
    	smartheartbeat_->OnLongLinkEstablished();
        break;

    case kConnectFailed:  // no break;
    case kDisConnected:
    	smartheartbeat_->OnLongLinkDisconnect();
        break;

    default:
        break;
    }
}

unsigned int LongLink::__GetNextHeartbeatInterval() {
    
    if (longlink_noop_interval() > 0) {
        return longlink_noop_interval();
    }
    
    if (!smartheartbeat_) return MinHeartInterval;
    
    bool use_smartheart_beat  = false;
    return smartheartbeat_->GetNextHeartbeatInterval(use_smartheart_beat);
}
