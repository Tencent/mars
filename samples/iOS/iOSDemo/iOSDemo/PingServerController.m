//
//  PingServerController.m
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
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
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)onButtonClick:(id)sender forEvent:(UIEvent *)event {
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
        LOG_INFO(kModuleViewController, @"recv hello response: %@", helloResponse.errmsg);
    }
    
    return helloResponse.retcode == 0 ? 0 : -1;
}

@end
