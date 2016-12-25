//
//  NetworkEvent.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/24.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "NetworkDelegate.h"

static const int SAYHELLO = 1;
static const int CONVERSATION_LIST = 2;
static const int SENDMSG = 3;

@interface NetworkEvent : NSObject<NetworkDelegate> {
    NSMutableDictionary   *tasks;
    NSMutableDictionary* controllers;
}

@end
