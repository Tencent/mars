//
// Created by Changpeng Pan on 2022/9/13.
//

#ifndef MMNET_BASE_CONTEXT_H
#define MMNET_BASE_CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "mars/app/base_app_manager.h"
#include "mars/stn/base_stn_manager.h"

using namespace mars::stn;

namespace mars {
namespace app {

class BaseContext {
 public:
    virtual ~BaseContext() {
    }

 public:
    virtual int Init() = 0;
    virtual int UnInit() = 0;
//    virtual int SetContextId(std::string& context_id) = 0;
//    virtual std::string GetContextId() = 0;
    virtual BaseAppManager* GetAppManager() = 0;
    virtual BaseStnManager* GetStnManager() = 0;
};

#ifdef _WIN32
#if defined(NEWT_EXPORTS)
#define MARS_API extern "C" __declspec(dllexport)
#elif defined(NEWT_IMPORTS)
#define MARS_API extern "C" __declspec(dllimport)
#else
#define MARS_API extern "C"
#endif
#elif defined(__APPLE__) || defined(__ANDROID__) || defined(__linux__)
#define MARS_API extern "C" __attribute__((visibility("default")))
#else
#define MARS_API extern "C"
#endif

MARS_API BaseContext* CreateContext(const std::string& context_id);
MARS_API void DestroyContext(BaseContext* context);

}  // namespace app
}  // namespace mars

#endif  // MMNET_BASE_CONTEXT_H
