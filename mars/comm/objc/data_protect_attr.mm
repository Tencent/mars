// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  data_protect_attr.m
//  comm
//
//  Created by yanguoyue on 15/9/18.
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
