/*
 * TestCoreService.cpp
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#include "TestCoreService.h"
#include "ServiceUtil.h"

namespace design_patterns {

//SERVICE_REGISTER(TestCoreService, "", EPublic, EStartup);

IMPLEMENT_CORE_SERVICE(TestCoreService);

TestCoreService::TestCoreService()
:CoreServiceBase(ServiceName())
{

}

TestCoreService::~TestCoreService() {
}

} /* namespace design_patterns */
