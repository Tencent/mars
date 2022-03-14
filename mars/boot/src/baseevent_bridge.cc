//
// Created by Cpan on 2022/3/10.
//

#include "mars/boot/baseevent_bridge.h"
#include "mars/baseevent/baseevent.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/boost/signals2.hpp"
#include "mars/boot/mars.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/platform_comm.h"

using namespace mars::comm;

namespace mars {
namespace boot {

static Mars *mars = NULL;

static const std::string kLibName = "sdt";

static void onCreate() {
  xinfo2(TSF "mars oncreate");
  if (mars == nullptr) {
    mars = new Mars();
  }

  mars->OnCreate("default");
}

static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
  mars = new Mars();
  mars->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

static void onDestroy() {
  mars->OnDestroy();
  delete mars;
  mars = nullptr;
}

static void onSingalCrash(int _sig) { mars->OnSignalCrash(_sig); }

static void onExceptionCrash() { mars->OnExceptionCrash(); }

static void onNetworkChange() { mars->OnNetworkChange(); }

static void OnNetworkDataChange(const char *_tag, ssize_t _send,
                                ssize_t _recv) {
  mars->onNetworkDataChange(_tag, _send, _recv);
}

#ifdef ANDROID
// must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) { Alarm::onAlarmImpl(_id); }
#endif

static void __initbind_baseprjevent() {
#ifdef WIN32
  boost::filesystem::path::imbue(std::locale(
      std::locale(), new boost::filesystem::detail::utf8_codecvt_facet));
#endif

#ifdef ANDROID
  mars::baseevent::addLoadModule(kLibName);
  GetSignalOnAlarm().connect(&onAlarm);
#endif
  GetSignalOnCreate().connect(&onCreate);
  GetSignalOnInitBeforeOnCreate().connect(
      boost::bind(&onInitConfigBeforeOnCreate, _1));
  GetSignalOnDestroy().connect(&onDestroy); // low priority signal func
  GetSignalOnSingalCrash().connect(&onSingalCrash);
  GetSignalOnExceptionCrash().connect(&onExceptionCrash);
  GetSignalOnNetworkChange().connect(5, &onNetworkChange); // define group 5

#ifndef XLOGGER_TAG
#error "not define XLOGGER_TAG"
#endif

  GetSignalOnNetworkDataChange().connect(&OnNetworkDataChange);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);

void BaseEventBridge::HelloWorld() { xinfo2(TSF "Hello Mars"); }

void xxxx() { xinfo2(TSF "Hello Mars"); }
} // namespace boot
} // namespace mars
