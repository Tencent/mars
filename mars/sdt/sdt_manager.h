//
// Created by Changpeng Pan on 2022/7/22.
//

#ifndef MMNET_SDT_MANAGER_H
#define MMNET_SDT_MANAGER_H

#include "sdt.h"
#include "sdt_core.h"
#include "sdt_logic.h"

namespace mars {
namespace sdt {
class SdtManager {

public:
    SdtManager();
    virtual ~SdtManager();

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
