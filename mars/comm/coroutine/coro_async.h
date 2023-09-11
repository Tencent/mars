/*
 * coro_async.h
 *
 *  Created on: 2016-12-28
 *      Author: yerungui
 */

#ifndef CORO_ASYNC_H_
#define CORO_ASYNC_H_

#include "coroutine.h"
#include "mars/boost/intrusive_ptr.hpp"
#include "mars/comm/thread/thread.h"

namespace coroutine {

/////////////////running in coroutine utils///////////////////////////////
class WaitThread {
 private:
    struct Wrapper_ {
        mars::comm::Mutex mutex;
        mars::comm::mq::MessagePost_t message_timeout;
        mars::comm::mq::MessagePost_t message_running;
        boost::intrusive_ptr<Wrapper> wrapper;
        int status;
    };

 public:
    enum {
        kTimeout = 0,
        kFinish,
        kCancel,
    };

 public:
    WaitThread(const char* _name = NULL) : thread_(_name), wrapper_(new Wrapper_) {
        wrapper_->status = kTimeout;
    }
    ~WaitThread() {
    }

    template <typename F>
    typename boost::disable_if<typename boost::is_void<typename boost::result_of<F()>::type>,
                               typename boost::result_of<F()>::type>::type
    operator()(const F& _block_func, int64_t _timeout = -1, int* _status = NULL) {
        std::shared_ptr<Wrapper_> wrapper = wrapper_;
        mars::comm::ScopedLock lock(wrapper_->mutex);
        wrapper->wrapper = RunningCoroutine();

        if (0 <= _timeout) {
            wrapper->message_timeout = Resume(wrapper->wrapper, _timeout);
        }

        typedef typename boost::result_of<F()>::type R;
        std::shared_ptr<R> result(new R);

        mars::comm::mq::AsyncResult<R> async_result(
            _block_func,
            [wrapper, result](const R& _result, bool _valid) {
                ASSERT(_valid);

                mars::comm::ScopedLock lock(wrapper->mutex);
                *result = _result;
                if (!wrapper->wrapper)
                    return;
                if (mars::comm::mq::KNullPost != wrapper->message_running) {
                    return;
                }
                if (mars::comm::mq::KNullPost == wrapper->message_timeout
                    || mars::comm::mq::CancelMessage(wrapper->message_timeout)) {
                    wrapper->status = kFinish;
                    wrapper->message_running = Resume(wrapper->wrapper);
                    return;
                }
            },
            NULL);

        thread_.start(async_result);
        lock.unlock();
        Yield();

        lock.lock();
        if (_status)
            *_status = wrapper->status;
        return *result;
    }

    template <typename F>
    typename boost::enable_if<typename boost::is_void<typename boost::result_of<F()>::type>>::type
    operator()(const F& _block_func, int64_t _timeout = -1, int* _status = NULL) {
        std::shared_ptr<Wrapper_> wrapper = wrapper_;
        mars::comm::ScopedLock lock(wrapper_->mutex);
        wrapper->wrapper = RunningCoroutine();

        if (0 <= _timeout) {
            wrapper->message_timeout = Resume(wrapper->wrapper, _timeout);
        }

        mars::comm::mq::AsyncResult<void> async_result(_block_func, [wrapper](bool _valid) {
            ASSERT(_valid);

            mars::comm::ScopedLock lock(wrapper->mutex);
            if (!wrapper->wrapper)
                return;
            if (mars::comm::mq::KNullPost != wrapper->message_running) {
                return;
            }
            if (mars::comm::mq::KNullPost == wrapper->message_timeout
                || mars::comm::mq::CancelMessage(wrapper->message_timeout)) {
                wrapper->status = kFinish;
                wrapper->message_running = Resume(wrapper->wrapper);
                return;
            }
        });

        thread_.start(async_result);
        lock.unlock();
        Yield();

        if (_status)
            *_status = wrapper->status;
        return;
    }

    void Cancel() const {
        mars::comm::ScopedLock lock(wrapper_->mutex);
        if (!wrapper_->wrapper)
            return;
        if (mars::comm::mq::KNullPost != wrapper_->message_running) {
            return;
        }
        if (mars::comm::mq::KNullPost == wrapper_->message_timeout
            || mars::comm::mq::CancelMessage(wrapper_->message_timeout)) {
            wrapper_->status = kCancel;
            wrapper_->message_running = Resume(wrapper_->wrapper);
            return;
        }
    }

    const boost::intrusive_ptr<Wrapper>& wrapper() const {
        return wrapper_->wrapper;
    }

 private:
    WaitThread(const WaitThread&);
    void operator=(const WaitThread&);

 private:
    mars::comm::Thread thread_;
    std::shared_ptr<Wrapper_> wrapper_;
};

template <typename F>
typename boost::result_of<F()>::type MessageInvoke(const F& _func) {
    boost::intrusive_ptr<Wrapper> wrapper = RunningCoroutine();

    typedef typename boost::result_of<F()>::type R;
    mars::comm::mq::AsyncResult<R> result([_func, wrapper]() {
        Resume(wrapper);
        return _func();
    });

    mars::comm::mq::AsyncInvoke(result, mars::comm::mq::Post2Handler(mars::comm::mq::RunningMessageID()));
    Yield();
    return result.Result();
}

}  // namespace coroutine

#endif
