//
// Created by Changpeng Pan on 2022/9/9.
//

#include "context.h"

#include "mars/app/app_manager.h"
#include "mars/comm/comm_util.h"
#include "mars/stn/stn_manager.h"

#define SCOPED_LOCK() std::unique_lock<std::recursive_mutex> lock(mutex_)
#define SCOPED_UNLOCK() lock.unlock()

#define S_SCOPED_LOCK() std::unique_lock<std::recursive_mutex> s_lock(s_mutex_)
#define S_SCOPED_UNLOCK() s_lock.unlock()

static int context_instance_counter_ = 0;

namespace mars {
namespace boot {

std::map<std::string, Context*> Context::s_context_map_;
std::recursive_mutex Context::s_mutex_;

Context::Context() {
    context_instance_counter_++;
//    app_manager_ = new mars::app::AppManager(this);
//    stn_manager_ = new mars::stn::StnManager(this);
}

Context::~Context() {
    UnInit();
    safe_delete(app_manager_);
    safe_delete(stn_manager_);
    context_instance_counter_--;
}

int Context::Init() {
    SCOPED_LOCK();
    if (!is_init_) {
        // do somethings
        is_init_ = true;
    }
    return 0;
}

int Context::UnInit() {
    SCOPED_LOCK();
    if (is_init_) {
        // do somethings
        is_init_ = false;
    } else {
        return -1;
    }
    return 0;
}

void Context::SetContextId(const std::string& context_id) {
    std::call_once(set_context_id_flag_, [this, context_id] {
        context_id_ = context_id;
    });
}

const std::string& Context::GetContextId() {
    return context_id_;
}

BaseAppManager* Context::GetAppManager() {
    return app_manager_;
}

BaseStnManager* Context::GetStnManager() {
    return stn_manager_;
}


Context* Context::CreateContext(const std::string& context_id) {
    S_SCOPED_LOCK();
    if (!context_id.empty()) {
        if (s_context_map_.find(context_id) != s_context_map_.end()) {
            return s_context_map_[context_id];
        } else {
            auto context = new Context();
            context->SetContextId(context_id);
            s_context_map_[context_id] = context;
            return context;
        }
    } else {
        auto context = new Context();
        auto tmp_context_id = std::to_string(reinterpret_cast<uint64_t>(context));
        context->SetContextId(tmp_context_id);
        s_context_map_[tmp_context_id] = context;
        return context;
    }
}

void Context::DeleteContext(BaseContext* context) {
    S_SCOPED_LOCK();
    if (context != nullptr) {
        auto* temp_context = dynamic_cast<Context*>(context);
        auto context_id = temp_context->GetContextId();
        if (s_context_map_.find(context_id) != s_context_map_.end()) {
            s_context_map_.erase(context_id);
        }
        delete temp_context;
        context = nullptr;
    }
}


 MARS_API BaseContext *CreateContext(const std::string &context_id) {
    return Context::CreateContext(context_id);
}

 MARS_API void DestroyContext(BaseContext *context) {
    Context::DeleteContext(context);
}

//BaseContext* (*CreateContext)(const std::string& context_id) = [](const std::string& context_id) -> BaseContext* {
//    return Context::CreateContext(context_id);
//};
//
//void (*DestroyContext)(BaseContext* context) = [](BaseContext* context) {
//    Context::DeleteContext(context);
//};

//BaseContext* CreateContext(const std::string& context_id) {
//    return Context::CreateContext(context_id);
//}
//
//void DestroyContext(BaseContext* context) {
//    Context::DeleteContext(context);
//}

//mars::boot::CreateContext = MMCreateContext;
//
//mars::boot::DestroyContext = MMDestroyContext;

}  // namespace boot
}  // namespace mars
