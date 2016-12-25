//
//  TopicViewController.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/25.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "UINotifyDelegate.h"

@interface TopicViewController : UIViewController<UINotifyDelegate> {
    NSString* text;
}

@property (weak, nonatomic) IBOutlet UITextField *textField;
@property (weak, nonatomic) IBOutlet UITextField *contentField;
@property (weak, nonatomic) IBOutlet UITextField *recvContentField;

@end
