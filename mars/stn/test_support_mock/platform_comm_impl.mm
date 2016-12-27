//
//  platform_comm_impl.mm
//  MicroMessenger
//
//  Created by zhoushaotao on 13-7-3.
//  Copyright (c) 2013年 Tencent. All rights reserved.
//
#include <string>
#import <Foundation/Foundation.h>

struct XLoggerInfo_t;

std::string getAppPrivatePath()
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* nsLibraryPath = (NSString*)[paths objectAtIndex:0];
    if([nsLibraryPath length] > 0)
    {
        return [[[NSString alloc] initWithFormat:@"%@/wechat/WechatPrivate", nsLibraryPath] UTF8String];
    }
    return "";
}

void printConsoleLog(const XLoggerInfo_t* _info, const char* _log)
{
    if (NULL==_info || NULL==_log) return;
    NSLog([NSString stringWithUTF8String:_log]);
}

