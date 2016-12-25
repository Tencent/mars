//
//  main.m
//  iOSDemo
//
//  Created by 曹少琨 on 16/5/10.
//  Copyright © 2016年 曹少琨. All rights reserved.
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
        appender_set_console_log(true);
#else
        xlogger_SetLevel(kLevelInfo);
        appender_set_console_log(false);
#endif
        appender_open(kAppednerAsync, [logPath UTF8String], "Test");
        
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
