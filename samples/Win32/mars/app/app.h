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
