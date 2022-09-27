//
// Created by Changpeng Pan on 2022/9/9.
//

#ifndef MMNET_CONTEXT_H
#define MMNET_CONTEXT_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "mars/boot/base_context.h"

namespace mars {

namespace boot {
class Context : public BaseContext {
 public:
    explicit Context();
    ~Context() override;

 public:
    int Init() override;
    int UnInit() override;
    void SetContextId(const std::string& context_id) override;
    const std::string& GetContextId() override;
    BaseAppManager* GetAppManager() override;
    BaseStnManager* GetStnManager() override;

 public:
    BaseAppManager* appManager() {
        return app_manager_;
    }
    BaseStnManager* stnManager() {
        return stn_manager_;
    }

 public:
    static Context* CreateContext(const std::string& context_id);
    static void DeleteContext(BaseContext* context);

 private:
    static std::map<std::string, Context*> s_context_map_;
    static std::recursive_mutex s_mutex_;

 private:
    bool is_init_ = false;
    std::string context_id_;
    std::once_flag set_context_id_flag_;
    std::recursive_mutex mutex_;
    BaseAppManager* app_manager_ = nullptr;
    BaseStnManager* stn_manager_ = nullptr;
};

}  // namespace boot
}  // namespace mars

#endif  // MMNET_CONTEXT_H
