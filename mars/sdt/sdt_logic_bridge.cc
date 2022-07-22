//
// Created by Changpeng Pan on 2022/7/22.
//

#include "sdt_logic_bridge.h"

#include "mars/baseevent/baseevent.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"

namespace mars {
namespace sdt {

//sdt logic
void SetCallBack(Callback* const callback) {
    xassert2(sdt_manager_ != NULL);
    sdt_manager_->SetCallBack(callback);
}

//void SetHttpNetcheckCGI(std::string cgi) {
//    xassert2(sdt_manager_ != NULL);
//    sdt_manager_->SetHttpNetcheckCGI(cgi);
//}

void StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout) {
    xassert2(sdt_manager_ != NULL);
    sdt_manager_->StartActiveCheck(_longlink_check_item, _shortlink_check_item, _mode, _timeout);
}

void CancelActiveCheck() {
    xassert2(sdt_manager_ != NULL);
    sdt_manager_->CancelActiveCheck();
}
//end sdt logic

//sdt.h
#if !defined(ANDROID) || defined (CPP_CALL_BACK)
void (*ReportNetCheckResult)(const std::vector<CheckResultProfile>& _check_results)
= [](const std::vector<CheckResultProfile>& _check_results) {
        xassert2(sdt_manager_ != NULL);
        sdt_manager_->ReportNetCheckResult(_check_results);
};
#endif
//end std.h

static void onCreate() {
    xinfo2(TSF"sdt oncreate");
    sdt_manager_ = new SdtManager();
    sdt_manager_->OnCreate();
}

static void onDestroy() {
    xinfo2(TSF"sdt onDestroy");
    sdt_manager_->OnDestroy();
    delete sdt_manager_;
}



static void __initbind_baseprjevent() {
    GetSignalOnCreate().connect(&onCreate);
    GetSignalOnDestroy().connect(5, &onDestroy);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);

}
}
