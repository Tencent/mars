//
//  PushNotifyDelegate.h
//  iOSDemo
//
//  Created by 曹少琨 on 16/12/27.
//  Copyright © 2016年 曹少琨. All rights reserved.
//

#ifndef PushNotifyDelegate_h
#define PushNotifyDelegate_h

@protocol PushNotifyDelegate <NSObject>

@required -(void)notifyPushMessage:(NSData*)pushData withCmdId:(int)cmdId;

@end

#endif /* PushNotifyDelegate_h */
