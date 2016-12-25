#pragma once

#include "mars/app/app.h"
#include "mars/app/app_logic.h"

namespace mars {
	namespace app {
	
class AppCallBack : public Callback {

public:

	AppCallBack() {}
	~AppCallBack() {}

	virtual std::string GetAppFilePath();

	virtual AccountInfo GetAccountInfo();

	virtual unsigned int GetClientVersion();

	virtual DeviceInfo GetDeviceInfo();
};

	}
}

