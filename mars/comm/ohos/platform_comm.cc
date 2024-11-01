//
// Created by gjt on 10/18/24.
//
#include "mars/comm/platform_comm.h"

namespace mars {
namespace comm {
namespace {
std::shared_ptr<NetworkInfoCallback> s_default_network_cb = std::make_shared<NetworkInfoCallback>();
std::weak_ptr<NetworkInfoCallback> s_network_cb;

std::shared_ptr<NetworkInfoCallback> getNetworkInfoCallback() {
    auto cb = s_network_cb.lock();
    if (cb) {
        return cb;
    }
    return s_default_network_cb;
}
}  // namespace

void SetNetworkInfoCallback(const std::shared_ptr<NetworkInfoCallback>& _cb) {
    s_network_cb = _cb;
}
bool getProxyInfo(int& port, std::string& strProxy, const std::string& _host) {
    return getNetworkInfoCallback()->getProxyInfo(port, strProxy, _host);
}
bool getAPNInfo(APNInfo& info) {
    return getNetworkInfoCallback()->getAPNInfo(info);
}
NetType getNetInfo(bool realtime) {
    return getNetworkInfoCallback()->getNetInfo(realtime);
}
NetTypeForStatistics getNetTypeForStatistics() {
    return getNetworkInfoCallback()->getNetTypeForStatistics();
}
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _info) {
    return getNetworkInfoCallback()->getCurRadioAccessNetworkInfo(_info);
}
bool getCurWifiInfo(WifiInfo& _wifi_info, bool _force_refresh) {
    return getNetworkInfoCallback()->getCurWifiInfo(_wifi_info, _force_refresh);
}
bool getCurSIMInfo(SIMInfo& _sim_info, bool realtime) {
    return getNetworkInfoCallback()->getCurSIMInfo(_sim_info, realtime);
}
uint32_t getSignal(bool isWifi) {
    return getNetworkInfoCallback()->getSignal(isWifi);
}
bool isNetworkConnected() {
    return getNetworkInfoCallback()->isNetworkConnected();
}
bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ifip) {
    return getNetworkInfoCallback()->getIfAddrsIpv4HotSpot(_ifname, _ifip);
}
void OnPlatformNetworkChange() {
    // do nothing
}

int OSVerifyCertificate(const std::string& hostname, const std::vector<std::string>& certschain) {
    // todo tiemuhuaguo 证书校验
    return 0;
}
}  // namespace comm
}  // namespace mars