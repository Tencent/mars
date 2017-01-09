
#include "../../interface/app.h"
#include "runtime_logic.h"
#include "xlogger.h"
#include "comm\platform_comm.h"
#include "runtime_utils.h"

using namespace PublicComponent;
using namespace Platform;

std::string getUserNameNoLog()
{
	String^ psUserName = Runtime2Cs_Logic::Singleton()->getUserName();

	//std::wstring wsUsername(psUserName->Data());
	std::string sUsername = String2stdstring(psUserName);
	//sUsername.assign(wsUsername.begin(), wsUsername.end());

	return sUsername;
}

std::string getUserName()
{
	xverbose_function();

	std::string userName = getUserNameNoLog();
    xdebug2(TSF"userName= %0", userName);
    return userName;
}

//  ���ص�ǰ�û������û�У�����֮ǰ��½���Ǹ��û�����Ҫ���ؿվͿ���
//wp�ͷ��ص�ǰusername
std::string getRecentUserName()
{
	xverbose_function();

	return getUserName();
}

unsigned int getClientVersionNoLog()
{
	return Runtime2Cs_Logic::Singleton()->getClientVersion();
}

unsigned int getClientVersion()
{
	xverbose_function();

	unsigned int clientVersion = getClientVersionNoLog();

    xdebug2(TSF"clientVersion= %0", clientVersion);
    return clientVersion;	 
}


AccountInfo getAccountInfo()
{
	xverbose_function();
	AccountInfo accountInfo;
	AccountInfoRef^ inforef = Runtime2Cs_Logic::Singleton()->getAccountInfo();
	if(inforef != nullptr)
	{
		accountInfo.qquin = inforef->qquin;
	    accountInfo.uin = inforef->uin;
	    //std::wstring wsUsername(inforef->username->Data());
	    accountInfo.username = String2stdstring(inforef->username);
	}

	return accountInfo;
}

void getUplodLogExtrasInfo(std::string& _extrasinfo)
{
	xverbose_function();
	String^ psExInfo = Runtime2Cs_Logic::Singleton()->getUplodLogExtrasInfo();
	
	//std::wstring wsExinfo2(TSFpsExInfo->Data());
	_extrasinfo = String2stdstring(psExInfo);
}
void getWatchDogPath(std::string& _watchDogPath)
{
	xverbose_function();
	_watchDogPath = getAppFilePath();
	_watchDogPath += "\\watchdog\\";
}
void getKVCommPath(std::string& _kvCommPath)
{
	xverbose_function();
	_kvCommPath = getAppFilePath();
	_kvCommPath += "\\kvcomm\\";
}
void getCrashFilePath(std::string& filePath, int timespan)
{
	xverbose_function();
	String^ psfilePath = Runtime2Cs_Logic::Singleton()->getCrashFilePath(timespan);
	
	//std::wstring wsfilePath(psfilePath->Data());
	filePath = String2stdstring(psfilePath);
}

unsigned int getNextNoopTime()
{
	return Runtime2Cs_Logic::Singleton()->getNextNoopTime();
}
unsigned int getNoopInterval()
{
	return Runtime2Cs_Logic::Singleton()->getNoopInterval();
}

std::string getDeviceType()
{
	return String2stdstring(Runtime2Cs_Logic::Singleton()->getDeviceType());
}

std::string getCurLanguage()
{
	String^ curLanguage = Runtime2Cs_Logic::Singleton()->getCurLanguage();
	return String2stdstring(curLanguage);
}

std::string getDeviceInfo()
{
	return "";
}

