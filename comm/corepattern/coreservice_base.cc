/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * CoreServiceBase.cpp
 *
 *  Created on: 2013-6-20
 *      Author: yerungui
 */

#include <set>

#include "comm/corepattern/ServiceImpl.inl"
#include "comm/bootregister.h"
#include "comm/corepattern/coreservice_base.h"
#include "comm/xlogger/xlogger.h"

namespace design_patterns {


CoreServiceBase::CoreServiceBase(const char* _servicename)
    : ServiceBase(_servicename) {
    __StartupCreater();
}


CoreServiceBase::~CoreServiceBase() {
    for (std::vector<ServiceBase*>::reverse_iterator it = m_releasevec.rbegin(); it != m_releasevec.rend(); ++it) {
        delete(*it);
    }
}

void CoreServiceBase::__StartupCreater() {
    std::vector<ServiceRegister>& svrreg = BootRegister_Container<ServiceRegister>();
    std::vector<ServiceRegister> veccreater;

    for (std::vector<ServiceRegister>::iterator it = svrreg.begin(); it != svrreg.end(); ++it) {
        if (it->coreservice == ServiceName() && EStartup == it->servicecreate) {
            veccreater.push_back(*it);
        }
    }

    __Creater(veccreater);
}


void CoreServiceBase::__FirstGetCreater(const std::string& _servicename) {
    if (m_services.end() != m_services.find(_servicename)) return;

    std::vector<ServiceRegister>& svrreg = BootRegister_Container<ServiceRegister>();
    std::vector<ServiceRegister> veccreater;

    for (std::vector<ServiceRegister>::iterator it = svrreg.begin(); it != svrreg.end(); ++it) {
        if (it->coreservice == ServiceName() && it->ServiceName() == _servicename && EFirstGet == it->servicecreate) {
            veccreater.push_back(*it);
            break;
        }
    }

    __Creater(veccreater);
}


void CoreServiceBase::__Creater(std::vector<ServiceRegister>& _vec) {
    std::vector<ServiceRegister>& svrreg = BootRegister_Container<ServiceRegister>();
    std::set<std::string> servicedepend;

    for (std::vector<ServiceRegister>::iterator it = _vec.begin(); it != _vec.end(); ++it) {
        servicedepend.insert(it->DependServicesName().begin(), it->DependServicesName().end());
    }

    do {
        size_t count = servicedepend.size();

        for (std::vector<ServiceRegister>::iterator it = svrreg.begin(); it != svrreg.end(); ++it) {
            if (it->coreservice == ServiceName()  && servicedepend.end() != servicedepend.find(it->ServiceName())) {
                servicedepend.insert(it->DependServicesName().begin(), it->DependServicesName().end());
            }
        }

        if (count >= servicedepend.size()) {
            break;
        }
    } while (true);


    for (std::vector<ServiceRegister>::iterator it = svrreg.begin(); it != svrreg.end(); ++it) {
        if (it->coreservice == ServiceName()  && servicedepend.end() != servicedepend.find(it->ServiceName())) {
            servicedepend.insert(it->DependServicesName().begin(), it->DependServicesName().end());
            _vec.push_back(*it);
        }
    }

    do {
        size_t count = _vec.size();

        for (std::vector<ServiceRegister>::iterator it = _vec.begin(); it != _vec.end();) {
            if (m_services.end() != m_services.find(it->ServiceName())) {
                it = _vec.erase(it);
                continue;
            }

            ServiceBase* p = it->ServiceCreater(m_services, m_dependservices);

            if (p) {
                m_releasevec.push_back(p);
                m_services[it->ServiceName()] = p;

                if (EPublic == it->servicevisible) m_publicservices[it->ServiceName()] = p;

                it = _vec.erase(it);
            } else {
                ++it;
            }
        }

        if (0 == _vec.size()) break;

        if (count <= _vec.size()) {
            xassert2(false);
            break;
        }
    } while (true);
}
}  // namespace design_patterns
