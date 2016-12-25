//
//  NetworkStatus.m
//  iOSDemo
//
//  Created by yanguoyue on 16/12/19.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#import "NetworkStatus.h"

#import <Foundation/Foundation.h>
#import <SystemConfiguration/CaptiveNetwork.h>
#import <SystemConfiguration/SCNetworkReachability.h>
#import <netinet/in.h>


SCNetworkReachabilityRef g_Reach;


static void ReachCallback(SCNetworkReachabilityRef target, SCNetworkConnectionFlags flags, void* info)
{
    @autoreleasepool {
        [(__bridge id)info performSelector:@selector(ChangeReach)];
    }
}

@implementation NetworkStatus

static NetworkStatus * sharedSingleton = nil;

+ (NetworkStatus*)sharedInstance {
    @synchronized (self) {
        if (sharedSingleton == nil) {
            sharedSingleton = [[NetworkStatus alloc] init];
        }
    }
    
    return sharedSingleton;
}

-(void) Start:(__unsafe_unretained id<NetworkStatusDelegate>)delNetworkStatus {
    
    m_delNetworkStatus = delNetworkStatus;
    
    if (g_Reach == nil) {
        struct sockaddr_in zeroAddress;
        bzero(&zeroAddress, sizeof(zeroAddress));
        zeroAddress.sin_len = sizeof(zeroAddress);
        zeroAddress.sin_family = AF_INET;
        g_Reach = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (struct sockaddr *)&zeroAddress);
    }

  
    SCNetworkReachabilityContext context = {0, (__bridge void *)self, NULL, NULL, NULL};
    if(SCNetworkReachabilitySetCallback(g_Reach, ReachCallback, &context)) {
        if(!SCNetworkReachabilityScheduleWithRunLoop(g_Reach, CFRunLoopGetCurrent(), kCFRunLoopCommonModes)) {

            SCNetworkReachabilitySetCallback(g_Reach, NULL, NULL);
            return;
        }
    }
    

    
}

-(void) Stop {
    if(g_Reach != nil) {
        SCNetworkReachabilitySetCallback(g_Reach, NULL, NULL);
        SCNetworkReachabilityUnscheduleFromRunLoop(g_Reach, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFRelease(g_Reach);
        g_Reach = nil;
    }

    m_delNetworkStatus = nil;
}

-(void) ChangeReach {
    
    SCNetworkConnectionFlags connFlags;

    if(!SCNetworkReachabilityGetFlags(g_Reach, &connFlags)) {
        return;
    }
   
    if(m_delNetworkStatus != nil) {
        [m_delNetworkStatus ReachabilityChange:connFlags];
    }


}

@end
