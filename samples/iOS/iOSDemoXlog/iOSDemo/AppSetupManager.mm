//
//  AppSetupManager.m
//  mactest
//
//  Created by caoshaokun on 16/5/10.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#import "AppSetupManager.h"

#include <mars/xlog/xlogger.h>
#include <mars/xlog/appender.h>


@interface AppSetupManager(){
}
@end

@implementation AppSetupManager

+ (AppSetupManager *)shareInstance{
    static AppSetupManager *_instance = nil;
    
    @synchronized (self) {
        if (_instance == nil) {
            _instance = [[self alloc] init];
        }
    }
    
    return _instance;
}

- (void)setupApp{
    
    [self setupPublicCompenont];
    
}

#pragma mark - PublicComponent
- (void)setupPublicCompenont {

    xinfo2("%s %d", "1234", 1);
    
    
}



- (void)testDownload {
    
}

@end
