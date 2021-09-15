//
//  GeneratedMessage+WeChat.h
//  ProtobufLite
//
//  Created by yanyang on 2020/1/17.
//  Copyright © 2020 yanyang. All rights reserved.
//

#ifndef GeneratedMessage_WeChat_h
#define GeneratedMessage_WeChat_h

#import "GeneratedMessage+ObjC.h"

@class BaseRequest;
@class BaseResponse;

@interface WXPBGeneratedMessage (WeChat)

// some method for warning
- (void) setBaseRequest:(BaseRequest*) value;
- (BaseResponse *)baseResponse;
- (UInt32) continueFlag;

@end

#endif /* GeneratedMessage_WeChat_h */
