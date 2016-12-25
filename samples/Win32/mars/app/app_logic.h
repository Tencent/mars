/*
 * app_logic.h
 *
 *  Created on: 2016/3/3
 *      Author: caoshaokun
 */

#ifndef APPCOMM_INTERFACE_APPCOMM_LOGIC_H_
#define APPCOMM_INTERFACE_APPCOMM_LOGIC_H_

#include <string>

#include "app.h"

class AutoBuffer;

namespace mars {
namespace app {

	class Callback {
	public:
		virtual ~Callback() {};

        virtual std::string GetAppFilePath() = 0;
        
		virtual AccountInfo GetAccountInfo() = 0;

		virtual unsigned int GetClientVersion() = 0;

		virtual DeviceInfo GetDeviceInfo() = 0;

	};

	void SetCallback(Callback* const callback);
}}


#endif /* APPCOMM_INTERFACE_APPCOMM_LOGIC_H_ */
