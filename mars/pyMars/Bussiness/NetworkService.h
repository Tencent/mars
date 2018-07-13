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
//  NetworkService.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef NetworkService_h
#define NetworkService_h

#import <Foundation/Foundation.h>

#import "NetworkDelegate.h"
#import "NetworkStatus.h"

@class CGITask;
@class ViewController;

@interface NetworkService : NSObject<NetworkStatusDelegate>

@property(nonatomic, strong) id<NetworkDelegate> delegate;

+ (NetworkService*)sharedInstance;

- (void)setCallBack;
- (void)createMars;

- (void)setClientVersion:(UInt32)clientVersion;
- (void)setShortLinkDebugIP:(NSString *)IP port:(const unsigned short)port;
- (void)setShortLinkPort:(const unsigned short)port;
- (void)setLongLinkAddress:(NSString *)string port:(const unsigned short)port debugIP:(NSString *)IP;
- (void)setLongLinkAddress:(NSString *)string port:(const unsigned short)port;
- (void)makesureLongLinkConnect;
- (void)destoryMars;

- (void)addPushObserver:(id<PushNotifyDelegate>)observer withCmdId:(int)cmdId;
- (int)startTask:(CGITask *)task ForUI:(id<UINotifyDelegate>)delegateUI;
- (void)stopTask:(NSInteger)taskID;


// event reporting
- (void)reportEvent_OnForground:(BOOL)isForground;
- (void)reportEvent_OnNetworkChange;

// callbacks

- (BOOL)isAuthed;
- (NSArray *)OnNewDns:(NSString *)address;
- (void)OnPushWithCmd:(NSInteger)cid data:(NSData *)data;
- (NSData*)Request2BufferWithTaskID:(uint32_t)tid userContext:(const void *)context;
- (NSInteger)Buffer2ResponseWithTaskID:(uint32_t)tid ResponseData:(NSData *)data userContext:(const void *)context;
- (NSInteger)OnTaskEndWithTaskID:(uint32_t)tid userContext:(const void *)context errType:(uint32_t)errtype errCode:(uint32_t)errcode;
- (void)OnConnectionStatusChange:(int32_t)status longConnStatus:(int32_t)longConnStatus;

@end

#endif /* NetworkService_hpp */
