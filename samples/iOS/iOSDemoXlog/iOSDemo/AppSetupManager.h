//
//  AppSetupManager.h
//  mactest
//
//  Created by caoshaokun on 16/5/10.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#ifndef AppSetupManager_h
#define AppSetupManager_h

#import <Foundation/Foundation.h>

@interface AppSetupManager : NSObject

+ (AppSetupManager *)shareInstance;
- (void)setupApp;

- (void)setupPublicCompenont;


@end

#endif /* AppSetupManager_h */
