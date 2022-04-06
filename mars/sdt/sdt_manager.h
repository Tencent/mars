//
// Created by Cpan on 2022/4/6.
//

#ifndef MMNET_SDT_MANAGER_H
#define MMNET_SDT_MANAGER_H

#include <vector>
#include "mars/sdt/sdt.h"
#include "sdt/src/sdt_core.h"

namespace mars {
namespace sdt {

class StnManagerCallback {
public:
    ~StnManagerCallback();
};

class SdtManager : public StnManagerCallback {
public:
    SdtManager();
    ~SdtManager();

    void SetCallBack(StnManagerCallback* const callback);

    void SetHttpNetCheckCGI(std::string cgi);

    //active netcheck interface
    void StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout);
    void CancelActiveCheck();

private:
    StnManagerCallback* callback_;
    SdtCore* sdt_core_;
};

}
}
#endif //MMNET_SDT_MANAGER_H
