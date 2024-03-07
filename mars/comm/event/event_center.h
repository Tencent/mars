//
// Created by Changpeng Pan on 2024/3/4.
//

#ifndef MMNET_EVENT_CENTER_H
#define MMNET_EVENT_CENTER_H

#include <mars/xlog/xlogger.h>

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace mars {
namespace event {

class IEvent {
 public:
    IEvent(const std::string& _event_type, bool _consumed = false) : event_type_(_event_type), consumed_(_consumed) {
    }

    virtual ~IEvent() = default;

    const std::string& getType() const {
        return event_type_;
    }

    bool isConsumed() const {
        return consumed_;
    }

    void consume() {
        consumed_ = true;
    }

 protected:
    std::string event_type_;
    bool consumed_ = false;
};

class IListener {
 public:
    virtual void handleEvent(IEvent& event) = 0;

    int getPriority() const {
        return priority;
    }

 protected:
    int priority = 0;  // 默认优先级为0
};

class EventCenter {
 public:
    EventCenter(const EventCenter&) = delete;
    EventCenter& operator=(const EventCenter&) = delete;

    static EventCenter& getInstance() {
        static EventCenter instance;  // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

 private:
    EventCenter() {
        xdebug_function();
    }
    ~EventCenter() {
        xdebug_function();
    }

 public:
    void registerListener(IListener* listener) {
        std::lock_guard<std::mutex> lock(mutex);
        listeners.push_back(listener);
        std::sort(listeners.begin(), listeners.end(), [](const IListener* a, const IListener* b) {
            return a->getPriority() > b->getPriority();  // 优先级高的先处理
        });
    }

    void unregisterListener(IListener* listener) {
        std::lock_guard<std::mutex> lock(mutex);
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    bool publish(IEvent* event) {
        std::lock_guard<std::mutex> lock(mutex);
        for (IListener* listener : listeners) {
            listener->handleEvent(*event);
            if (event->isConsumed()) {
                return true;
            }
        }
        return false;
    }

 private:
    std::mutex mutex;
    std::vector<IListener*> listeners;
};

}  // namespace event
}  // namespace mars

#endif  // MMNET_EVENT_CENTER_H
