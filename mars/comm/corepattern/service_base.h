// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


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
