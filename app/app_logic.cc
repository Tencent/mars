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


