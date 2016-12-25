//
//  MessageItemsController.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "Main.pb.h"

#import "UINotifyDelegate.h"

@interface MessageItemsController : UITableViewController<UINotifyDelegate> {
    NSArray<Conversation*> * converSations;
}

@end
