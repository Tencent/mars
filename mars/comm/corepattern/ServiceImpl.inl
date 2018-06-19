/*
 * ServiceUtil.h
 *
 *  Created on: 2013-6-20
 *      Author: yerungui
 */

#ifndef SERVICEUTIL_H_
#define SERVICEUTIL_H_

#include "comm/bootregister.h"

#include "comm/corepattern/service_base.h"

namespace design_patterns
{

enum TServiceVisible
{
    EPublic,
    EPrivate,
};

enum TServiceCreate
{
    EStartup,
    EFirstGet,
};

struct ServiceRegister
{
    ServiceRegister(ServiceBase* (*_ServiceCreater)(const TServicesMap&, const TServicesMap&),
                    const char* (*_ServiceName)(), const std::set<std::string>& (*_DependServicesName)(),
    const std::string& _coreservice, TServiceVisible _servicevisible, TServiceCreate  _servicecreate)
    : ServiceCreater(_ServiceCreater), ServiceName(_ServiceName), DependServicesName(_DependServicesName),
      coreservice(_coreservice), servicevisible(_servicevisible), servicecreate(_servicecreate)
      {}

    ServiceBase* (*ServiceCreater)(const TServicesMap& _services, const TServicesMap& _parentsservices);
    const char* (*ServiceName)();
    const std::set<std::string>& (*DependServicesName)();

    std::string coreservice;
    TServiceVisible servicevisible;
    TServiceCreate  servicecreate;
};

template <typename T>
inline ServiceBase* ServiceCreater(const TServicesMap& _services, const TServicesMap& _parentsservices)
{
    const std::set<std::string>& dependservicesname = T::DependServicesName();

    TServicesMap servicestemp;
    for (std::set<std::string>::const_iterator it=dependservicesname.begin(); it!=dependservicesname.end(); ++it)
    {
        TServicesMap::const_iterator it_find = _services.find(*it);
        if (_services.end()!=it_find)
        {
            servicestemp[*it] = it_find->second;
        }
        else
        {
            it_find = _parentsservices.find(*it);
            if (_parentsservices.end()!=it_find)
            {
                servicestemp[*it] = it_find->second;
            }
            else
            {
                return NULL;
            }
        }
    }
    ServiceBase* ret = new T();
    ret->DependServices(servicestemp);
    return ret;
}

struct __null_type
{
    static const char* ServiceName() { return "";}    
};

template <typename T0=__null_type, typename T1=__null_type, typename T2=__null_type, typename T3=__null_type,
          typename T4=__null_type, typename T5=__null_type, typename T6=__null_type, typename T7=__null_type>
struct __ServicesNameSet
{
    static std::set<std::string> Make()
    {
        std::set<std::string> rtn;
        rtn.insert(T0::ServiceName());
        rtn.insert(T1::ServiceName());
        rtn.insert(T2::ServiceName());
        rtn.insert(T3::ServiceName());
        rtn.insert(T4::ServiceName());
        rtn.insert(T5::ServiceName());
        rtn.insert(T6::ServiceName());
        rtn.insert(T7::ServiceName());
        rtn.erase("");
        return rtn;
    }
};
    
inline std::set<std::string> _ComputeCoreServiceDepend(const std::string& _servicename)
{
    std::vector<ServiceRegister>& svrreg = BOOT_REGISTER_CONTAINER<ServiceRegister>();
    std::set<std::string> servicedepend;
    for (std::vector<ServiceRegister>::iterator it=svrreg.begin(); it!=svrreg.end(); ++it)
    {
        if (it->coreservice==_servicename)
        {
            servicedepend.insert(it->DependServicesName().begin(), it->DependServicesName().end());
        }
    }
    
    for (std::vector<ServiceRegister>::iterator it=svrreg.begin(); it!=svrreg.end(); ++it)
    {
        if (it->coreservice==_servicename && servicedepend.end()!=servicedepend.find(it->ServiceName()))
        {
            servicedepend.erase(it->ServiceName());
        }
    }
    
    return servicedepend;
    
}

}

#define SERVICE_REGISTER(classname, coreservice, servicevisible, servicecreate) \
    BOOT_REGISTER_CHECK(classname, design_patterns::ServiceRegister(&design_patterns::ServiceCreater<classname>, &classname::ServiceName, &classname::DependServicesName,\
            coreservice::ServiceName(), servicevisible, servicecreate))

#define IMPLEMENT_SERVICE(classname, ...) \
    static const char* const _test_servicename_##classname##_check = #classname;\
    const char* classname::ServiceName()\
    {return _test_servicename_##classname##_check;}\
    const std::set<std::string>& classname::DependServicesName()\
    {\
        static std::set<std::string> s_dependservicesname = design_patterns::__ServicesNameSet<__VA_ARGS__>::Make();\
        return s_dependservicesname;\
    }


#define IMPLEMENT_CORE_SERVICE(classname) \
    VARIABLE_IS_NOT_USED static const char* const _test_servicename_##classname##_check = #classname;\
    const char* classname::ServiceName()\
    {return _test_servicename_##classname##_check;}\
    const std::set<std::string>& classname::DependServicesName()\
    {\
        static std::set<std::string> s_dependservicesname = design_patterns::_ComputeCoreServiceDepend(classname::ServiceName());\
        return s_dependservicesname;\
    }
#endif /* SERVICEUTIL_H_ */
