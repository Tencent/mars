//
//  NetworkStatus.m
//  iOSDemo
//
//  Created by yanguoyue on 16/12/19.
//  Copyright © 2016年 Tencet. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol NetworkStatusDelegate

-(void) ReachabilityChange:(UInt32)uiFlags;

@end

@interface NetworkStatus : NSObject {
	__unsafe_unretained id<NetworkStatusDelegate> m_delNetworkStatus;
}

+ (NetworkStatus*)sharedInstance;

-(void) Start:(__unsafe_unretained id<NetworkStatusDelegate>)delNetworkStatus;
-(void) Stop;
-(void) ChangeReach;

@end
