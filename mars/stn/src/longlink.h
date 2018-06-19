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
 * longlink.h
 *
 *  Created on: 2014-2-27
 *      Author: yerungui
 */

#ifndef STN_SRC_LONGLINK_H_
#define STN_SRC_LONGLINK_H_

#include <string>
#include <list>

#include "boost/signals2.hpp"
#include "boost/function.hpp"

#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/alarm.h"
#include "mars/comm/tickcount.h"
#include "mars/comm/move_wrapper.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/socket/socketselect.h"

#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"

#include "mars/stn/src/net_source.h"
#include "mars/stn/src/longlink_identify_checker.h"

class AutoBuffer;
class XLogger;
class WakeUpLock;

class SmartHeartbeat;


namespace mars {
    namespace comm {
        class ProxyInfo;
    }
    namespace stn {

class NetSource;
class longlink_tracker;

struct LongLinkNWriteData {
    LongLinkNWriteData(ssize_t _writelen, const Task& _task)
    : writelen(_writelen), task(_task) {}
    
    ssize_t writelen;
    Task task;
};
        
struct StreamResp {
    StreamResp(const Task& _task = Task(Task::kInvalidTaskID))
    : task(_task), stream(KNullAtuoBuffer), extension(KNullAtuoBuffer) {}
    
    Task task;
    move_wrapper<AutoBuffer> stream;
    move_wrapper<AutoBuffer> extension;
};

class LongLink {
  public:
    enum TLongLinkStatus {
        kConnectIdle = 0,
        kConnecting = 1,
        kConnected,
        kDisConnected,
        kConnectFailed,
    };

    // Note: Never Delete Item!!!Just Add!!!
    enum TDisconnectInternalCode {
        kNone = 0,
        kReset = 10000,        // no use
        kRemoteClosed = 10001,
        kUnknownErr = 10002,
        kNoopTimeout = 10003,
        kDecodeError = 10004,
        kUnknownRead = 10005,
        kUnknownWrite = 10006,
        kDecodeErr = 10007,
        kTaskTimeout = 10008,
        kNetworkChange = 10009,
        kIDCChange = 10010,
        kNetworkLost = 10011,
        kSelectError = 10012,
        kPipeError = 10013,
        kHasNewDnsIP = 10014,
        kSelectException = 10015,
        kLinkCheckTimeout = 10016,
        kForceNewGetDns = 10017,
        kLinkCheckError = 10018,
        kTimeCheckSucc = 10019,
    };

  public:
    boost::signals2::signal<void (TLongLinkStatus _connectStatus)> SignalConnection;
    boost::signals2::signal<void (const ConnectProfile& _connprofile)> broadcast_linkstatus_signal_;
    
    boost::function< void (uint32_t _taskid)> OnSend;
    boost::function< void (uint32_t _taskid, size_t _cachedsize, size_t _package_size)> OnRecv;
    boost::function< void (ErrCmdType _error_type, int _error_code, uint32_t _cmdid, uint32_t _taskid, AutoBuffer& _body, AutoBuffer& _extension, const ConnectProfile& _info)> OnResponse;
    boost::function<void (int _line, ErrCmdType _errtype, int _errcode, const std::string& _ip, uint16_t _port)> fun_network_report_;

  public:
    LongLink(const mq::MessageQueue_t& _messagequeueid, NetSource& _netsource);
    virtual ~LongLink();

    bool    Send(const AutoBuffer& _body, const AutoBuffer& _extension, const Task& _task);
    bool    SendWhenNoData(const AutoBuffer& _body, const AutoBuffer& _extension, uint32_t _cmdid, uint32_t _taskid);
    bool    Stop(uint32_t _taskid);

    bool            MakeSureConnected(bool* _newone = NULL);
    void            Disconnect(TDisconnectInternalCode _scene);
    TLongLinkStatus ConnectStatus() const;

    ConnectProfile  Profile() const   { return conn_profile_; }
    tickcount_t&    GetLastRecvTime() { return lastrecvtime_; }
    
  private:
    LongLink(const LongLink&);
    LongLink& operator=(const LongLink&);

  protected:
    void    __ConnectStatus(TLongLinkStatus _status);
    void    __UpdateProfile(const ConnectProfile& _conn_profile);
    void    __RunResponseError(ErrCmdType _type, int _errcode, ConnectProfile& _profile, bool _networkreport = true);

    bool    __SendNoopWhenNoData();
    bool    __NoopReq(XLogger& _xlog, Alarm& _alarm, bool need_active_timeout);
    bool    __NoopResp(uint32_t _cmdid, uint32_t _taskid, AutoBuffer& _buf, AutoBuffer& _extension, Alarm& _alarm, bool& _nooping, ConnectProfile& _profile);

    virtual void     __OnAlarm();
    virtual void     __Run();
    virtual SOCKET   __RunConnect(ConnectProfile& _conn_profile);
    virtual void     __RunReadWrite(SOCKET _sock, ErrCmdType& _errtype, int& _errcode, ConnectProfile& _profile);
    
  protected:
    
    uint32_t   __GetNextHeartbeatInterval();
    void       __NotifySmartHeartbeatConnectStatus(TLongLinkStatus _status);
    void       __NotifySmartHeartbeatHeartReq(ConnectProfile& _profile, uint64_t _internal, uint64_t _actual_internal);
    void       __NotifySmartHeartbeatHeartResult(bool _succes, bool _fail_of_timeout, ConnectProfile& _profile);
    void       __NotifySmartHeartbeatJudgeMIUIStyle();

  protected:
    MessageQueue::ScopeRegister     asyncreg_;
    NetSource&                      netsource_;
    
    Mutex                           mutex_;
    Thread                          thread_;

    boost::scoped_ptr<longlink_tracker>         tracker_;
    NetSource::DnsUtil                          dns_util_;
    SocketBreaker                               connectbreak_;
	SocketBreaker             testproxybreak_;
    TLongLinkStatus                             connectstatus_;
    ConnectProfile                              conn_profile_;
    TDisconnectInternalCode                     disconnectinternalcode_;
    
    SocketBreaker                                        readwritebreak_;
    LongLinkIdentifyChecker                              identifychecker_;
    std::list<std::pair<Task, move_wrapper<AutoBuffer>>> lstsenddata_;
    tickcount_t                                          lastrecvtime_;
    
    SmartHeartbeat*                              smartheartbeat_;
    WakeUpLock*                                  wakelock_;
};
        
}}

#endif // STN_SRC_LONGLINK_H_
