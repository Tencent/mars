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
//  NetworkService.mm
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#include "NetworkService.h"

#import <SystemConfiguration/SCNetworkReachability.h>

#import "NetworkDelegate.h"
#import "CGITask.h"

#import "app_callback.h"
#import "stn_callback.h"

#import <mars/app/app_logic.h>
#import <mars/baseevent/base_logic.h>
#import <mars/xlog/xlogger.h>
#import <mars/xlog/xloggerbase.h>
#import <mars/xlog/appender.h>

#import "stnproto_logic.h"

using namespace mars::stn;

@interface NetworkService ()

@end

@implementation NetworkService

static NetworkService * sharedSingleton = nil;


+ (NetworkService*)sharedInstance {
    @synchronized (self) {
        if (sharedSingleton == nil) {
            sharedSingleton = [[NetworkService alloc] init];
        }
    }

    return sharedSingleton;
}

- (void)dealloc {
    
}

- (void)setCallBack {
    mars::stn::SetCallback(mars::stn::StnCallBack::Instance());
    mars::app::SetCallback(mars::app::AppCallBack::Instance());
}

- (void) createMars {
    mars::baseevent::OnCreate();
}

- (void)setClientVersion:(UInt32)clientVersion {
    mars::stn::SetClientVersion(clientVersion);
}

- (void)setShortLinkDebugIP:(NSString *)IP port:(const unsigned short)port {
    std::string ipAddress([IP UTF8String]);
    mars::stn::SetShortlinkSvrAddr(port, ipAddress);
}

- (void)setShortLinkPort:(const unsigned short)port {
    mars::stn::SetShortlinkSvrAddr(port, "");
}

- (void)setLongLinkAddress:(NSString *)string port:(const unsigned short)port debugIP:(NSString *)IP {
    std::string ipAddress([string UTF8String]);
    std::string debugIP([IP UTF8String]);
    std::vector<uint16_t> ports;
    ports.push_back(port);
    mars::stn::SetLonglinkSvrAddr(ipAddress,ports,debugIP);
}

- (void)setLongLinkAddress:(NSString *)string port:(const unsigned short)port {
    std::string ipAddress([string UTF8String]);
    std::vector<uint16_t> ports;
    ports.push_back(port);
    mars::stn::SetLonglinkSvrAddr(ipAddress,ports, "");
}

- (void)makesureLongLinkConnect {
    mars::stn::MakesureLonglinkConnected();
}

- (void)destoryMars {
    mars::baseevent::OnDestroy();
}

- (int)startTask:(CGITask *)task ForUI:(id<UINotifyDelegate>)delegateUI {
    Task ctask;
    ctask.cmdid = task.cmdid;
    ctask.channel_select = task.channel_select;
    ctask.cgi = std::string(task.cgi.UTF8String);
    ctask.shortlink_host_list.push_back(std::string(task.host.UTF8String));
    ctask.user_context = (__bridge void*)task;
    
    mars::stn::StartTask(ctask);
    
    NSString *taskIdKey = [NSString stringWithFormat:@"%d", ctask.taskid];
    [_delegate addObserver:delegateUI forKey:taskIdKey];
    [_delegate addCGITasks:task forKey:taskIdKey];
    
    return ctask.taskid;
}

- (void)stopTask:(NSInteger)taskID {
    mars::stn::StopTask((int32_t)taskID);
}



// event reporting
- (void)reportEvent_OnForground:(BOOL)isForground {
    mars::baseevent::OnForeground(isForground);
}

- (void)reportEvent_OnNetworkChange {
    mars::baseevent::OnNetworkChange();
}

// callbacks
- (BOOL)isAuthed {
    return [_delegate isAuthed];
}

- (NSArray *)OnNewDns:(NSString *)address {
    return [_delegate OnNewDns:address];
}

- (void)OnPushWithCmd:(NSInteger)cid data:(NSData *)data {
    return [_delegate OnPushWithCmd:cid data:data];
}

- (NSData*)Request2BufferWithTaskID:(uint32_t)tid userContext:(const void *)context {
    CGITask *task = (__bridge CGITask *)context;
    return [_delegate Request2BufferWithTaskID:tid task:task];
}

- (NSInteger)Buffer2ResponseWithTaskID:(uint32_t)tid ResponseData:(NSData *)data userContext:(const void *)context {
    CGITask *task = (__bridge CGITask *)context;
    return [_delegate Buffer2ResponseWithTaskID:tid responseData:data task:task];
}

- (NSInteger)OnTaskEndWithTaskID:(uint32_t)tid userContext:(const void *)context errType:(uint32_t)errtype errCode:(uint32_t)errcode; {
    CGITask *task = (__bridge CGITask *)context;
    return [_delegate OnTaskEndWithTaskID:tid task:task errType:errtype errCode:errcode];
}

- (void)OnConnectionStatusChange:(int32_t)status longConnStatus:(int32_t)longConnStatus {
    [_delegate OnConnectionStatusChange:status longConnStatus:longConnStatus];
}

- (void)addPushObserver:(id<PushNotifyDelegate>)observer withCmdId:(int)cmdId {
    [_delegate addPushObserver:observer withCmdId:cmdId];
}


#pragma mark NetworkStatusDelegate
-(void) ReachabilityChange:(UInt32)uiFlags {
    
    if ((uiFlags & kSCNetworkReachabilityFlagsConnectionRequired) == 0) {
        mars::baseevent::OnNetworkChange();
    }
    
}

@end

