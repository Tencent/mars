/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * ServiceBase.h
 *
 *  Created on: 2013-6-20
 *      Author: yerungui
 */

#ifndef SERVICEBASE_H_
#define SERVICEBASE_H_

#include <string>
#include <set>
#include <map>

namespace design_patterns {


class ServiceBase;
typedef std::map<std::string, ServiceBase*> TServicesMap;

class ServiceBase {
  public:
    virtual ~ServiceBase() {}
    void DependServices(const TServicesMap& _dependservices) { m_dependservices = _dependservices;}
    const char* ServiceName() const { return m_servicename.c_str();}

  protected:
    ServiceBase(const char* _servicename) : m_servicename(_servicename) {}

    template<typename T>
    T* _Service() {
        if (m_dependservices.end() != m_dependservices.find(T::ServiceName()))
            return (T*)m_dependservices[T::ServiceName()];

        return NULL;
    }

  protected:
    std::string   m_servicename;
    TServicesMap m_dependservices;
};

}

#define DEFINE_SERVICE() \
    static  const char* ServiceName();\
    static const std::set<std::string>& DependServicesName();
#endif
