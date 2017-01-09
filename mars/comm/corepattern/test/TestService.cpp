/*
 * TestService.cpp
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#include "TestService.h"
#include "TestCoreService.h"
#include "ServiceUtil.h"


namespace design_patterns {

SERVICE_REGISTER(TestService, TestCoreService::ServiceName(), EPublic, EFirstGet);
IMPLEMENT_SERVICE(TestService)

TestService::~TestService() {
}

} /* namespace design_patterns */
