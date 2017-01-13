// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  stn_logic.h
//  network
//
//  Created by yanguoyue on 16/2/18.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef MARS_STN_LOGIC_H_
#define MARS_STN_LOGIC_H_

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"

namespace mars{
namespace stn{
    //callback interface
    class Callback
    {
    public:
    	virtual ~Callback() {}
        virtual bool MakesureAuthed() = 0;
        
        //流量统计
        virtual void TrafficData(ssize_t _send, ssize_t _recv);
        
        //底层询问上层该host对应的ip列表
        virtual std::vector<std::string> OnNewDns(const std::string& host);
        //网络层收到push消息回调
        virtual void OnPush(int32_t cmdid, const AutoBuffer& msgpayload) = 0;
        //底层获取task要发送的数据
        virtual bool Req2Buf(int32_t taskid, void* const user_context, AutoBuffer& outbuffer, int& error_code, const int channel_select) = 0;
        //底层回包返回给上层解析
        virtual int Buf2Resp(int32_t taskid, void* const user_context, const AutoBuffer& inbuffer, int& error_code, const int channel_select) = 0;
        //任务执行结束
        virtual int  OnTaskEnd(int32_t taskid, void* const user_context, int error_type, int error_code) = 0;

        //上报网络连接状态
        virtual void ReportConnectStatus(int status, int longlink_status) = 0;
        //长连信令校验 ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2
        virtual int  GetLonglinkIdentifyCheckBuffer(AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) = 0;
        //长连信令校验回包
        virtual bool OnLonglinkIdentifyResponse(const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) = 0;
        
        
        virtual void RequestSync() = 0;
        
        //验证是否已登录
        virtual bool IsLogoned() = 0;
    };

    void SetCallback(Callback* const callback);
    

    void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports);
    void SetShortlinkSvrAddr(const uint16_t port);
    

    // 'host' will be ignored when 'debugip' is not empty.
    void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip);
    
    // 'task.host' will be ignored when 'debugip' is not empty.
    void SetShortlinkSvrAddr(const uint16_t port, const std::string& debugip);
    
    // setting debug ip address for the corresponding host
    void SetDebugIP(const std::string& host, const std::string& ip);
    
    // setting backup iplist for the corresponding host
    // if debugip is not empty, iplist will be ignored.
    // iplist will be used when newdns/dns ip is not available.
    void SetBackupIPs(const std::string& host, const std::vector<std::string>& iplist);
    

    // async function.
    void StartTask(const Task& task);
    
    // sync function
    void StopTask(int32_t taskid);
    
    // check whether task's list has the task or not.
    bool HasTask(int32_t taskid);

    // reconnect longlink and redo all task
    // when you change svr ip, you must call this function.
    void RedoTasks();
    
    // stop and clear all task
    void ClearTasks();
    
    // the same as ClearTasks(), but also reinitialize network.
    void Reset();
    
    //setting signalling's parameters.
    //if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
    void SetSignallingStrategy(long period, long keeptime);

    // used to keep longlink active
    // keep signnaling once 'period' and last 'keeptime'
    void KeepSignalling();
    

    void StopSignalling();
    
    // connect quickly if longlink is not connected.
    void MakesureLonglinkConnected();
    
    bool LongLinkIsConnected();

    // noop is used to keep longlink conected
    // get noop taskid
    uint32_t getNoopTaskID();
}}

#endif /* MARS_STN_LOGIC_H_ */
