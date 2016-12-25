//
//  MessagesController.h
//  mactest
//
//  Created by caoshaokun on 16/11/28.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "Main.pb.h"

#import "NetworkDelegate.h"
#import "UINotifyDelegate.h"

@interface MessagesController : NSTableView<UINotifyDelegate, NSTableViewDelegate, NSTableViewDataSource> {
    NSArray<Conversation*> * converSations;
}

@end
