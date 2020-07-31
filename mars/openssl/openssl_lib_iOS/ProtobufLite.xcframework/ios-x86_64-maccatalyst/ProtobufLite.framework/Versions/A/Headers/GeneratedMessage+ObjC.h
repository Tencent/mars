//
//  GeneratedMessage+ObjC.h
//  MMCommon
//
//  Created by sanhuazhang on 2019/9/20.
//  Copyright © 2019 WXG. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface WXPBGeneratedMessage : NSObject

+ (instancetype) parseFromData:(NSData*) data;

- (BOOL) isInitialized;
- (int32_t) serializedSize;
- (NSData*) serializedData;

- (instancetype) mergeFromData:(NSData*) data;

- (BOOL) hasProperty:(int)index;
#if defined(DEBUG) || defined(DAILY_BUILD)
- (NSString*) debugDescription2;
#endif
@end

NS_ASSUME_NONNULL_END
