// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  TopicViewController.m
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "TopicViewController.h"

#import "Chat.pb.h"
#import "Messagepush.pb.h"

#import "CGITask.h"
#import "CommandID.h"
#import "NetworkService.h"

#import "Constants.h"
#import "LogUtil.h"

@interface TopicViewController ()

@end

@implementation TopicViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    [[NetworkService sharedInstance] addPushObserver:self withCmdId:kPushMessageCmdId];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(NSData*)requestSendData {
    SendMessageRequest* sendMsgRequest = [[[[[[[SendMessageRequest builder] setFrom:@"anonymous"] setTo:@"all"] setText:self->text] setAccessToken:@"123456"] setTopic:_topicName]build];
    LOG_INFO(kModuleViewController, @"send msg to topic:%@", _topicName);
    NSData* data = [sendMsgRequest data];
    return data;
}


-(int)notifyUIWithResponse:(NSData*)responseData {
    SendMessageResponse *sendMsgResponse = [SendMessageResponse parseFromData:responseData];
    
    [_recvContentField setText:sendMsgResponse.errMsg];
    
    return sendMsgResponse.errCode == 0 ? 0 : -1;
}

- (IBAction)sendMessage:(id)sender forEvent:(UIEvent *)event {
    CGITask *sendMsgCGI = [[CGITask alloc] initAll:ChannelType_LongConn AndCmdId:kSendMsg AndCGIUri:@"/mars/sendmessage" AndHost:@"www.marsopen.cn"];
    [[NetworkService sharedInstance] startTask:sendMsgCGI ForUI:self];
    
    self->text = _textField.text;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [_contentField setText:self->text];
    });
    [_textField setText:@""];
}

- (void)notifyPushMessage:(NSData*)pushData withCmdId:(int)cmdId {
    MessagePush* messagePush = [MessagePush parseFromData:pushData];
    if (messagePush != nil) {
        NSString* recvtext = [NSString stringWithFormat:@"%@ : %@", messagePush.pb_from, messagePush.content];
        dispatch_queue_t mainQueue = dispatch_get_main_queue();
        dispatch_async(mainQueue, ^{
             [_recvContentField setText:recvtext];
        });
       
    }
}

@end
