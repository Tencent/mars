// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/** * created on : 2012-11-28 * author : yerungui, caoshaokun
 */
#include "stn_callback.h"

#import <mars/comm/autobuffer.h>
// #import <mars/xlog/xlogger.h>
#import <xlogger.h>
#import <mars/stn/stn.h>

#include "NetworkService.h"


#include <stn_callback.h>
#include <stn_logic.h>
#include <app_callback.h>
#include <stnproto_logic.h>
#include "CGITask.h"
#include <CommandID.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eval.h>
#include <pybind11/functional.h>

#include <iostream>
#include <LogUtil.h>

namespace py = pybind11;


namespace mars {
    namespace stn {


void* _push_cb_run(std::string str_py, py::object &func, 
                    uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, 
                    const void* _body, const void* _extend);


StnCallBack* StnCallBack::instance_ = NULL;


int set_push_callback(py::object &func) {
    if(func) {
        StnCallBack::Instance()->push_func = func;
    }else {
        printf("func is None.\n");
    }
    return 0;
}

// todo: more robust needed.
py::object get_push_callback() {
    return StnCallBack::Instance()->push_func;
}

StnCallBack* StnCallBack::Instance() {
    // printf("stn init.\n");
    if(instance_ == NULL) {
        instance_ = new StnCallBack();
    }
    
    return instance_;
}

StnCallBack* StnCallBack::Instance(py::object &func) {
    // printf("stn init.\n");
    if(instance_ == NULL) {
        instance_ = new StnCallBack();
        instance_->setPushCallback(func);
    }
    
    return instance_;
}

        
void StnCallBack::Release() {
    printf("stn released.\n");
    delete instance_;
    instance_ = NULL;
}

bool StnCallBack::MakesureAuthed() {
    return true;
}

void StnCallBack::TrafficData(ssize_t _send, ssize_t _recv) {
    xdebug2(TSF"send:%_, recv:%_", _send, _recv);
}

std::vector<std::string> StnCallBack::OnNewDns(const std::string& _host) {
    xdebug2("on new dns.\n");
    std::vector<std::string> vector;
    return vector;
}

// for test utilies.
void StnCallBack::setName(const std::string &name_) {
    instance_->name = name_;
}

const std::string &StnCallBack::getName() const {
    return instance_->name;
}

void StnCallBack::setPushCallback(py::object &func_) {
    printf("func address %p\n", &func_);
    instance_->push_func = func_;
    printf("instance func address:%p\n", &instance_->push_func);
}

py::object StnCallBack::getPushCallback()  { 
    // printf("return func address: %p\n", &instance_->push_func);
    return instance_->push_func;
}



void StnCallBack::OnPush(uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    // printf("push. length of body:%zu\n", _body.Length());
    if (_body.Length() > 0) {
        // NSData* recvData = [NSData dataWithBytes:(const void *) _body.Ptr() length:_body.Length()];
        // [[NetworkService sharedInstance] OnPushWithCmd:_cmdid data:recvData];
        // printf("callback thread id:%d\n", PyThread_get_thread_ident());
         py::object func = instance_->getPushCallback();
        _push_cb_run("print 'good.'", func, _channel_id, _cmdid, _taskid, (const void *) _body.Ptr(), (const void *) _extend.Ptr());
    }
}

bool StnCallBack::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select) {

    std::string *sp = static_cast<std::string*>(_user_context);
    std::string s = *sp;
    printf("user_content:%s\n", s.c_str());

    outbuffer.AllocWrite(s.length());
    outbuffer.Write(s.c_str(), s.length());

    BOOL ret = s.length() > 0;
    
    return ret;
}

int StnCallBack::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) {
    printf("buf 2 resp. length of data:%zu\n", _inbuffer.Length());
    int handle_type = mars::stn::kTaskFailHandleNormal;
    NSData* responseData = [NSData dataWithBytes:(const void *) _inbuffer.Ptr() length:_inbuffer.Length()];
    NSInteger errorCode = [[NetworkService sharedInstance] Buffer2ResponseWithTaskID:_taskid ResponseData:responseData userContext:_user_context];


    if (errorCode) {
        handle_type = mars::stn::kTaskFailHandleDefault;
    }
    
    return handle_type;
}

int  StnCallBack::OnTaskEnd(uint32_t _taskid, void* const _user_context, int _error_type, int _error_code) {
    return (int)[[NetworkService sharedInstance] OnTaskEndWithTaskID:_taskid userContext:_user_context errType:_error_type errCode:_error_code];
}


void StnCallBack::ReportConnectStatus(int _status, int _longlink_status) {
    // printf("report connect status\n");
    switch (_longlink_status) {
        case mars::stn::kServerFailed:
        case mars::stn::kServerDown:
        case mars::stn::kGateWayFailed:
            break;
        case mars::stn::kConnecting:
            break;
        case mars::stn::kConnected:
            break;
        case mars::stn::kNetworkUnkown:
            return;
        default:
            return;
    }
    
}

// synccheck：长链成功后由网络组件触发
// 需要组件组包，发送一个req过去，网络成功会有resp，但没有taskend，处理事务时要注意网络时序
// 不需组件组包，使用长链做一个sync，不用重试
int  StnCallBack::GetLonglinkIdentifyCheckBuffer(AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid) {
    // printf("GetLonglinkIdentifyCheckBuffer\n");
    return IdentifyMode::kCheckNever;
}

bool StnCallBack::OnLonglinkIdentifyResponse(const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash) {
    // printf("OnLonglinkIdentifyResponse\n");
    return false;
}
//
void StnCallBack::RequestSync() {
    printf("RequestSync\n");
}
        
bool StnCallBack::IsLogoned() {
    printf("IsLogoned true.\n");
    return true;
}
        
    }
}






