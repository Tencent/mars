//
// Created by Changpeng Pan on 2024/8/20.
//

#ifndef MMNET_BASE_EVENT_H
#define MMNET_BASE_EVENT_H
#include "boot.h"

namespace mars {
namespace boot {

class BaseEvent {
 public:
    virtual void OnCreate(SetupConfig _config){};
    virtual void OnDestroy(){};
    virtual void OnSignalCrash(int _sig){};
    virtual void OnExceptionCrash(){};
    virtual void OnForeground(bool _foreground){};
    virtual void OnNetworkChange(){};
    virtual void OnNetworkDataChange(const char* _tag, int32_t _send, int32_t _recv){};
#ifdef ANDROID
    virtual void OnAlarm(int64_t _id){};
#endif
};

}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_EVENT_H
