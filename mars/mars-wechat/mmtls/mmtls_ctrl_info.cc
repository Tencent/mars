/*
 * MmtlsCtrlInfoLogic.cpp
 *
 *  Created on: 2016年3月25日
 *      Author: elviswu
 */

#include "mmtls_ctrl_info.h"

#include <sys/stat.h>

#include <string>

#include "mars/app/app.h"
#include "mars/comm/singleton.h"
#include "mars/comm/unix/thread/lock.h"
#include "mars/comm/unix/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_lib/client/mmtls_client_credential_manager.h"

using namespace mars::stn;

static const char* kMmtlsCtrlInfoSection = "SEC_CTRL_INFO";
static const char* kMmtlsEnableFlag = "KEY_XXX_ENABLE";
// psk type
static const uint8_t PSK_NONE = 0;
static const uint8_t PSK_ACCESS = 1;
static const uint8_t PSK_REFRESH = 2;
static const uint8_t PSK_MAX = 15;

#define MMTLS_CTRL_INFO_PATH (mars::app::GetAppFilePath() + "/host")

MMTLSCtrlInfo::MMTLSCtrlInfo(mars::boot::Context* _context, bool _use_mmtls, const std::string& _filepath)
: context_(_context), use_mmtls(_use_mmtls), ini(_filepath + "/mmtls_ctrl.ini") {
    ini.Parse();

    if (ini.Select(kMmtlsCtrlInfoSection)) {
        bool mmtls_enable = ini.Get<bool>(kMmtlsEnableFlag, true);  // default true
        use_mmtls = mmtls_enable;
    }

    xinfo2(TSF " MmtlsCtrlInfo.use_mmtls=%_", use_mmtls);
}

MMTLSCtrlInfo::~MMTLSCtrlInfo() {
}

void MMTLSCtrlInfo::Save(bool _use_mmtls) {
    use_mmtls = _use_mmtls;
    bool ret = ini.Select(kMmtlsCtrlInfoSection);
    if (!ret)
        ini.Create(kMmtlsCtrlInfoSection);

    ini.Set<bool>(kMmtlsEnableFlag, use_mmtls);
    ini.Save();
}

bool MMTLSCtrlInfo::IsMMTLSEnabled() {
    return use_mmtls;
}

void MMTLSCtrlInfo::ClearAllMMtlsPsk() {
    xdebug2(TSF "mars2 ClearAllMMtlsPsk");
    mmtls::ClientCredentialManager::GetInstance(context_, "default").DeletePsk(PSK_ACCESS);
    // mmtls::ClientCredentialManager::GetInstance("default").DeletePsk(PSK_REFRESH); //no need delete refresh psk
}
