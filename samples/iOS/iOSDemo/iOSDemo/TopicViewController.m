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

#import "Chat.pbobjc.h"
#import "Messagepush.pbobjc.h"

#import "CGITask.h"
#import "CommandID.h"
#import "NetworkService.h"

#import "Constants.h"
#import "LogUtil.h"

#import "AppDelegate.h"
@import CoreText;

@interface TopicViewController () <UITableViewDelegate, UITableViewDataSource, UITextFieldDelegate>
@property (nonatomic, strong) NSMutableArray *messages;
@end

@implementation TopicViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = _conversation.notice;
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    self.tableView.tableFooterView = [UIView new];
    self.textField.delegate = self;
    _messages = [NSMutableArray new];
    [[NetworkService sharedInstance] addPushObserver:self withCmdId:kPushMessageCmdId];
}

- (NSString *)username {
    return [(AppDelegate *)[UIApplication sharedApplication].delegate username];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    if (textField.text.length > 0) {
        [self sendMessage];
    } else {
        [textField resignFirstResponder];
    }
    return NO;
}

#pragma mark - Message Delegates

-(NSData*)requestSendData {
    SendMessageRequest *sendMsgRequest = [SendMessageRequest new];
    sendMsgRequest.from = [self username];
    sendMsgRequest.to = @"all";
    sendMsgRequest.text = _textField.text;
    sendMsgRequest.accessToken = @"123456";
    sendMsgRequest.topic = _conversation.topic;
    LOG_INFO(kModuleViewController, @"send msg to topic:%@", _conversation.notice);
    NSData* data = [sendMsgRequest data];
    dispatch_async(dispatch_get_main_queue(), ^{
        _textField.text = @"";
    });
    return data;
}


-(int)onPostDecode:(NSData*)responseData {
    SendMessageResponse *sendMsgResponse = [SendMessageResponse parseFromData:responseData error:nil];
    dispatch_async(dispatch_get_main_queue(), ^{
        NSString *recvtext = [NSString stringWithFormat:@"%@ : %@", sendMsgResponse.from, sendMsgResponse.text];
        [self.messages addObject:recvtext];
        [self.tableView reloadData];
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.messages.count-1 inSection:0];
        [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionBottom animated:YES];
    });
    return sendMsgResponse.errCode == 0 ? 0 : -1;
}

- (void)sendMessage {
    CGITask *sendMsgCGI = [[CGITask alloc] initAll:ChannelType_LongConn AndCmdId:kSendMsg AndCGIUri:@"/mars/sendmessage" AndHost:@"www.marsopen.cn"];
    [[NetworkService sharedInstance] startTask:sendMsgCGI ForUI:self];
}

- (void)notifyPushMessage:(NSData*)pushData withCmdId:(int)cmdId {
    MessagePush* messagePush = [MessagePush parseFromData:pushData error:nil];
    if (messagePush != nil) {
        dispatch_async(dispatch_get_main_queue(), ^{
            NSString *recvtext = [NSString stringWithFormat:@"%@ : %@", messagePush.from, messagePush.content];
            [self.messages addObject:recvtext];
            [self.tableView reloadData];
            NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.messages.count-1 inSection:0];
            [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionBottom animated:YES];
        });
    }
}

- (int)onTaskEnd:(uint32_t)tid errType:(uint32_t)errtype errCode:(uint32_t)errcode {
    
    return 0;
}

#pragma mark - UITableView Delegates

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return _messages.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"cell" forIndexPath:indexPath];
    cell.textLabel.font = [UIFont fontWithName:@"PingFangSC-Regular" size:17];
    cell.textLabel.text = _messages[indexPath.row];
    cell.textLabel.textAlignment = NSTextAlignmentLeft;
    cell.textLabel.numberOfLines = 0;
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    NSString *string = _messages[indexPath.row];
    NSAttributedString *attr = [[NSAttributedString alloc] initWithString:string
                                                               attributes:@{NSForegroundColorAttributeName:[UIColor darkTextColor],
                                                                            NSFontAttributeName: [UIFont fontWithName:@"PingFangSC-Regular" size:17]}];
    CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString((CFMutableAttributedStringRef)attr);
    CGSize suggestedSize = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, CFRangeMake(0, 0), NULL, CGSizeMake(SCREEN_WIDTH, CGFLOAT_MAX), NULL);
    CFRelease(framesetter);
    return suggestedSize.height + 25;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [_textField resignFirstResponder];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
