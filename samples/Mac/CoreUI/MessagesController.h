// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  MessagesController.h
//  mactest
//
//  Created by chenzihao on 17/05/15.
//  Copyright © 2017年 chenzihao. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "Main.pb.h"

#import "NetworkDelegate.h"
#import "UINotifyDelegate.h"
#import "MessagesDelegate.h"

@interface MessagesController : NSViewController<MessagesDelegate> {
}

-(void)setHostController:(NSViewController*)controller;
-(void)setConversation:(Conversation*)conversation;
@end
