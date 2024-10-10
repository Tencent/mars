#include "mars/comm/platform_comm.h"

#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace comm {

void OnPlatformNetworkChange() {
}

int getNetInfo(bool) {
    return kWifi;
}

int getNetTypeForStatistics() {
    return (int)NetTypeForStatistics::NETTYPE_UNKNOWN;
}

bool getCurRadioAccessNetworkInfo(struct RadioAccessNetworkInfo& _info) {
    return false;
}

bool getCurWifiInfo(WifiInfo& _wifi_info, bool _force_refresh) {
    return false;
}

bool getCurSIMInfo(SIMInfo& _sim_info, bool) {
    return false;
}

bool getAPNInfo(APNInfo& info) {
    return false;
}

unsigned int getSignal(bool isWifi) {
    return 0;
}

bool isNetworkConnected() {
    return true;
}

bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ifip) {
    return false;
}

int OSVerifyCertificate(const std::string& hostname, const std::vector<std::string>& certschain) {
    // todo tiemuhuaguo
    return 0;
}

}  // namespace comm

#ifndef OHOS
namespace xlog {
void ConsoleLog(const XLoggerInfo* _info, const char* _log) {
    if (NULL == _info || NULL == _log)
        return;
    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"   // fatal
    };
    char strFuncName[128] = {0};
    ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));
    const char* file_name = ExtractFileName(_info->filename);
    char log[16 * 1024] = {0};
    snprintf(log,
             sizeof(log),
             "[%s][%s][%s, %s, %d][%s\n",
             levelStrings[_info->level],
             NULL == _info->tag ? "" : _info->tag,
             file_name,
             strFuncName,
             _info->line,
             _log);
    printf("%s", log);
}
}  // namespace xlog
#endif  // ifndef OHOS

}  // namespace mars
