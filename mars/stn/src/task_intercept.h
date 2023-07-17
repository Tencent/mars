#ifndef STN_SRC_TASK_INTERCEPT_H_
#define STN_SRC_TASK_INTERCEPT_H_

#include <map>
#include <string>

struct TaskInterceptInfo {
    std::string name = "";
    uint64_t intercept_time = 0;
    std::string data = "";
};

class TaskIntercept {
 public:
    TaskIntercept() = default;
    ~TaskIntercept() = default;

 public:
    void AddInterceptTask(const std::string& _name, const std::string& _data);
    bool GetInterceptTaskInfo(const std::string& _name, std::string& _last_data);

 private:
    std::map<std::string, TaskInterceptInfo> intercept_tasks_;
};

#endif  // MARS_TASK_INTERCEPT_H_