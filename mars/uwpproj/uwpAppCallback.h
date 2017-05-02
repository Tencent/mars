#pragma once
#include "app/app_logic.h"

class uwpAppCallback : public mars::app::Callback {

public:
	virtual ~uwpAppCallback() {};

	virtual std::string GetAppFilePath();

	virtual mars::app::AccountInfo GetAccountInfo();

	virtual unsigned int GetClientVersion();

	virtual mars::app::DeviceInfo GetDeviceInfo();

};


