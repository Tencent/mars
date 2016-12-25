/*
 * sdt_logic.cc
 *
 *  Created on: 2016年3月18日
 *      Author: caoshaokun
 */

#include "mars/sdt/sdt_logic.h"

#include "mars/baseevent/baseevent.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"
#include "mars/sdt/constants.h"

#include "sdt_core.h"

namespace mars {
namespace sdt {

static Callback* sg_callback = NULL;

static const std::string kLibName = "sdt";

#define SDT_WEAK_CALL(func) \
    boost::shared_ptr<SdtCore> sdt_ptr = SdtCore::Singleton::Instance_Weak().lock();\
    if (!sdt_ptr) {\
        xwarn2(TSF"sdt uncreate");\
        return;\
    }\
	sdt_ptr->func

static void onCreate() {
    xinfo2(TSF"sdt oncreate");
    SdtCore::Singleton::Instance();
}

static void onDestroy() {
    xinfo2(TSF"sdt onDestroy");
    SdtCore::Singleton::Release();
}

static void __initbind_baseprjevent() {

#ifdef ANDROID
	mars::baseevent::addLoadModule(kLibName);
#endif
	GetSignalOnCreate().connect(&onCreate);
	GetSignalOnDestroy().connect(&onDestroy);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);

//active netcheck interface
void StartActiveCheck(CheckIPPorts& _longlink_check_items, CheckIPPorts& _shortlink_check_items, int _mode, int _timeout) {
	SDT_WEAK_CALL(StartCheck(_longlink_check_items, _shortlink_check_items, _mode, _timeout));
}

void CancelActiveCheck() {
	SDT_WEAK_CALL(CancelCheck());
}

void SetCallBack(Callback* const callback) {
	sg_callback = callback;
}

#ifndef ANDROID

WEAK_FUNC void ReportNetCheckResult(std::vector<CheckResultProfile>& _check_results) {

}

#endif

}}
