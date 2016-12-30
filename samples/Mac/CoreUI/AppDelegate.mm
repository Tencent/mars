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
//  AppDelegate.m
//  mactest
//
//  Created by zhoudingpin on 16/4/21.
//  Copyright © 2016年 zhoudingpin. All rights reserved.
//

#import "AppDelegate.h"

#import <mars/xlog/appender.h>

#import "NetworkService.h"
#import "NetworkEvent.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    [NetworkService sharedInstance].delegate = [[NetworkEvent alloc] init];
    [[NetworkService sharedInstance] setCallBack];
    [[NetworkService sharedInstance] setClientVersion:200];
    [[NetworkService sharedInstance] createMars];
    [[NetworkService sharedInstance] setLongLinkAddress:@"localhost" port:8081];
    [[NetworkService sharedInstance] setShortLinkPort:8080];

    [[NetworkService sharedInstance] reportEvent_OnForground:YES];
    [[NetworkService sharedInstance] makesureLongLinkConnect];
    [[NetworkStatus sharedInstance] Start:[NetworkService sharedInstance]];

}

- (void)applicationWillTerminate:(NSNotification *)aNotification {

    [[NetworkService sharedInstance] destoryMars];
    appender_close();
}

@end
