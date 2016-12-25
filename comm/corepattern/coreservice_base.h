/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
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
