#include "task_intercept.h"

#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"

void TaskIntercept::AddInterceptTask(const std::string& _name, const std::string& _data) {
    xinfo2(TSF "save task name %_", _name);
    if (_name.empty()) {
        return;
    }
    TaskInterceptInfo info;
    info.name = _name;
    info.data = _data;
    info.intercept_time = ::gettickcount();
    intercept_tasks_[_name] = info;
}

bool TaskIntercept::GetInterceptTaskInfo(const std::string& _name, std::string& _last_data) {
    auto info = intercept_tasks_.find(_name);
    if (info == intercept_tasks_.end()) {
        return false;
    }
    auto current_time = ::gettickcount();
    xinfo2(TSF "find intercepted task: %_, time: %_, current: %_, data size %_",
           _name,
           info->second.intercept_time,
           current_time,
           info->second.data.size());
    xdebug2(TSF "intercepted last delte: %_", (current_time - info->second.intercept_time));
    if (current_time - info->second.intercept_time > 60 * 1000) {
        intercept_tasks_.erase(info);
        return false;
    }
    _last_data = info->second.data;
    return false;
}