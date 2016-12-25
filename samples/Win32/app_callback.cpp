#include "stdafx.h"
#include "app_callback.h"

namespace mars {
	namespace app {
	
		std::string AppCallBack::GetAppFilePath() {
			return "e:\\temp";
		}

		AccountInfo AppCallBack::GetAccountInfo() {
			AccountInfo info;
			info.uin = 1;
			info.username = "test";
			return info;
		}

		unsigned int AppCallBack::GetClientVersion() {
			return 0;	//your app client version
		}

		DeviceInfo AppCallBack::GetDeviceInfo() {
			DeviceInfo info;
			info.devicename = "device name";
			info.devicetype = "device type";
			return info;
		}
	
	}
}
