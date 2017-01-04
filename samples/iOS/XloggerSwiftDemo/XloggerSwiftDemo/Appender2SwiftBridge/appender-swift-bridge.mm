//  xlog
//
//  Created by Jinkey on 2017/1/2.
//  Copyright © 2017年 Jinkey. All rights reserved.
//

//  appender-swift-bridge.mm

#import "appender-swift-bridge.h"
#import <mars/xlog/appender.h>
#import <mars/xlog/xlogger.h>
#import <sys/xattr.h>

@implementation JinkeyMarsBridge

// 封装了初始化 Xlogger 方法
// initialize Xlogger
-(void)initXlogger: (XloggerType)debugLevel releaseLevel: (XloggerType)releaseLevel path: (NSString*)path prefix: (const char*)prefix{
    
    NSString* logPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingString:path];
    
    // set do not backup for logpath
    const char* attrName = "io.jinkey";
    u_int8_t attrValue = 1;
    setxattr([logPath UTF8String], attrName, &attrValue, sizeof(attrValue), 0, 0);
    
    // init xlog
    #if DEBUG
    switch (debugLevel) {
        case debug:
            xlogger_SetLevel(kLevelDebug);
        case info:
            xlogger_SetLevel(kLevelInfo);
        case warning:
            xlogger_SetLevel(kLevelWarn);
        case error:
            xlogger_SetLevel(kLevelError);
        default:
            break;
    }
    appender_set_console_log(true);
    #else
    switch (releaseLevel) {
        case debug:
            xlogger_SetLevel(kLevelDebug);
        case info:
            xlogger_SetLevel(kLevelInfo);
        case warning:
            xlogger_SetLevel(kLevelWarn);
        case error:
            xlogger_SetLevel(kLevelError);
        default:
            break;
    }
    appender_set_console_log(false);
    #endif
    appender_open(kAppednerAsync, [logPath UTF8String], prefix);
    
}

// 封装了关闭 Xlogger 方法
// deinitialize Xlogger
-(void)deinitXlogger {
    appender_close();
}


// 利用微信提供的 LogUtil.h 封装了打印日志的方法
// print log using LogUtil.h provided by Wechat
-(void) log: (XloggerType) level tag: (const char*)tag content: (NSString*)content{
    
    NSString* levelDescription = @"";
    
    switch (level) {
        case debug:
            LOG_DEBUG(tag, content);
            levelDescription = @"Debug";
            break;
        case info:
            LOG_INFO(tag, content);
            levelDescription = @"Info";
            break;
        case warning:
            LOG_WARNING(tag, content);
            levelDescription = @"Warn";
            break;
        case error:
            LOG_ERROR(tag, content);
            levelDescription = @"Error";
            break;
        default:
            break;
    }
    
}

@end


