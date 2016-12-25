//
//  TopicViewController.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "UINotifyDelegate.h"

@interface TopicViewController : NSViewController<UINotifyDelegate> {
    NSString* text;
}

@property (weak) IBOutlet NSTextField *textField;
@property (weak) IBOutlet NSTextField *sendTextField;
@property (weak) IBOutlet NSTextField *recvTextField;

@end
