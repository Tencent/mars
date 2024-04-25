//
// Created by Changpeng Pan on 2023/6/9.
//

#include "app_config.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace app {

AppConfig::AppConfig() {
}

void AppConfig::UpdateConfig(Config _config) {
    config_ = _config;
    xinfo2(TSF "update config %_", config_.dump());
}

bool AppConfig::IsCellularNetworkEnable() {
    return config_.cellular_network_enable;
}

int AppConfig::CellularNetworkConnIndex() {
    return config_.cellular_network_conn_index;
}

int AppConfig::CellularNetworkTrafficLimit() {
    return config_.cellular_network_traffic_limit_mb;
}

}  // namespace app
}  // namespace mars
