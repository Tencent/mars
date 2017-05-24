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
//  MarsDemoEntryController.m
//  mactest
//
//  Created by caoshaokun on 16/11/28.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "DemoEntryController.h"
#import "TopicViewController.h"
#import "MessagesController.h"

@interface DemoEntryController () {
    NSTabViewController* _tabviewController;
    MessagesController* _messageController;
    TopicViewController* _topicController;
}

@end

@implementation DemoEntryController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}
-(void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if(![segue.destinationController isKindOfClass:[NSTabViewController class]])return;
    _tabviewController = segue.destinationController;
    for(NSViewController* childController in _tabviewController.childViewControllers) {
        if([childController isKindOfClass:[MessagesController class]]) {
            _messageController = (MessagesController*)childController;
            if(_messageController)[_messageController setHostController:self];
        }
        else if([childController isKindOfClass:[TopicViewController class]]) {
            _topicController = (TopicViewController*)childController;
            if(_topicController)[_topicController setHostController:self];
        }
    }
    _tabviewController.selectedTabViewItemIndex = 0;
    [_tabviewController removeChildViewControllerAtIndex:2];
}

-(void)setConversation:(Conversation*)conversation {
    if(!_tabviewController || !_messageController || !_topicController)
        return;
    [_tabviewController removeChildViewControllerAtIndex:1];
    [_topicController setConversation:conversation];
    [_tabviewController addChildViewController:_topicController];
    _tabviewController.selectedTabViewItemIndex = 1;
    
}
@end
