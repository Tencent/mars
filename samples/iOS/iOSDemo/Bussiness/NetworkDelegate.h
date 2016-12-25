//
//  NetworkDelegate.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/23.
//  Copyright © 2016 Tencent. All rights reserved.
//


#import "UINotifyDelegate.h"

@class CGITask;

@protocol NetworkDelegate <NSObject>

@required - (void)addObserver:(id<UINotifyDelegate>)observer forKey:(NSString *)key;
@required - (void)addCGITasks:(CGITask*)cgiTask forKey:(NSString *)key;

@required - (BOOL)isAuthed;
@optional - (NSArray *)OnNewDns:(NSString *)address;
@optional - (void)OnPushWithCmd:(NSInteger)cid data:(NSData *)data;

@required - (NSData*)Request2BufferWithTaskID:(uint32_t)tid task:(CGITask *)task;
@required - (NSInteger)Buffer2ResponseWithTaskID:(uint32_t)tid responseData:(NSData *)data task:(CGITask *)task;
//
@required - (NSInteger)OnTaskEndWithTaskID:(uint32_t)tid task:(CGITask *)task errType:(uint32_t)errtype errCode:(uint32_t)errcode;
// report connection status
@optional - (void)OnConnectionStatusChange:(int32_t)status longConnStatus:(int32_t)longConnStatus;

@end

