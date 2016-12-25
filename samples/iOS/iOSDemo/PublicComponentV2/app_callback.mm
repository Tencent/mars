//
//  appcomm_callback.mm
//  MPApp
//
//  Created by yanguoyue on 16/3/3.
//  Copyright © 2016年 tencent. All rights reserved.
//

#include "app_callback.h"

#include <sstream>
#include <mars/comm/autobuffer.h>

namespace mars {
    namespace app {

AppCallBack* AppCallBack::instance_ = NULL;

AppCallBack* AppCallBack::Instance() {
    if(instance_ == NULL) {
        instance_ = new AppCallBack();
    }
    
    return instance_;
}

void AppCallBack::Release() {
    delete instance_;
    instance_ = NULL;
}

// return your app path
std::string AppCallBack::GetAppFilePath(){
    return "";
}
        
AccountInfo AppCallBack::GetAccountInfo() {
    AccountInfo info;
    
    return info;
}

unsigned int AppCallBack::GetClientVersion() {
    
    return 0;
}

DeviceInfo AppCallBack::GetDeviceInfo() {
    DeviceInfo info;

    info.devicename = "";
    info.devicetype = 1;
    
    return info;
}

}}
