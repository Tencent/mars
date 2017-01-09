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
//  UINotifyDelegate.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/24.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#ifndef UINotifyDelegate_h
#define UINotifyDelegate_h

@protocol UINotifyDelegate <NSObject>

@required -(NSData*)requestSendData;
@required -(int)onPostDecode:(NSData*)responseData;
@required -(int)onTaskEnd:(uint32_t)tid errType:(uint32_t)errtype errCode:(uint32_t)errcode;

@end

#endif /* UINotifyDelegate_h */
