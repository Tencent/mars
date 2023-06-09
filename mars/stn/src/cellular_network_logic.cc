//
// Created by Changpeng Pan on 2023/5/22.
//

#include "cellular_network_logic.h"

#include "mars/app/app.h"
#include "mars/comm/xlogger/xlogger.h"

static const char* const kINIDefaultSelect = "default";
static const char* const kINIFileName = "/host/cellular.ini";
static const char* const kINIKeyRecoredDate = "recored_date";
//static const char* const kINIKeyMaxSendSize = "max_send";
//static const char* const kINIKeyMaxRevcSize = "max_revc";
static const char* const kINIKeyMaxTrafficSize = "max_traffic";
static const char* const kINIKeyTotalSendSize = "total_send";
static const char* const kINIKeyTotalRevcSize = "total_revc";

using namespace mars::app;

namespace mars {
namespace stn {

CellularNetworkLogic::CellularNetworkLogic() : cellular_network_ini_(GetAppFilePath() + kINIFileName) {
    xdebug2(TSF "[dual-channel] ini file:%_", GetAppFilePath() + kINIFileName);
    __Init();
}

void CellularNetworkLogic::OnTaskEvent(const TaskProfile& _task_profile) {
    xdebug2(TSF
            "[dual-channel] is bind cellular network:%_, sent_size:%_, send_data_size:%_, received_size:%_, "
            "receive_data_size:%_",
            _task_profile.transfer_profile.connect_profile.is_bind_cellular_network,
            _task_profile.transfer_profile.sent_size,
            _task_profile.transfer_profile.send_data_size,
            _task_profile.transfer_profile.received_size,
            _task_profile.transfer_profile.receive_data_size);

    if (__CheckDate()) {
        total_send_size_ += _task_profile.transfer_profile.send_data_size;
        total_revc_size_ += _task_profile.transfer_profile.receive_data_size;
    } else {
        total_send_size_ = _task_profile.transfer_profile.send_data_size;
        total_revc_size_ = _task_profile.transfer_profile.receive_data_size;
    }
    xinfo2(TSF "[dual-channel] total send size:%_ total revc size:%_", total_send_size_, total_revc_size_);
    if (IsOverSize()) {
        xinfo2(TSF "[dual-channel] over size.");
    } else {
        __SaveData();
    }
}

bool CellularNetworkLogic::IsOverSize() {
    if (total_send_size_ + total_revc_size_ >= max_trafice_size) {
        xinfo2(TSF "[dual-chaneel] over size.");
        return true;
    }
    return false;
}

void CellularNetworkLogic::__Init() {
    xdebug_function();
    std::lock_guard<std::mutex> lock(mutex_);
    cellular_network_ini_.Parse();

    if (!cellular_network_ini_.Select(kINIDefaultSelect)) {
        cellular_network_ini_.Create(kINIDefaultSelect);
    }

    recored_date_ = cellular_network_ini_.Get(kINIKeyRecoredDate, "");
    //max_send_size_ = cellular_network_ini_.Get(kINIKeyMaxSendSize, CELLULAR_NETWORK_MAX_SEND_SIZE);
    //max_revc_size_ = cellular_network_ini_.Get(kINIKeyMaxRevcSize, CELLULAR_NETWORK_MAX_REVC_SIZE);
    max_trafice_size = cellular_network_ini_.Get(kINIKeyMaxTrafficSize, CELLULAR_NETWORK_MAX_TRAFFIC_MB * 1024 * 1024);
    
    if (__CheckDate()) {
        total_send_size_ = cellular_network_ini_.Get(kINIKeyTotalSendSize, 0);
        total_revc_size_ = cellular_network_ini_.Get(kINIKeyTotalRevcSize, 0);
    } else {
        //__SaveData(); //会死锁,不要在这里保存
    }
    is_init_ = true;
    
    xinfo2(TSF "[dual-channel] max traffic size:%_ send_size:%_ revc_size:%_",
           max_trafice_size,
           total_send_size_,
           total_revc_size_);
}

std::string CellularNetworkLogic::__GetCurrentDate() {
    time_t sec = std::time(nullptr);
    tm tcur = *localtime((const time_t*)&sec);
    char temp[64] = {0};
    snprintf(temp, 64, "%d%02d%02d", 1900 + tcur.tm_year, 1 + tcur.tm_mon, tcur.tm_mday);
    return temp;
}

bool CellularNetworkLogic::__CheckDate() {
    std::string current_date = __GetCurrentDate();
    if (current_date == recored_date_) {
        return true;
    } else {
        xinfo2(TSF "[dual-channel] date changed. reset data. current:%_ last_recored:%_", current_date, recored_date_);
        recored_date_ = current_date;
        total_send_size_ = 0;
        total_revc_size_ = 0;
        return false;
    }
}

void CellularNetworkLogic::__SaveData() {
    xinfo_function();
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_init_) {
        xinfo2(TSF "[dual-channle] logic is no init.");
        return;
    }
    cellular_network_ini_.Set(kINIKeyRecoredDate, recored_date_);
    cellular_network_ini_.Set(kINIKeyTotalSendSize, total_send_size_);
    cellular_network_ini_.Set(kINIKeyTotalRevcSize, total_revc_size_);
    cellular_network_ini_.Save();
}

}  // namespace stn
}  // namespace mars
