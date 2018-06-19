//
//  PushNotifyDelegate.h
//  MacDemo
//
//  Created by chenzihao on 17/05/15.
//  Copyright © 2017年 chenzihao. All rights reserved.
//

#ifndef PushNotifyDelegate_h
#define PushNotifyDelegate_h

@protocol PushNotifyDelegate <NSObject>

@required -(void)notifyPushMessage:(NSData*)pushData withCmdId:(int)cmdId;

@end

#endif /* PushNotifyDelegate_h */
