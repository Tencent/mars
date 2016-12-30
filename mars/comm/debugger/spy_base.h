// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  spy.h
//  PublicComponent
//
//  Created by yerungui on 14-5-14.
//

#ifndef PublicComponent_spy_h
#define PublicComponent_spy_h

#include <map>
#include <string>

#include "boost/any.hpp"

#include "singleton.h"

class Spy {
  public:
    Spy(void* _this): m_this(_this) {}
    virtual ~Spy() {}

    template <class var> void Attach(const char* _key, var _var) {
        m_variablemap[_key] = _var;
        __OnAttach(_key);
    }

    void Detach(const char* _key) {
        __OnDetach(_key);
        m_variablemap.erase(_key);
    }

    template <class var> var* Variable(const char* _key) const
    { return boost::any_cast<var>(m_variablemap.find(_key)->second); }

    void* This() const {return m_this;}

  private:
    virtual void __OnAttach(const char* _key) = 0;
    virtual void __OnDetach(const char* _key) = 0;

  private:
    void* m_this;
    std::map<const std::string, boost::any> m_variablemap;
};

class SpyCore {
  public:
    SINGLETON_INTRUSIVE(SpyCore, new SpyCore, delete);

    void AddSpy(void* _this, const char* _name, Spy* _spy) {
        m_thismap[_this] = _spy;

        if (_name) m_strmap[_name] = _spy;
    }

    void RemoveSpy(void* _this) {
        m_thismap.erase(_this);

        std::map<const std::string, Spy*>::iterator it;

        for (it = m_strmap.begin(); it != m_strmap.end(); ++it) {
            if (it->second->This() == _this) {
                m_strmap.erase(it);
                break;
            }
        }
    }

    Spy* GetSpy(const void* _this) const
    { return m_thismap.find(_this)->second; }

    Spy* GetSpy(const char* _name) const
    { return m_strmap.find(_name)->second; }

  private:
    SpyCore() {}
    ~SpyCore() {}

  private:
    std::map<const void*, Spy*> m_thismap;
    std::map<const std::string, Spy*> m_strmap;
};

#endif
