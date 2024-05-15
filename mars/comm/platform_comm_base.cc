#include <mutex>
#include <utility>

#include "macro.h"
#include "platform_comm.h"

namespace mars {
namespace comm {

#ifndef __APPLE__
NO_DESTROY static std::function<bool(std::string&)> g_new_wifi_id_cb;
NO_DESTROY static std::mutex wifi_id_mutex;

std::function<bool(std::string&)> SetWiFiIdCallBack(std::function<bool(std::string&)> _cb) {
    std::lock_guard<std::mutex> lock(wifi_id_mutex);
    std::function<bool(std::string&)> old = g_new_wifi_id_cb;
    g_new_wifi_id_cb = std::move(_cb);
    return old;
}

void ResetWiFiIdCallBack() {
    std::lock_guard<std::mutex> lock(wifi_id_mutex);
    g_new_wifi_id_cb = nullptr;
}
bool IsWiFiIdCallBackExists() {
    std::lock_guard<std::mutex> lock(wifi_id_mutex);
    return g_new_wifi_id_cb != nullptr;
}
#endif

const char* kMarsDefaultNetworkIDNetLabel = "newnetid_default_netid";
const char* kMarsNetworkIDNetLabelPrefix = "newnetid_";
const char* kMarsWifiNetLabelPrefix = "wifi_";
const char* kMarsMobileNetLabelPrefix = "mobile_";
const char* kMarsDefaultNetLabel = "default";

int getCurrNetLabelImpl(std::string& netInfo, bool realtime) {
    netInfo = mars::comm::kMarsDefaultNetLabel;
    int nettype = getNetInfo(realtime);
    switch (nettype) {
        case kWifi: {
            WifiInfo info;
            getCurWifiInfo(info, realtime);
            netInfo = kMarsWifiNetLabelPrefix + info.ssid;
        } break;

        case kMobile: {
            SIMInfo info;
            getCurSIMInfo(info, realtime);
            netInfo = kMarsMobileNetLabelPrefix + info.isp_code;
        } break;
        default:
            break;
    }
    return nettype;
}

int getCurrNetLabel(std::string& netInfo) {
    return getCurrNetLabelImpl(netInfo, false);
}

int getRealtimeNetLabel(std::string& netInfo) {
    return getCurrNetLabelImpl(netInfo, true);
}

#ifndef __APPLE__
int getNetworkIDLabel(std::string& netInfo) {
    netInfo = mars::comm::kMarsDefaultNetLabel;
    int nettype = getNetInfo(false);
    {
        std::lock_guard<std::mutex> lock(wifi_id_mutex);
        if (g_new_wifi_id_cb && g_new_wifi_id_cb(netInfo) && !netInfo.empty()) {
            return nettype;
        }
    }

    // fallback
    return getCurrNetLabel(netInfo);
}
#endif

}  // namespace comm
}  // namespace mars
