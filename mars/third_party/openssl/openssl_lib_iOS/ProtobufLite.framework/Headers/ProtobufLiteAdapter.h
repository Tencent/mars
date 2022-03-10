//
//  ProtobufLiteAdapter.h
//  protobuf_lite
//
//  Created by yanyang on 2020/1/14.
//  Copyright © 2020 yanyang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LogLevelDef.h"

@protocol ProtobufLiteAdapterDelegate <NSObject>

@optional

- (BOOL)ShouldLog:(int)level;
- (void)LogWithinCommon:(WTLogLevel)log_level module:(const char *)module file:(const char *)file line:(int)line funcName:(const char *)funcName message:(NSString *)message;

@end


@interface ProtobufLiteAdapter : NSObject

+ (void)SetupWithDelegate:(id<ProtobufLiteAdapterDelegate>)delegate;

@end

#ifdef __cplusplus
extern "C" {
#endif
	BOOL ShouldLog(int level);
    void LogWithinCommon(WTLogLevel log_level, const char *module, const char *file, int line, const char *funcname, NSString *message);
#ifdef __cplusplus
}
#endif
