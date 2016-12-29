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
//  MessageItemsController.m
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "MessageItemsController.h"

#import "TopicViewController.h"
#import "LogUtil.h"

#import "Main.pb.h"

#import "CGITask.h"
#import "CommandID.h"
#import "NetworkService.h"

@interface MessageItemsController ()

@end

@implementation MessageItemsController

- (void)loadView {
    [super loadView];
    
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    
    converSations = [[NSArray alloc] init];
    CGITask *convlstCGI = [[CGITask alloc] initAll:ChannelType_ShortConn AndCmdId:kConvLst AndCGIUri:@"/mars/getconvlist" AndHost:@"www.marsopen.cn"];
    [[NetworkService sharedInstance] startTask:convlstCGI ForUI:self];
}

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self->converSations.count;
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell=[[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
    [cell setHidden:NO];
    cell.textLabel.text = converSations[indexPath.row].notice;
    cell.detailTextLabel.text = converSations[indexPath.row].notice;
    return cell;
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    NSString* topicName = converSations[indexPath.row].topic;
    TopicViewController *topicControl = [[self storyboard]instantiateViewControllerWithIdentifier:@"Topic"];
    topicControl.topicName = topicName;
    [[self navigationController] pushViewController:topicControl animated:YES];
}

- (NSData*)requestSendData {
    
    ConversationListRequest* convlstRequest = [[[[ConversationListRequest builder] setType:0] setAccessToken:@"123456"] build];
    NSData* data = [convlstRequest data];
    
    return data;
}

- (int)notifyUIWithResponse:(NSData*)responseData {
    
    ConversationListResponse *convlstResponse = [ConversationListResponse parseFromData:responseData];
    self->converSations = convlstResponse.list;
    LOG_INFO(kModuleViewController, @"recv conversation list, size: %lu", (unsigned long)[self->converSations count]);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.tableView reloadData];
    });
    
    return [self->converSations count] > 0 ? 0 : -1;

}

@end
