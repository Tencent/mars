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
//  TopicViewController.m
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "TopicViewController.h"

#import "Chat.pb.h"

#import "CGITask.h"
#import "CommandID.h"
#import "NetworkService.h"

@interface TopicViewController ()

@end

@implementation TopicViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

-(NSData*)requestSendData {
    SendMessageRequest* sendMsgRequest = [[[[[[[SendMessageRequest builder] setFrom:@"anonymous"] setTo:@"all"] setText:self->text] setAccessToken:@"123456"] setTopic:@"0"] build];
    NSData* data = [sendMsgRequest data];
    return data;
}


-(int)onPostDecode:(NSData*)responseData {
    SendMessageResponse *sendMsgResponse = [SendMessageResponse parseFromData:responseData];
    
    [_recvTextField setStringValue:sendMsgResponse.errMsg];
    
    return sendMsgResponse.errCode == 0 ? 0 : -1;
}

- (IBAction)sendMessage:(id)sender {
    CGITask *sendMsgCGI = [[CGITask alloc] initAll:ChannelType_LongConn AndCmdId:kSendMsg AndCGIUri:@"/mars/sendmessage" AndHost:@"www.marsopen.cn"];
    [[NetworkService sharedInstance] startTask:sendMsgCGI ForUI:self];
    
    self->text = _textField.stringValue;
    [_textField setStringValue:self->text];
    
    [_sendTextField setStringValue:@""];
}

- (int)onTaskEnd:(uint32_t)tid errType:(uint32_t)errtype errCode:(uint32_t)errcode {
    
    return 0;
}

@end
