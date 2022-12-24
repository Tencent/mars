// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * appcomm.h
 *
 *  Created on: 2016年3月3日
 *      Author: caoshaokun
 */

#ifndef APPCOMM_INTERFACE_APPCOMM_H_
#define APPCOMM_INTERFACE_APPCOMM_H_

#include <string>
#include <stdint.h>
#include <memory>

#include "mars/comm/comm_data.h"

namespace mars {
namespace app {

struct AccountInfo {
	AccountInfo():uin(0), is_logoned(false){}
	int64_t uin;
	std::string username;
	bool is_logoned;
};

struct DeviceInfo {
	std::string devicename;
	std::string devicetype;
};
    
extern mars::comm::ProxyInfo GetProxyInfo(const std::string& _host);
extern std::string GetAppFilePath();
extern AccountInfo GetAccountInfo();
extern std::string GetUserName();
extern std::string GetRecentUserName();
extern unsigned int GetClientVersion();
extern DeviceInfo GetDeviceInfo();
extern double GetOsVersion();

#ifdef NATIVE_CALLBACK

    class AppLogicNativeCallback {
    public:
        AppLogicNativeCallback() = default;
        virtual ~AppLogicNativeCallback() = default;

        virtual mars::comm::ProxyInfo GetProxyInfo(const std::string& _host) {return mars::comm::ProxyInfo();}
        virtual std::string GetAppFilePath() {return "";}
        virtual AccountInfo GetAccountInfo() {return AccountInfo();}
        virtual std::string GetUserName() {return "";}
        virtual std::string GetRecentUserName() {return "";}
        virtual unsigned int GetClientVersion() {return 0;}
        virtual DeviceInfo GetDeviceInfo() {return DeviceInfo();}
    };
    extern void SetAppLogicNativeCallback(std::shared_ptr<AppLogicNativeCallback> _cb);

#endif //NATIVE_CALLBACK

}}

#endif /* APPCOMM_INTERFACE_APPCOMM_H_ */
