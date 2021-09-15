#ifndef STN_SRC_SHORTLINK_EMPTY_IMPL_H_
#define STN_SRC_SHORTLINK_EMPTY_IMPL_H_

#include "shortlink_interface.h"

#include <string>
#include <thread>
#include <functional>
#include <future>
#include <memory>
#include <mutex>

#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"

#include "mars/stn/stn.h"
#include "mars/stn/shortlink_wrapper.h"

namespace mars {

namespace stn {

class ThreadWrapper {
public:
    ThreadWrapper(std::function<void()> _fun) 
    : function_(std::move(_fun)) {
        running_ = done_.get_future();
    }

    ~ThreadWrapper() {}

public:
    bool Start() {
        if (!function_) {
            xerror2(TSF"no run implments");
            return false;
        }
        runner_ = std::thread(&ThreadWrapper::_Run, this);
        return true;
    }

    void Join() {
        runner_.join();
    }
    void Detach() {
        runner_.detach();
    }

    bool IsRunning() {
        std::unique_lock<std::mutex> lock(mutex_);
        auto ret = running_.wait_for(std::chrono::milliseconds(0));
        return !(ret == std::future_status::ready);
    }
private:
    void _Run() {
        function_();
        std::unique_lock<std::mutex> lock(mutex_);
        done_.set_value(true);
        xinfo2(TSF"shortlink run end");
    }

private:
    std::function<void()> function_;
    std::thread runner_;
    std::promise<bool> done_;
    std::future<bool> running_;
    std::mutex mutex_;
};


class ShortlinkEmptyImpl : public ShortLinkInterface {
public:
    ShortlinkEmptyImpl(const Task& _task, std::shared_ptr<ShortlinkCallbackBridge> _callback_bridge);
    ~ShortlinkEmptyImpl();
public:
    void OnReceiveDataFromCaller(const std::string& _data, int _status_code) override;
    void OnReceiveErrorFromCaller(ErrCmdType _error_type, int _error_code) override;
protected:
    void SendRequest(AutoBuffer& _buffer_req, AutoBuffer& _buffer_extend) override;

private:
    void _StartInternal();
    void _OnResponse(const std::string& data, int _status_code);
    void _OnResponseError(ErrCmdType _error_type, int _error_code);

private:
    AutoBuffer  send_body_;
    AutoBuffer  send_extend_;
    Task task_;
    ThreadWrapper runner_;
    std::weak_ptr<ShortlinkCallbackBridge> shortlink_callback_bridge_;
    ShortLinkWrapper wrapper_;

};

}} //namespace mars stn

#endif //SHORTLINK_EMPTY_IMPL_H_
