#include <sys/xattr.h>
#import <mars/xlog/xloggerbase.h>
#import <mars/xlog/xlogger.h>
#import <mars/xlog/appender.h>
#import <mars/xlog/xlogger_interface.h>
#import <xlog-swift-bridge.h>

@implementation XLogBridge
- (void)open:(NSUInteger)level cacheDir:(NSString*)cacheDir logDir:(NSString*)logDir prefix:(NSString*)prefix cacheDays:(NSUInteger)cacheDays pubKey:(NSString*)pubKey consoleLogOpen:(BOOL)consoleLogOpen {
    xlogger_SetLevel((TLogLevel)level);
    mars::xlog::appender_set_console_log(consoleLogOpen);
    mars::xlog::XLogConfig config;
    config.pub_key_ = pubKey;
    config.mode_ = mars::xlog::kAppenderAsync;
    config.logdir_ = [logDir UTF8String];
    config.nameprefix_ = [prefix UTF8String];
    config.compress_mode_ = mars::xlog::kZlib;
    config.compress_level_ = 0;
    config.cachedir_ = [cacheDir UTF8String];
    config.cache_days_ = cacheDays;
    mars::xlog::appender_open(config);
    //keep Dart_XloggerWrite
    Dart_XloggerWrite(0, 2, "FlutterXLog", 0, 0, 0, "Open Flutter XLog.");
}

- (void)close {
    mars::xlog::appender_close();
}

@end
