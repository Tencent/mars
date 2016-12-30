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
//  NetworkDelegate.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016 Tencent. All rights reserved.
//


#import "UINotifyDelegate.h"
#import "PushNotifyDelegate.h"

@class CGITask;

@protocol NetworkDelegate <NSObject>

@required - (void)addPushObserver:(id<PushNotifyDelegate>)observer withCmdId:(int)cmdId;
@required - (void)addObserver:(id<UINotifyDelegate>)observer forKey:(NSString *)key;
@required - (void)addCGITasks:(CGITask*)cgiTask forKey:(NSString *)key;

@required - (BOOL)isAuthed;
@optional - (NSArray *)OnNewDns:(NSString *)address;
@optional - (void)OnPushWithCmd:(NSInteger)cid data:(NSData *)data;

@required - (NSData*)Request2BufferWithTaskID:(uint32_t)tid task:(CGITask *)task;
@required - (NSInteger)Buffer2ResponseWithTaskID:(uint32_t)tid responseData:(NSData *)data task:(CGITask *)task;
//
@required - (NSInteger)OnTaskEndWithTaskID:(uint32_t)tid task:(CGITask *)task errType:(uint32_t)errtype errCode:(uint32_t)errcode;
// report connection status
@optional - (void)OnConnectionStatusChange:(int32_t)status longConnStatus:(int32_t)longConnStatus;

@end

