//
//  main.m
//  mactest
//
//  Created by perryzhou on 16/4/21.
//  Copyright © 2016年 perryzhou. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <mars/xlog/xloggerbase.h>
#import <mars/xlog/xlogger.h>
#import <mars/xlog/appender.h>
#import <Foundation/Foundation.h>

#import "AppDelegate.h"

int main(int argc, const char * argv[]) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString *libraryDirectory = [paths firstObject];
    
    // init xlog
#if DEBUG
    xlogger_SetLevel(kLevelDebug);
    appender_set_console_log(true);
#else
    xlogger_SetLevel(kLevelInfo);
    appender_set_console_log(false);
#endif
    appender_open(kAppednerAsync, [[libraryDirectory stringByAppendingString:@"/log/"] UTF8String], "test");

    return NSApplicationMain(argc, argv);
}
