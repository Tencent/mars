//
//  AppDelegate.m
//  mactest
//
//  Created by perryzhou on 16/4/21.
//  Copyright © 2016年 perryzhou. All rights reserved.
//

#import "AppDelegate.h"

#import <mars/xlog/appender.h>

#import "NetworkService.h"
#import "NetworkEvent.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    [NetworkService sharedInstance].delegate = [[NetworkEvent alloc] init];
    [[NetworkService sharedInstance] setCallBack];
    [[NetworkService sharedInstance] setClientVersion:200];
    [[NetworkService sharedInstance] createMars];
    [[NetworkService sharedInstance] setLongLinkAddress:@"localhost" port:8081];
    [[NetworkService sharedInstance] setShortLinkPort:8080];

    [[NetworkService sharedInstance] makesureLongLinkConnect];
    [[NetworkStatus sharedInstance] Start:[NetworkService sharedInstance]];

}

- (void)applicationWillTerminate:(NSNotification *)aNotification {

    [[NetworkService sharedInstance] destoryMars];
    appender_close();
}

@end
