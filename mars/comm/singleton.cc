// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



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
