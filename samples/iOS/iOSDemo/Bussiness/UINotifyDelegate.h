//
//  UINotifyDelegate.h
//  iOSDemo
//
//  Created by caoshaokun on 16/11/24.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#ifndef UINotifyDelegate_h
#define UINotifyDelegate_h

@protocol UINotifyDelegate <NSObject>

@required -(NSData*)requestSendData;
@required -(int)notifyUIWithResponse:(NSData*)responseData;

@end

#endif /* UINotifyDelegate_h */
