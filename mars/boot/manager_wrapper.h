//
// Created by Changpeng Pan on 2022/11/29.
//

#ifndef MMNET_MANAGER_WRAPPER_H
#define MMNET_MANAGER_WRAPPER_H
namespace mars {
namespace boot {
template <typename T>
class ManagerWrapper {
    T* mObject;

 public:
    template <typename... ARGS>
    explicit ManagerWrapper(ARGS... a) {
        mObject = new T(a...);
    }

    explicit ManagerWrapper(T* obj) {
        mObject = obj;
    }

    virtual ~ManagerWrapper() {
    }

    T* get() const {
        return mObject;
    }
};

}  // namespace boot
}  // namespace mars

#endif  // MMNET_MANAGER_WRAPPER_H
