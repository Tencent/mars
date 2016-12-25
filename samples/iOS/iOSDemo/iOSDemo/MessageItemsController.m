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
    TopicViewController *topicControl = [[self storyboard]instantiateViewControllerWithIdentifier:@"Topic"];
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
    
    [self.tableView reloadData];
    
    return [self->converSations count] > 0 ? 0 : -1;

}

@end
