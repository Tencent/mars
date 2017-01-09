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

namespace mars {
namespace app {

struct AccountInfo {
	AccountInfo():uin(0){}
	int64_t uin;
	std::string username;
};

struct DeviceInfo {
	std::string devicename;
	std::string devicetype;
};
    
extern std::string GetAppFilePath();
extern AccountInfo GetAccountInfo();
extern std::string GetUserName();
extern std::string GetRecentUserName();
extern unsigned int GetClientVersion();
extern DeviceInfo GetDeviceInfo();
}}

#endif /* APPCOMM_INTERFACE_APPCOMM_H_ */
