//
// Created by Cpan on 2022/9/16.
//

#ifndef MMNET_BASE_MANAGER_H
#define MMNET_BASE_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace mars {
namespace boot {

class BaseManager {
 public:
    virtual ~BaseManager() {
    }
    virtual std::string GetName() = 0;

    virtual void OnCreate(){};
    virtual void OnDestroy(){};
    virtual void OnSingalCrash(int _sig){};
    virtual void OnExceptionCrash(){};
    virtual void OnForeground(bool _isforeground){};
    virtual void OnNetworkChange(void (*pre_change)()){};
    virtual void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv){};
#ifdef ANDROID
    virtual void OnAlarm(int64_t _id);
#endif
};
}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_MANAGER_H
