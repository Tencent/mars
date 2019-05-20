
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

	ProxyInfo^ proxy = callback->GetProxyInfo();
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

	return callback->GetNetInfo();
}

bool getCurRadioAccessNetworkInfo(struct RadioAccessNetworkInfo& _info)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;

	CurRadioAccessNetworkInfo^ info = callback->GetCurRadioAccessNetworkInfo();
	if (info == nullptr)
		return false;
	_info.radio_access_network = String2stdstring(info->radio_access_network);
	return true;
}

bool getCurWifiInfo(WifiInfo& wifiInfo)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;

	CurWifiInfo^ wifi = callback->GetCurWifiInfo();
	if (wifi == nullptr)
		return false;
	wifiInfo.ssid = String2stdstring(wifi->ssid);
	wifiInfo.bssid = String2stdstring(wifi->bssid);
	return true;
}

bool getCurSIMInfo(SIMInfo& simInfo)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;

	CurSIMInfo^ sim = callback->GetCurSIMInfo();
	if (sim == nullptr)
		return false;
	simInfo.isp_code = String2stdstring(sim->isp_code);
	simInfo.isp_name = String2stdstring(sim->isp_name);
	return true;
}

bool getAPNInfo(APNInfo& apninfo)
{

	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;

	CurAPNInfo^ newinfo = callback->GetAPNInfo();
	if (newinfo == nullptr)
		return false;
	apninfo.nettype = newinfo->nettype;
	apninfo.sub_nettype = newinfo->sub_nettype;
	apninfo.extra_info = String2stdstring(newinfo->extra_info);

	return true;
}

unsigned int getSignal(bool bIsWifi/*isWifi*/)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return 0;

	return callback->GetSignal(bIsWifi);
}

bool isNetworkConnected()
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback) return false;

	return callback->IsNetworkConnected();

}

bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ifip)//???
{
	return false;
}

void ConsoleLog(const XLoggerInfo* _info, const char* _log) 
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
			Platform::String^ tag = stdstring2String(_info->tag == NULL ? "" : _info->tag);
			Platform::String^ filename = stdstring2String(_info->filename == NULL ? "" : _info->filename);
			Platform::String^ funcname = stdstring2String(_info->func_name == NULL ? "" : _info->func_name);
			Platform::String^ log = stdstring2String(_log);
			callback->ConsoleLog(_info->level, tag, filename, funcname, _info->line, log);
		}

	}
}

