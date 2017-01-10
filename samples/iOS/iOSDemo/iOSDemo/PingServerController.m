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

#import "CGITask.h"
#import "CommandID.h"
#import "NetworkService.h"
#import "AppDelegate.h"
#import "LoginViewController.h"

@interface PingServerController ()

@end

@implementation PingServerController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        UIStoryboard *sb= [UIStoryboard storyboardWithName:@"Main" bundle:nil];
        LoginViewController *lvc = [sb instantiateViewControllerWithIdentifier:@"LoginViewController"];
        [self.tabBarController presentViewController:lvc animated:YES completion:nil];
    });
}

- (NSString *)username {
    return [(AppDelegate *)[UIApplication sharedApplication].delegate username];
}

- (IBAction)onButtonClick:(id)sender forEvent:(UIEvent *)event {
    CGITask *helloCGI = [[CGITask alloc] initAll:ChannelType_All AndCmdId:kSayHello AndCGIUri:@"/mars/hello" AndHost:@"www.marsopen.cn"];
    [[NetworkService sharedInstance] startTask:helloCGI ForUI:self];
}

- (NSData*)requestSendData {
    HelloRequest *helloRequest = [HelloRequest new];
    helloRequest.user = [self username];
    helloRequest.text = @"Hello world";
    NSData *data = [helloRequest data];
    return data;
}

- (int)onPostDecode:(NSData*)responseData {
    helloResponse = [HelloResponse parseFromData:responseData error:nil];
    if ([helloResponse hasErrmsg]) {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:helloResponse.errmsg preferredStyle:UIAlertControllerStyleAlert];
        [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
        [self presentViewController:alert animated:YES completion:nil];
        LOG_INFO(kModuleViewController, @"recv hello response: %@", helloResponse.errmsg);
    }
    
    return helloResponse.retcode == 0 ? 0 : -1;
}

- (int)onTaskEnd:(uint32_t)tid errType:(uint32_t)errtype errCode:(uint32_t)errcode {
    
    return 0;
}

@end
