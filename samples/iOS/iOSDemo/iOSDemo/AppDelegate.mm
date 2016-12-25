//
//  AppDelegate.m
//  test
//
//  Created by caoshaokun on 16/5/10.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "AppDelegate.h"

#import <mars/xlog/appender.h>

#import "NetworkService.h"
#import "NetworkEvent.h"
#import "NetworkStatus.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

@synthesize window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    [NetworkService sharedInstance].delegate = [[NetworkEvent alloc] init];
    [[NetworkService sharedInstance] setCallBack];
    [[NetworkService sharedInstance] createMars];
    [[NetworkService sharedInstance] setClientVersion:200];
    [[NetworkService sharedInstance] setLongLinkAddress:@"www.marsopen.cn" port:8081];
    [[NetworkService sharedInstance] setShortLinkPort:8080];
    [[NetworkService sharedInstance] makesureLongLinkConnect];
    
    [[NetworkStatus sharedInstance] Start:[NetworkService sharedInstance]];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {

}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    [[NetworkService sharedInstance] reportEvent_OnForground:NO];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    [[NetworkService sharedInstance] reportEvent_OnForground:YES];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {

}

- (void)applicationWillTerminate:(UIApplication *)application {
    
    [[NetworkService sharedInstance] destoryMars];
    
    appender_close();
}

@end
