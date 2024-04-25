//
// Created by Changpeng Pan on 2023/6/9.
//

#ifndef MMNET_APP_CONFIG_H
#define MMNET_APP_CONFIG_H

#include "mars/app/app.h"
#include <map>

namespace mars {
namespace app {

class AppConfig {
 public:
    AppConfig();
    
 public:
    void UpdateConfig(Config _config);

 public:
    bool IsCellularNetworkEnable();
    int CellularNetworkConnIndex();
    int CellularNetworkTrafficLimit();


 private:
    Config config_;
};
}  // namespace app
}  // namespace mars

#endif  // MMNET_APP_CONFIG_H
