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

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "boost/signals2.hpp"

#include "mars/comm/thread/lock.h"

#define SINGLETON_STRONG(class_name) design_patterns::Singleton::Instance<class_name>\
    (design_patterns::SingletonHelper::CreateInstanceHelper<class_name>(), design_patterns::SingletonHelper::DestoryInstanceHelper<class_name>())
#define SINGLETON_WEAK(class_name) design_patterns::Singleton::Instance_Weak<class_name>()

#define SINGLETON_RELEASE(class_name) design_patterns::Singleton::Release<class_name>()
#define SINGLETON_RELEASE_ALL() design_patterns::Singleton::ReleaseAll()

#define SINGLETON_INTRUSIVE(classname, creater, destoryer) \
    class Singleton\
    {\
      public:\
        static boost::shared_ptr<classname>& instance_shared_ptr() { static boost::shared_ptr<classname> s_ptr;return s_ptr;}\
        static Mutex& singleton_mutex() {static Mutex s_mutex; return s_mutex;}\
        static boost::signals2::signal<void ()>& SignalInstanceBegin() { static boost::signals2::signal<void ()> s_signal; return s_signal;} \
        static boost::signals2::signal<void (boost::shared_ptr<classname>)>& SignalInstance() { static boost::signals2::signal<void (boost::shared_ptr<classname>)> s_signal; return s_signal;} \
        static boost::signals2::signal<void (boost::shared_ptr<classname>)>& SignalRelease() { static boost::signals2::signal<void (boost::shared_ptr<classname>)> s_signal; return s_signal;} \
        static boost::signals2::signal<void ()>& SignalReleaseEnd() { static boost::signals2::signal<void ()> s_signal; return s_signal;} \
        static boost::signals2::signal<void (classname&)>& SignalResetOld() { static boost::signals2::signal<void (classname&)> s_signal; return s_signal;} \
        static boost::signals2::signal<void (classname&)>& SignalResetNew() { static boost::signals2::signal<void (classname&)> s_signal; return s_signal;} \
     \
      public:\
        static boost::shared_ptr<classname> Instance()\
        {\
            boost::shared_ptr<classname> ret = instance_shared_ptr();\
            if (ret) return ret;\
            \
            ScopedLock    lock(singleton_mutex());\
            if (!instance_shared_ptr())\
            {\
                SignalInstanceBegin()();\
                boost::shared_ptr<classname> temp(const_cast<classname*>(creater), Singleton::Delete);\
                SignalInstance()(temp);\
                instance_shared_ptr().swap(temp); \
            }\
            return ( instance_shared_ptr());\
        }\
        \
        template<class T>\
        static boost::shared_ptr<classname> Instance(const T& _creater)\
        {\
            boost::shared_ptr<classname> ret = instance_shared_ptr();\
            if (ret) return ret;\
            \
            ScopedLock    lock(singleton_mutex());\
            if (!instance_shared_ptr())\
            {\
                SignalInstanceBegin()();\
                boost::shared_ptr<classname> temp(const_cast<classname*>(_creater()), Singleton::Delete);\
                SignalInstance()(temp);\
                instance_shared_ptr().swap(temp); \
            }\
            return ( instance_shared_ptr());\
        }\
        \
        static boost::weak_ptr<classname> Instance_Weak() { return instance_shared_ptr();} \
        \
        static void Release()\
        {\
            ScopedLock    lock(singleton_mutex());\
            if (instance_shared_ptr())\
            {\
                SignalRelease()(instance_shared_ptr());\
                instance_shared_ptr().reset();\
                SignalReleaseEnd()();\
            }\
        }\
        \
        static boost::shared_ptr<classname>  Reset()\
        {\
            ScopedLock    lock(singleton_mutex());\
            if (instance_shared_ptr())\
            {\
                SignalResetOld()(*const_cast<classname*>(instance_shared_ptr().get()));\
                instance_shared_ptr().reset();\
            }\
            \
            if (!instance_shared_ptr())\
            {\
                instance_shared_ptr().reset(const_cast<classname*>(creater), Singleton::Delete); \
                SignalResetNew()(*const_cast<classname*>(instance_shared_ptr().get()));\
            }\
            return ( instance_shared_ptr());\
        }\
        \
      private: \
        static void Delete(classname* _ptr) { destoryer(_ptr); }\
    }


namespace design_patterns {

namespace SingletonHelper {
template<typename T>
class CreateInstanceHelper {
  public:
    T* operator()() {
        return new T();
    }
};

template<typename T>
class DestoryInstanceHelper {
  public:
    void operator()(T* _instance) {
        delete _instance;
    }
};
}

class Singleton {
  protected:
    class SingletonInfo {
      public:
        virtual ~SingletonInfo() {}
        virtual void DoRelease() = 0;
        virtual void* GetInstance() const = 0;
    };

  private:
    template<typename T>
    class SingletonInstance {
      public:
        //static boost::shared_ptr<T> instance_shared_ptr;
        
        static boost::shared_ptr<T>& instance_shared_ptr() {
            static boost::shared_ptr<T> ptr;
            return ptr;
        }
        static Mutex& singleton_mutex() {
            static Mutex s_mutex;
            return s_mutex;
        }
    };

    template<typename T>
    class SingletonInfoImpl : public SingletonInfo {
      public:
        SingletonInfoImpl() {}
        virtual void DoRelease() {
            ScopedLock    lock(SingletonInstance<T>::singleton_mutex());

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

    template<typename T> static
    void Release() { _ReleaseSigleton(const_cast<T*>(SingletonInstance<T>::instance_shared_ptr().get())); }

    template<typename T, typename CREATER, typename DESTORYER> static
    boost::shared_ptr<T> Instance(CREATER _creater, DESTORYER _destoryer) {
        boost::shared_ptr<T> ret = SingletonInstance<T>::instance_shared_ptr();

        if (ret) return ret;

        ScopedLock    lock(SingletonInstance<T>::singleton_mutex());

        if (!SingletonInstance<T>::instance_shared_ptr()) {
            _AddSigleton(new SingletonInfoImpl<T>());
            SingletonInstance<T>::instance_shared_ptr().reset(const_cast<T*>(_creater()), _destoryer);
        }

        return (SingletonInstance<T>::instance_shared_ptr());
    }

    template<typename T> static
    boost::weak_ptr<T> Instance_Weak() { return SingletonInstance<T>::instance_shared_ptr(); }


  protected:
    static void _AddSigleton(SingletonInfo* _helper);
    static void _ReleaseSigleton(void* _instance);

  private:
    static std::list<SingletonInfo*> lst_singleton_releasehelper_;
};

//template<typename T>
//boost::shared_ptr<T> Singleton::SingletonInstance<T>::instance_shared_ptr;

//template<typename T>
//Mutex Singleton::SingletonInstance<T>::singleton_mutex();
}



#endif	// COMM_SINGLETON_H_
