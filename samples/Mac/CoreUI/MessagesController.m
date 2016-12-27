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
//  MessagesController.m
//  mactest
//
//  Created by caoshaokun on 16/11/28.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "MessagesController.h"

#import "TopicViewController.h"
#import "LogUtil.h"

#import "CommandID.h"
#import "CGITask.h"
#import "NetworkService.h"

@implementation MessagesController

- (id)initWithCoder:(NSCoder *)coder {
    if (self = [super initWithCoder:coder]) {
        self->converSations = [[NSArray alloc] init];
        self.dataSource = self;
        self.delegate = self;
        [self setDoubleAction:NSSelectorFromString(@"doubleClick:")];
    }
    
    CGITask *convlstCGI = [[CGITask alloc] initAll:ChannelType_ShortConn AndCmdId:kConvLst AndCGIUri:@"/mars/getconvlist" AndHost:@"localhost"];
    [[NetworkService sharedInstance] startTask:convlstCGI ForUI:self];
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
}

- (NSInteger)numberOfRows {
    return [self->converSations count];
}

- (nullable NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row {
    NSTableCellView *cellView = [tableView makeViewWithIdentifier:@"message" owner:self];
    
    cellView.textField.stringValue = self->converSations[row].topic;
    
    return cellView;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [self->converSations count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;
{
    return [self->converSations objectAtIndex:row];
}

- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return NO;
}

- (void) doubleClick: (id)sender
{
    NSInteger rowNumber = [self clickedRow];
    
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return YES;
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
        [self reloadData];
    });
    
    return [self->converSations count] > 0 ? 0 : -1;
    
}

@end
