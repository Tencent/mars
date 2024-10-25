#include "mars/comm/platform_comm.h"

#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace comm {

void OnPlatformNetworkChange() {
}

NetType getNetInfo(bool) {
    return kWifi;
}

NetTypeForStatistics getNetTypeForStatistics() {
    return NetTypeForStatistics::NETTYPE_UNKNOWN;
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
}  // namespace mars
