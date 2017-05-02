
#include "boost/utility/result_of.hpp"
#include "comm\platform_comm.h"

#include "runtime2cs.h"
#include "comm\xlogger\xlogger.h"
#include "runtime_utils.h"

#include "mars/comm/ptrbuffer.h"
using namespace mars;
using namespace Platform;
using namespace std;

bool getProxyInfo(int& _port, std::string& _str_proxy, const std::string& _host)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();
	if (nullptr == callback)
	{
		return false;
	}

	ProxyInfo^ proxy = callback->getProxyInfo();
	if (nullptr == proxy)
	{
		return false;
	}

	_port = proxy->port;
	std::wstring wsProxy(proxy->strProxy->Data());
	_str_proxy.assign(wsProxy.begin(), wsProxy.end());
	return !_str_proxy.empty();
}


int getNetInfo()
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return -1;

	return callback->getNetInfo();
}



bool getCurRadioAccessNetworkInfo(struct RadioAccessNetworkInfo& _info)//???
{
	return false;
}

bool getCurWifiInfo(WifiInfo& wifiInfo)
{
	wifiInfo.ssid = "unkown";
	return true;
}


bool getCurSIMInfo(SIMInfo& simInfo)
{
	simInfo.isp_code = "0";
	simInfo.isp_name = "unkown";
	return true;
}

bool getAPNInfo(APNInfo& apninfo)
{

	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;

	RuntimeNewNetInterfaceInfo^ newinfo = callback->getNewNetInferfaceInfo();
	if (newinfo == nullptr)
		return false;
	apninfo.nettype = newinfo->interfaceType;
	apninfo.sub_nettype = newinfo->interfaceSubType;
	apninfo.extra_info = String2stdstring(newinfo->interfaceName);

	return true;
}


unsigned int getSignal(bool bIsWifi/*isWifi*/)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return 0;

	return callback->getSignal(bIsWifi);
}

bool isNetworkConnected()
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;


	return callback->isNetworkConnected();

}



bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ifip)//???
{
	return false;
}


void log_formater(const XLoggerInfo* _info, const char* _logbody, PtrBuffer& _log)//???
{
}
void ConsoleLog(const XLoggerInfo* _info, const char* _log)//???
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();
	if(_info == NULL)
	{ 
		if (_log != NULL)
		{
			Platform::String^ log = stdstring2String(_log);
			callback->ConsoleLog(0, "", "", "", 0, log);
		}
	}
	else
	{
		if (_log != NULL)
		{
			Platform::String^ tag = stdstring2String(_info->tag);
			Platform::String^ filename = stdstring2String(_info->filename);
			Platform::String^ funcname = stdstring2String(_info->func_name);
			Platform::String^ log = stdstring2String(_log);
			callback->ConsoleLog(_info->level, tag, filename, funcname, _info->line, log);
		}

	}
}

