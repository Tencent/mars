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
//  main.m
//  iOSDemo
//
//  Created by caoshaokun on 16/5/10.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <mars/xlog/xloggerbase.h>
#import <mars/xlog/xlogger.h>
#import <mars/xlog/appender.h>
#import <Foundation/Foundation.h>
#import <sys/xattr.h>

#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {
        NSString* logPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingString:@"/log"];
        
        // set do not backup for logpath
        const char* attrName = "com.apple.MobileBackup";
        u_int8_t attrValue = 1;
        setxattr([logPath UTF8String], attrName, &attrValue, sizeof(attrValue), 0, 0);
        
        // init xlog
#if DEBUG
        xlogger_SetLevel(kLevelDebug);
        appender_set_console_log(false);
#else
        xlogger_SetLevel(kLevelInfo);
        appender_set_console_log(false);
#endif
//        save private key
//        6333d4aa8be799bd4697f42e26ed0c63be8196f4bced576d6f5c76a6b86b9aad
//
//        appender_open's parameter:
//        e8163215499a4c6d91b0691177127691e884ed409c49f2496102b626734f93f412de01d5df53772a7c7bd0c57f08062078b37d3f8b8ab995b9b3ec623b5bd2ff
        appender_open(kAppednerAsync, [logPath UTF8String], "Test", "e8163215499a4c6d91b0691177127691e884ed409c49f2496102b626734f93f412de01d5df53772a7c7bd0c57f08062078b37d3f8b8ab995b9b3ec623b5bd2ff");
        
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
