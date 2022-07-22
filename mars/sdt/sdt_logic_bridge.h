//
// Created by Changpeng Pan on 2022/7/22.
//

#ifndef MMNET_SDT_LOGIC_BRIDGE_H
#define MMNET_SDT_LOGIC_BRIDGE_H

#include "mars/sdt/sdt_manager.h"

namespace mars {
namespace sdt {
 static SdtManager* sdt_manager_ = NULL;

 extern void SetCallBack(Callback* const callback);
 //extern void SetHttpNetcheckCGI(std::string cgi);
 //active netcheck interface
 extern void StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout);
 extern void CancelActiveCheck();

 //sdt.h
 extern void (*ReportNetCheckResult)(const std::vector<CheckResultProfile>& _check_results);

}
}


#endif //MMNET_SDT_LOGIC_BRIDGE_H
