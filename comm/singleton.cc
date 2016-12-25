/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#include "comm/singleton.h"

namespace design_patterns {

static Mutex sg_singleton_mutex;
std::list<Singleton::SingletonInfo*> Singleton::lst_singleton_releasehelper_;

void Singleton::ReleaseAll() {
    ScopedLock    lock(sg_singleton_mutex);
    std::list<Singleton::SingletonInfo*> lst_copy = lst_singleton_releasehelper_;
    lst_singleton_releasehelper_.clear();
    lock.unlock();

    for (std::list<Singleton::SingletonInfo*>::reverse_iterator it = lst_copy.rbegin();
            it != lst_copy.rend(); ++it) {
        (*it)->DoRelease();
        delete(*it);
    }
}

void Singleton::_AddSigleton(Singleton::SingletonInfo* _helper) {
    ScopedLock    lock(sg_singleton_mutex);
    std::list<Singleton::SingletonInfo*>& lst = lst_singleton_releasehelper_;
    lst.push_back(_helper);
}

void Singleton::_ReleaseSigleton(void* _instance) {
    if (0 == _instance) return;

    ScopedLock    lock(sg_singleton_mutex);
    Singleton::SingletonInfo* releasesigleton = NULL;

    std::list<Singleton::SingletonInfo*>& lst = lst_singleton_releasehelper_;
    std::list<Singleton::SingletonInfo*>::iterator it = lst.begin();

    for (; it != lst.end(); ++it) {
        if ((*it)->GetInstance() == _instance) {
            releasesigleton = *it;
            lst.erase(it);
            break;
        }
    }

    lock.unlock();

    if (releasesigleton) {
        releasesigleton->DoRelease();
        delete releasesigleton;
    }
}

}  // namespace design_patterns
