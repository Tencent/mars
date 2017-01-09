/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#ifdef WP8

#include "comm\platform_comm.h"

#include "runtime_comm.h"
#include "comm\xlogger\xlogger.h"
#include "runtime_utils.h"

using namespace PublicComponent;
using namespace Platform;
using namespace std;

bool isWifi() {
    return kWifi == getNetInfo();
}

int getStatisticsNetType() {
    return Runtime2Cs_Comm::Singleton()->getStatisticsNetType();
}



const std::string& getAppFilePath() {
    static std::string sAppFilePath;
    char rootpath[MAX_PATH];
    getIsoPath(rootpath);
    sAppFilePath.assign(rootpath);
    return sAppFilePath;
}

const std::string& getAppPrivateFilePath() {
    static std::string sAppPrivateFilePath;
    char rootpath[MAX_PATH];
    getIsoPath(rootpath);
    sAppPrivateFilePath.assign(rootpath);
    return sAppPrivateFilePath;
}

/*bool startAlarm(int64_t id, int after)
{
    return Runtime2Cs_Comm::Singleton()->startAlarm(id, after);
}

bool stopAlarm(int64_t id)
{
    return Runtime2Cs_Comm::Singleton()->stopAlarm(id);
}*/

void restartProcess() {
    // todo
}

bool getProxyInfo(int& /*port*/, std::string& strProxy, const std::string& /*_host*/) {
    // wp8 api implemented

    ProxyInfo^ info = Runtime2Cs_Comm::Singleton()->getProxyInfo();

    if (nullptr == info) return false;

    std::wstring wsProxy(info->strProxy->Data());
    strProxy.assign(wsProxy.begin(), wsProxy.end());
    return !strProxy.empty();
}

int getNetInfo() {
    return Runtime2Cs_Comm::Singleton()->getNetInfo();
}

bool getCurWifiInfo(WifiInfo& wifiInfo) {
    CurWifiInfo^ info = Runtime2Cs_Comm::Singleton()->getCurWifiInfo();

    // std::wstring wsSSID(info->ssid->Data());

    wifiInfo.ssid = String2stdstring(info->ssid);

    return false;
}


unsigned int getSignal(bool /*isWifi*/) {
    //// wp8 api implemented
    return 0;
}

bool isNetworkConnected() {
    return Runtime2Cs_Comm::Singleton()->isNetworkConnected();
}


String^ char2PString(const char* cStr) {
    if (NULL == cStr) return "";

    std::string str = std::string(cStr);
    std::wstring wstr = std::wstring(str.begin(), str.end());
    String^ psStr = ref new String(wstr.c_str());
    return psStr;
}

void ConsoleLog(const XLoggerInfo* _info, const char* _log) {
    Runtime2Cs_Comm::Singleton()->ConsoleLog((int)_info->level, char2PString(_info->tag), char2PString(_info->filename), char2PString(_info->func_name), _info->line, char2PString(_log));
}

#endif

