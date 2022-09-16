//
// Created by Changpeng Pan on 2022/7/22.
//

#ifndef MMNET_SDT_MANAGER_H
#define MMNET_SDT_MANAGER_H

#include "sdt.h"
#include "sdt_core.h"
#include "sdt_logic.h"
#include <mutex>
#include <string>
#include <vector>
#include <map>

namespace mars {
namespace sdt {
class SdtManager {

public:
    explicit SdtManager(const std::string& context_id);
    virtual ~SdtManager();
    void Init();
    void UnInit();

    /** transition logic    */
 private:
    std::string context_id_;

 public:
    static SdtManager* CreateSdtManager(const std::string& context_id);
    static void DestroySdtManager(SdtManager* manager);

 private:
    static std::map<std::string, SdtManager*> s_sdt_manager_map_;
    static std::recursive_mutex s_mutex_;
    /** transition logic  */

public:
    void OnCreate();
    void OnDestroy();

public:
    void SetCallBack(Callback* const callback);
    void SetHttpNetcheckCGI(std::string cgi);
    void StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout);
    void CancelActiveCheck();
    void ReportNetCheckResult(const std::vector<CheckResultProfile>& _check_results);

private:
    Callback* callback_;
    SdtCore* sdt_core_;
};

}
}



#endif //MMNET_SDT_MANAGER_H
