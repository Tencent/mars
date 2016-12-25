//
//  data_protect_attr.m
//  comm
//
//  Created by yanguoyue on 15/9/18.
//  Copyright © 2015年 Tencent. All rights reserved.
//
#import "comm/objc/data_protect_attr.h"

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <Foundation/Foundation.h>
#endif


bool setAttrProtectionNone(const char* _path) {
    
#if !TARGET_OS_IPHONE
    return true;
#else
    
    NSString* path = [[NSString alloc] initWithUTF8String:_path];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:path]) {
        [path release];
        return false;
    }
    
    NSDictionary* attr = [NSDictionary dictionaryWithObject:NSFileProtectionNone forKey:NSFileProtectionKey];
    
    BOOL ret = [fileManager setAttributes:attr ofItemAtPath:path error:nil];
    
    [path release];
    
    return ret;
#endif
    
}
