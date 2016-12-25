//
//  CGITask.m
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016 Tencent. All rights reserved.
//

#import "CGITask.h"

@implementation CGITask

- (id)init {

    if (self = [super init]) {
        self.channel_select = ChannelType_All;
        
    }
    
    return self;
}

- (id)initAll:(ChannelType)ChannelType AndCmdId:(uint32_t)cmdId AndCGIUri:(NSString*)cgiUri AndHost:(NSString*)host {
    
    if (self = [super init]) {
        self.channel_select = ChannelType;
        self.cmdid = cmdId;
        self.cgi = cgiUri;
        self.host = host;
    }
    
    return self;
}

- (void)dealloc {
}

@end
