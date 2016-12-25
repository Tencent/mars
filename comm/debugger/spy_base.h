//
//  spy.h
//  PublicComponent
//
//  Created by Ray on 14-5-14.
//  Copyright (c) 2014年 Tencent. All rights reserved.
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
