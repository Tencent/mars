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
#import "AppSetupManager.h"

#include <mars/xlog/xlogger.h>
#include <mars/xlog/appender.h>
#include <thread>

@interface AppDelegate ()

@end

@implementation AppDelegate

std::string RandStr(const int len) {
    std::string str;
    str.resize(len);
    int format_cnt = 0;
    for (int i = 0; i < len;) {
        str[i] = 'A' + rand() % 26;
        if (i%5 == 0 && format_cnt < 5) {
            ++i;
            str[i] = '%';
            ++i;
            str[i] = '_';
            format_cnt++;
        }
        ++i;
    }
    return str;
}

long GetCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void writeLog() {
    int CNT = 3000;
    for (int i = 0; i < CNT; ++i) {
        std::string s = RandStr(50);
//        xinfo2(TSF s.c_str(), "123", "456", "789", "234", (int)s.size()); // 最原始的方式单线程打日志87 ms  最原始的方式多线程打日志1350ms,  最原始的方式使用 4 个 mmap 打日志 635ms
        __xbi(455, 7890, 102, 123456, "123", "456", "789", "234", (int)s.size()); // 使用二进制单线程打日志36ms 使用二进制多线程打日志632ms, 使用二进制且使用 4 个 mmap 打日志 342ms
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    
    xinfo_function();
    xinfo2(TSF"%_ %_", "123", 123);
    
    xinfo2(TSF"%0 %1 %0", "123", 345);
    xinfo2("%s %d", "232", 123);

    
    std::string s = "123fdfdfd";
    __xbi(455, 7890, 102, 123456, s);
    
    long before = GetCurrentTime();
    std::thread t[10];
    for (int i = 0; i < 10; ++i) {
        t[i] = std::thread(writeLog);
    }
    
    for (int i = 0; i < 10; ++i) {
        t[i].join();
    }
    
    writeLog();
    long after = GetCurrentTime();
    NSLog(@"use time:%ld", (after - before));
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    mars::xlog::appender_close();
}

@end
