// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COMM_SINGLETON_H_
#define COMM_SINGLETON_H_

#include <list>
#include <thread>
#include <memory>

#include "boost/signals2.hpp"
#include "mars/comm/macro.h"
#include "mars/comm/thread/lock.h"

#define SINGLETON_STRONG(class_name)                                          \
    design_patterns::Singleton::Instance<class_name>(                         \
        design_patterns::SingletonHelper::CreateInstanceHelper<class_name>(), \
        design_patterns::SingletonHelper::DestoryInstanceHelper<class_name>())
#define SINGLETON_WEAK(class_name) design_patterns::Singleton::Instance_Weak<class_name>()

#define SINGLETON_RELEASE(class_name) design_patterns::Singleton::Release<class_name>()
#define SINGLETON_RELEASE_ALL() design_patterns::Singleton::ReleaseAll()

#define SINGLETON_INTRUSIVE(classname, creater, destoryer)                                                \
    class Singleton {                                                                                     \
     public:                                                                                              \
        static std::shared_ptr<classname>& instance_shared_ptr() {                                        \
            NO_DESTROY static std::shared_ptr<classname> s_ptr;                                           \
            return s_ptr;                                                                                 \
        }                                                                                                 \
        static mars::comm::Mutex& singleton_mutex() {                                                     \
            NO_DESTROY static mars::comm::Mutex s_mutex;                                                  \
            return s_mutex;                                                                               \
        }                                                                                                 \
        static boost::signals2::signal<void()>& SignalInstanceBegin() {                                   \
            NO_DESTROY static boost::signals2::signal<void()> s_signal;                                   \
            return s_signal;                                                                              \
        }                                                                                                 \
        static boost::signals2::signal<void(std::shared_ptr<classname>)>& SignalInstance() {              \
            NO_DESTROY static boost::signals2::signal<void(std::shared_ptr<classname>)> s_signal;         \
            return s_signal;                                                                              \
        }                                                                                                 \
        static boost::signals2::signal<void(std::shared_ptr<classname>)>& SignalRelease() {               \
            NO_DESTROY static boost::signals2::signal<void(std::shared_ptr<classname>)> s_signal;         \
            return s_signal;                                                                              \
        }                                                                                                 \
        static boost::signals2::signal<void()>& SignalReleaseEnd() {                                      \
            NO_DESTROY static boost::signals2::signal<void()> s_signal;                                   \
            return s_signal;                                                                              \
        }                                                                                                 \
        static boost::signals2::signal<void(classname&)>& SignalResetOld() {                              \
            NO_DESTROY static boost::signals2::signal<void(classname&)> s_signal;                         \
            return s_signal;                                                                              \
        }                                                                                                 \
        static boost::signals2::signal<void(classname&)>& SignalResetNew() {                              \
            NO_DESTROY static boost::signals2::signal<void(classname&)> s_signal;                         \
            return s_signal;                                                                              \
        }                                                                                                 \
                                                                                                          \
     public:                                                                                              \
        static std::shared_ptr<classname> Instance() {                                                    \
            std::shared_ptr<classname> ret = instance_shared_ptr();                                       \
            if (ret)                                                                                      \
                return ret;                                                                               \
                                                                                                          \
            mars::comm::ScopedLock lock(singleton_mutex());                                               \
            if (!instance_shared_ptr()) {                                                                 \
                SignalInstanceBegin()();                                                                  \
                std::shared_ptr<classname> temp(const_cast<classname*>(creater), Singleton::Delete);      \
                SignalInstance()(temp);                                                                   \
                instance_shared_ptr().swap(temp);                                                         \
            }                                                                                             \
            return (instance_shared_ptr());                                                               \
        }                                                                                                 \
                                                                                                          \
        template <class T>                                                                                \
        static std::shared_ptr<classname> Instance(const T& _creater) {                                   \
            std::shared_ptr<classname> ret = instance_shared_ptr();                                       \
            if (ret)                                                                                      \
                return ret;                                                                               \
                                                                                                          \
            mars::comm::ScopedLock lock(singleton_mutex());                                               \
            if (!instance_shared_ptr()) {                                                                 \
                SignalInstanceBegin()();                                                                  \
                std::shared_ptr<classname> temp(const_cast<classname*>(_creater()), Singleton::Delete);   \
                SignalInstance()(temp);                                                                   \
                instance_shared_ptr().swap(temp);                                                         \
            }                                                                                             \
            return (instance_shared_ptr());                                                               \
        }                                                                                                 \
                                                                                                          \
        static std::weak_ptr<classname> Instance_Weak() {                                               \
            return instance_shared_ptr();                                                                 \
        }                                                                                                 \
                                                                                                          \
        static void Release() {                                                                           \
            mars::comm::ScopedLock lock(singleton_mutex());                                               \
            if (instance_shared_ptr()) {                                                                  \
                SignalRelease()(instance_shared_ptr());                                                   \
                instance_shared_ptr().reset();                                                            \
                int waitCnt = 0;                                                                          \
                while (instance_shared_ptr() && instance_shared_ptr().use_count() > 0 && waitCnt < 40) {  \
                    usleep(5000);                                                                         \
                    waitCnt++;                                                                            \
                }                                                                                         \
                SignalReleaseEnd()();                                                                     \
            }                                                                                             \
        }                                                                                                 \
                                                                                                          \
        static void AsyncRelease() {                                                                      \
            mars::comm::ScopedLock lock(singleton_mutex());                                               \
            if (instance_shared_ptr()) {                                                                  \
                std::shared_ptr<classname> tmp_ptr = instance_shared_ptr();                               \
                SignalRelease()(tmp_ptr);                                                                 \
                instance_shared_ptr().reset();                                                            \
                SignalReleaseEnd()();                                                                     \
                std::thread t([=]() mutable {                                                             \
                    tmp_ptr.reset();                                                                      \
                });                                                                                       \
                t.detach();                                                                               \
            }                                                                                             \
        }                                                                                                 \
                                                                                                          \
        static std::shared_ptr<classname> Reset() {                                                       \
            mars::comm::ScopedLock lock(singleton_mutex());                                               \
            if (instance_shared_ptr()) {                                                                  \
                SignalResetOld()(*const_cast<classname*>(instance_shared_ptr().get()));                   \
                instance_shared_ptr().reset();                                                            \
            }                                                                                             \
                                                                                                          \
            if (!instance_shared_ptr()) {                                                                 \
                instance_shared_ptr().reset(const_cast<classname*>(creater), Singleton::Delete);          \
                SignalResetNew()(*const_cast<classname*>(instance_shared_ptr().get()));                   \
            }                                                                                             \
            return (instance_shared_ptr());                                                               \
        }                                                                                                 \
                                                                                                          \
     private:                                                                                             \
        static void Delete(classname* _ptr) {                                                             \
            destoryer(_ptr);                                                                              \
        }                                                                                                 \
    }

namespace design_patterns {

namespace SingletonHelper {
template <typename T>
class CreateInstanceHelper {
 public:
    T* operator()() {
        return new T();
    }
};

template <typename T>
class DestoryInstanceHelper {
 public:
    void operator()(T* _instance) {
        delete _instance;
    }
};
}  // namespace SingletonHelper

class Singleton {
 protected:
    class SingletonInfo {
     public:
        virtual ~SingletonInfo() {
        }
        virtual void DoRelease() = 0;
        virtual void* GetInstance() const = 0;
    };

 private:
    template <typename T>
    class SingletonInstance {
     public:
        // static std::shared_ptr<T> instance_shared_ptr;

        static std::shared_ptr<T>& instance_shared_ptr() {
            NO_DESTROY static std::shared_ptr<T> ptr;
            return ptr;
        }
        static mars::comm::Mutex& singleton_mutex() {
            NO_DESTROY static mars::comm::Mutex s_mutex;
            return s_mutex;
        }
    };

    template <typename T>
    class SingletonInfoImpl : public SingletonInfo {
     public:
        SingletonInfoImpl() {
        }
        virtual void DoRelease() {
            mars::comm::ScopedLock lock(SingletonInstance<T>::singleton_mutex());

            if (SingletonInstance<T>::instance_shared_ptr()) {
                SingletonInstance<T>::instance_shared_ptr().reset();
            }
        }

        virtual void* GetInstance() const {
            return const_cast<T*>(SingletonInstance<T>::instance_shared_ptr().get());
        }
    };

 private:
    Singleton();
    ~Singleton();
    Singleton(const Singleton&);
    Singleton& operator=(const Singleton&);

 public:
    static void ReleaseAll();

    template <typename T>
    static void Release() {
        _ReleaseSigleton(const_cast<T*>(SingletonInstance<T>::instance_shared_ptr().get()));
    }

    template <typename T, typename CREATER, typename DESTORYER>
    static std::shared_ptr<T> Instance(CREATER _creater, DESTORYER _destoryer) {
        std::shared_ptr<T> ret = SingletonInstance<T>::instance_shared_ptr();

        if (ret)
            return ret;

        mars::comm::ScopedLock lock(SingletonInstance<T>::singleton_mutex());

        if (!SingletonInstance<T>::instance_shared_ptr()) {
            _AddSigleton(new SingletonInfoImpl<T>());
            SingletonInstance<T>::instance_shared_ptr().reset(const_cast<T*>(_creater()), _destoryer);
        }

        return (SingletonInstance<T>::instance_shared_ptr());
    }

    template <typename T>
    static std::weak_ptr<T> Instance_Weak() {
        return SingletonInstance<T>::instance_shared_ptr();
    }

 protected:
    static void _AddSigleton(SingletonInfo* _helper);
    static void _ReleaseSigleton(void* _instance);

 private:
    NO_DESTROY static std::list<SingletonInfo*> lst_singleton_releasehelper_;
};

// template<typename T>
// std::shared_ptr<T> Singleton::SingletonInstance<T>::instance_shared_ptr;

// template<typename T>
// Mutex Singleton::SingletonInstance<T>::singleton_mutex();
}  // namespace design_patterns

#endif  // COMM_SINGLETON_H_
