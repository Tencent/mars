/**
 * created on : 2012-11-28
 * author : 叶润桂
 */

//#include "platform_logic.h"

#include <sstream>
#include <string>

class AutoBuffer;

struct AccountInfo
{
	AccountInfo():uin(0),qquin(0){}
	int uin;
    std::string username;
    int qquin;
};

std::string getUserName() { return "test";}
std::string getRecentUserName()
{
    return "test";
}

struct AccountInfo getAccountInfo()
{
    AccountInfo info;
	info.username = "test";
	info.uin = 0;
	info.qquin = 0;

    return info;
}

unsigned int getClientVersion() {
    return 0;
}

void getUplodLogExtrasInfo(std::string& _descData)
{
}

const std::string& getAppFilePath();

void getWatchDogPath(std::string& aPath)
{
}

void getKVCommPath(std::string& aPath)
{
}


void getCrashFilePath(std::string& filePath, int timespan) {
    // ipxx upload crash file
}


std::string getCurLanguage()
{
    return "";
}

std::string getDeviceType()
{
    return "";
}

std::string getDeviceInfo() {
    return "";
}

