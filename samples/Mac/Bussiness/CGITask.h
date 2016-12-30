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
//  CGITask.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum : int32_t {
    ChannelType_ShortConn = 1,
    ChannelType_LongConn = 2,
    ChannelType_All = 3
} ChannelType;

@interface CGITask : NSObject

- (id)init;

- (id)initAll:(ChannelType)ChannelType AndCmdId:(uint32_t)cmdId AndCGIUri:(NSString*)cgiUri AndHost:(NSString*)host;

@property(nonatomic) uint32_t taskid;
@property(nonatomic) ChannelType channel_select;
@property(nonatomic) uint32_t cmdid;
@property(nonatomic, copy) NSString *cgi;
@property(nonatomic, copy) NSString *host;

@end
