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
//  test
//
//  Created by caoshaokun on 16/5/10.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "AppDelegate.h"

#import <mars/xlog/appender.h>

#import "NetworkService.h"
#import "NetworkEvent.h"
#import "NetworkStatus.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

@synthesize window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    [NetworkService sharedInstance].delegate = [[NetworkEvent alloc] init];
    [[NetworkService sharedInstance] setCallBack];
    [[NetworkService sharedInstance] createMars];
    [[NetworkService sharedInstance] setClientVersion:200];
    [[NetworkService sharedInstance] setLongLinkAddress:@"www.marsopen.cn" port:8081];
    [[NetworkService sharedInstance] setShortLinkPort:8080];
    [[NetworkService sharedInstance] reportEvent_OnForeground:YES];
    [[NetworkService sharedInstance] makesureLongLinkConnect];
    
    [[NetworkStatus sharedInstance] Start:[NetworkService sharedInstance]];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {

}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    [[NetworkService sharedInstance] reportEvent_OnForeground:NO];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    [[NetworkService sharedInstance] reportEvent_OnForeground:YES];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {

}

- (void)applicationWillTerminate:(UIApplication *)application {
    
    [[NetworkService sharedInstance] destroyMars];
    
    appender_close();
}

@end
