//
//  PingServerController.m
//  mactest
//
//  Created by caoshaokun on 16/11/28.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "PingServerController.h"

#import "LogUtil.h"

#import "Main.pb.h"

#import "CGITask.h"
#import "CommandID.h"
#import "NetworkService.h"

@interface PingServerController ()

@end

@implementation PingServerController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (IBAction)onButtonClick:(id)sender {
    CGITask *helloCGI = [[CGITask alloc] initAll:ChannelType_All AndCmdId:kSayHello AndCGIUri:@"/mars/hello" AndHost:@"localhost"];
    [[NetworkService sharedInstance] startTask:helloCGI ForUI:self];
}

- (NSData*)requestSendData {
    HelloRequest* helloRequest = [[[[HelloRequest builder] setUser:@"caoshaokun"] setText:@"Hello world!"] build];
    NSData* data = [helloRequest data];
    return data;
}

- (int)notifyUIWithResponse:(NSData*)responseData {
    HelloResponse* helloResponse = [HelloResponse parseFromData:responseData];
    if ([helloResponse hasErrmsg]) {
        LOG_INFO(kModuleViewController, @"hello response: %@", helloResponse.errmsg);
    }
    
    return helloResponse.retcode == 0 ? 0 : -1;
}

@end
