// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

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
 * app_logic.cc
 *
 *  Created on: 2016/3/3
 *      Author: caoshaokun
 */

#include "mars/app/app_logic.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace app {

static Callback* sg_callback = NULL;

void SetCallback(Callback* const callback) {
	sg_callback = callback;
}

#ifndef ANDROID

    std::string GetAppFilePath() {
        xassert2(sg_callback != NULL);
        return sg_callback->GetAppFilePath();
    }
    
	AccountInfo GetAccountInfo() {
		xassert2(sg_callback != NULL);
		return sg_callback->GetAccountInfo();
	}

	std::string GetUserName() {
		xassert2(sg_callback != NULL);
		AccountInfo info = sg_callback->GetAccountInfo();
		return info.username;
	}

	std::string GetRecentUserName() {
		xassert2(sg_callback != NULL);
		return GetUserName();
	}

	unsigned int GetClientVersion() {
		xassert2(sg_callback != NULL);
		return sg_callback->GetClientVersion();
	}


	DeviceInfo GetDeviceInfo() {
		xassert2(sg_callback != NULL);
		return sg_callback->GetDeviceInfo();
	}


#endif

}
}


