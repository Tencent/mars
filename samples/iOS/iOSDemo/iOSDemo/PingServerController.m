// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

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
    CGITask *helloCGI = [[CGITask alloc] initAll:ChannelType_All AndCmdId:kSayHello AndCGIUri:@"/mars/hello" AndHost:@"www.marsopen.cn"];
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
