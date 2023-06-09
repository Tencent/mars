


//
// Created by Changpeng Pan on 2023/5/22.
//

#ifndef MMNET_CELLULAR_NETWORK_LOGIC_H
#define MMNET_CELLULAR_NETWORK_LOGIC_H

#include "mars/comm/singleton.h"
#include "mars/stn/task_profile.h"
#include "mars/comm/ini.h"
#include "mars/stn/stn.h"

namespace mars {
namespace stn {

/**
 * 当设备连接wifi,但又走了移动网络的相关逻辑
 *
 */
class CellularNetworkLogic {
 public:
    SINGLETON_INTRUSIVE(CellularNetworkLogic, new CellularNetworkLogic, delete);
    CellularNetworkLogic();
    
 public:
    void OnTaskEvent(const TaskProfile& _task_profile);
    //记录各任务的发送数据与接收数据，后面可用于判断是不是走手机网络
    bool IsOverSize();
    
    
private:
    void __Init();
    std::string __GetCurrentDate();
    bool __CheckDate();
    void __SaveData();
    
    
private:
    std::mutex mutex_;
    INI cellular_network_ini_;
    bool is_init_ = false;
    std::string recored_date_;
    //size_t max_send_size_ = CELLULAR_NETWORK_MAX_SEND_SIZE;
    //size_t max_revc_size_ = CELLULAR_NETWORK_MAX_REVC_SIZE;
    int64_t max_trafice_size = CELLULAR_NETWORK_MAX_TRAFFIC_MB * 1024 * 1024;
    int64_t total_send_size_ = 0;
    int64_t total_revc_size_ = 0;
    
};

}
}


#endif  // MMNET_CELLULAR_NETWORK_LOGIC_H
