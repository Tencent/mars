//
//  NetworkService.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef NetworkService_h
#define NetworkService_h

#import <Foundation/Foundation.h>

#import "NetworkDelegate.h"
#import "NetworkStatus.h"

@class CGITask;
@class ViewController;

@interface NetworkService : NSObject<NetworkStatusDelegate>

@property(nonatomic, strong) id<NetworkDelegate> delegate;

+ (NetworkService*)sharedInstance;

- (void)setCallBack;
- (void)createMars;

- (void)setClientVersion:(UInt32)clientVersion;
- (void)setShortLinkDebugIP:(NSString *)IP port:(const unsigned short)port;
- (void)setShortLinkPort:(const unsigned short)port;
- (void)setLongLinkAddress:(NSString *)string port:(const unsigned short)port debugIP:(NSString *)IP;
- (void)setLongLinkAddress:(NSString *)string port:(const unsigned short)port;
- (void)makesureLongLinkConnect;
- (void)destoryMars;


- (int)startTask:(CGITask *)task ForUI:(id<UINotifyDelegate>)delegateUI;
- (void)stopTask:(NSInteger)taskID;


// event reporting
- (void)reportEvent_OnForground:(BOOL)isForground;
- (void)reportEvent_OnNetworkChange;

// callbacks

- (BOOL)isAuthed;
- (NSArray *)OnNewDns:(NSString *)address;
- (void)OnPushWithCmd:(NSInteger)cid data:(NSData *)data;
- (NSData*)Request2BufferWithTaskID:(uint32_t)tid userContext:(const void *)context;
- (NSInteger)Buffer2ResponseWithTaskID:(uint32_t)tid ResponseData:(NSData *)data userContext:(const void *)context;
- (NSInteger)OnTaskEndWithTaskID:(uint32_t)tid userContext:(const void *)context errType:(uint32_t)errtype errCode:(uint32_t)errcode;
- (void)OnConnectionStatusChange:(int32_t)status longConnStatus:(int32_t)longConnStatus;

@end

#endif /* NetworkService_hpp */
