//
//  NetworkEvent.m
//  iOSDemo
//
//  Created by caoshaokun on 16/11/24.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "NetworkEvent.h"

#import "CGITask.h"

@implementation NetworkEvent

- (void)addObserver:(id<UINotifyDelegate>)observer forKey:(NSString *)key {
    [controllers setObject:observer forKey:key];
}

- (void)addCGITasks:(CGITask*)cgiTask forKey:(NSString *)key {
    [tasks setObject:cgiTask forKey:key];
}

- (id)init {
    
    if(self = [super init]) {
        tasks = [[NSMutableDictionary alloc] init];
        controllers = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (BOOL)isAuthed {
    
    return true;
}

- (NSArray *)OnNewDns:(NSString *)address {
    
    return NULL;
}

- (void)OnPushWithCmd:(NSInteger)cid data:(NSData *)data {
    
}

- (NSData*)Request2BufferWithTaskID:(uint32_t)tid task:(CGITask *)task {
    NSData* data = NULL;
    
    NSString *taskIdKey = [NSString stringWithFormat:@"%d", tid];
    
    id<UINotifyDelegate> uiObserver = [controllers objectForKey:taskIdKey];
    if (uiObserver != nil) {
        data = [uiObserver requestSendData];
    }
    
    return data;
}

- (NSInteger)Buffer2ResponseWithTaskID:(uint32_t)tid responseData:(NSData *)data task:(CGITask *)task {
    int returnType = 0;
    
    NSString *taskIdKey = [NSString stringWithFormat:@"%d", tid];
    
    id<UINotifyDelegate> uiObserver = [controllers objectForKey:taskIdKey];
    if (uiObserver != nil) {
        returnType = [uiObserver notifyUIWithResponse:data];
    }
    else {
        returnType = -1;
    }
    
    return returnType;
}

- (NSInteger)OnTaskEndWithTaskID:(uint32_t)tid task:(CGITask *)task errType:(uint32_t)errtype errCode:(uint32_t)errcode {
    
    NSString *taskIdKey = [NSString stringWithFormat:@"%d", tid];
    
    [tasks removeObjectForKey:taskIdKey];
    [controllers removeObjectForKey:taskIdKey];
    
    return 0;
}

- (void)OnConnectionStatusChange:(int32_t)status longConnStatus:(int32_t)longConnStatus {
    
}


@end
