//
//  CGITask.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum : int32_t {
    ChannelType_ShortConn = 1,
    ChannelType_LongConn = 2,
    ChannelType_All = 3
} ChannelType;

@interface CGITask : NSObject

- (id)init;

- (id)initAll:(ChannelType)ChannelType AndCmdId:(uint32_t)cmdId AndCGIUri:(NSString*)cgiUri AndHost:(NSString*)host;

@property(nonatomic) uint32_t taskid;
@property(nonatomic) ChannelType channel_select;
@property(nonatomic) uint32_t cmdid;
@property(nonatomic, copy) NSString *cgi;
@property(nonatomic, copy) NSString *host;

@end
