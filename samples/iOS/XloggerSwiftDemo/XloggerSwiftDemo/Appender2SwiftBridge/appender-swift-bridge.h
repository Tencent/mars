
//  Created by Jinkey on 2017/1/2.
//  Copyright © 2017年 Jinkey. All rights reserved.

//  appender-swift-bridge.h


#include <stdio.h>
#import <Foundation/Foundation.h>
#import "LogUtil.h"


typedef NS_ENUM(NSUInteger, XloggerType) {
    
    debug,
    info,
    warning,
    error,
    
};

@interface JinkeyMarsBridge: NSObject

- (void)initXlogger: (XloggerType)debugLevel releaseLevel: (XloggerType)releaseLevel path: (NSString*)path prefix: (const char*)prefix;
- (void)deinitXlogger;

- (void)log: (XloggerType) level tag: (const char*)tag content: (NSString*)content;

@end


