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
 * CoreServiceBase.h
 *
 *  Created on: 2013-6-20
 *      Author: yerungui
 */

#ifndef CORESERVICEBASE_H_
#define CORESERVICEBASE_H_

#include <vector>

#include "comm/corepattern/service_base.h"

namespace design_patterns {

struct ServiceRegister;

class CoreServiceBase: public ServiceBase {
  protected:
    CoreServiceBase(const char* _servicename);
    virtual ~CoreServiceBase();

  public:
    template<typename T>
    T* Service() {
        if (m_publicservices.end() != m_publicservices.find(T::ServiceName()))
            return (T*)m_publicservices[T::ServiceName()];

        __FirstGetCreater(T::ServiceName());

        if (m_publicservices.end() != m_publicservices.find(T::ServiceName()))
            return (T*)m_publicservices[T::ServiceName()];

        return NULL;
    }

  private:
    void __StartupCreater();
    void __FirstGetCreater(const std::string& _servicename);
    void __Creater(std::vector<ServiceRegister>& _vec);

  private:
    TServicesMap m_services;
    TServicesMap m_publicservices;
    std::vector<ServiceBase*> m_releasevec;
};

}
#endif
