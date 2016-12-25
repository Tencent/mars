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

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(NSData*)requestSendData {
    SendMessageRequest* sendMsgRequest = [[[[[[SendMessageRequest builder] setFrom:@"caoshaokun"] setTo:@"all"] setText:self->text] setAccessToken:@"123456"] build];
    NSData* data = [sendMsgRequest data];
    return data;
}


-(int)notifyUIWithResponse:(NSData*)responseData {
    SendMessageResponse *sendMsgResponse = [SendMessageResponse parseFromData:responseData];
    
    [_recvContentField setText:sendMsgResponse.errMsg];
    
    return sendMsgResponse.errCode == 0 ? 0 : -1;
}

- (IBAction)sendMessage:(id)sender forEvent:(UIEvent *)event {
    CGITask *sendMsgCGI = [[CGITask alloc] initAll:ChannelType_ShortConn AndCmdId:kSendMsg AndCGIUri:@"/mars/sendmessage" AndHost:@"www.marsopen.cn"];
    [[NetworkService sharedInstance] startTask:sendMsgCGI ForUI:self];
    
    self->text = _textField.text;
    [_contentField setText:self->text];
    
    [_textField setText:@""];
}

@end
