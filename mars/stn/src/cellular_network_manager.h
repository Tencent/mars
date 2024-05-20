//
// Created by Changpeng Pan on 2023/6/12.
//

#ifndef MMNET_CELLULAR_NETWORK_MANAGER_H
#define MMNET_CELLULAR_NETWORK_MANAGER_H

#include <mutex>

#include "mars/comm/ini.h"
#include "mars/boot/context.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

class CellularNetworkManager {
 public:
    CellularNetworkManager(boot::Context* _context);
 public:
    void OnTaskEvent(const TaskProfile& _task);
    bool IsTrafficLimit();

 private:
    void __Init();
    std::string __GetCurrentDate();
    bool __CheckDate();
    void __SaveData();

 private:
    boot::Context* context_;
    std::mutex mutex_;
    INI cellular_network_ini_;
    bool is_init_ = false;
    std::string recored_date_;
    int64_t total_send_size_ = 0;
    int64_t total_revc_size_ = 0;
};

}  // namespace stn
}  // namespace mars

#endif  // MMNET_CELLULAR_NETWORK_MANAGER_H
