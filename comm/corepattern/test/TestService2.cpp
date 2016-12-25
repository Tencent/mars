/*
 * TestService2.cpp
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#include "TestService2.h"
#include "TestService.h"
#include "TestCoreService2.h"
#include "ServiceUtil.h"


namespace design_patterns {

SERVICE_REGISTER(TestService2, TestCoreService2::ServiceName(), EPublic, EFirstGet);
IMPLEMENT_SERVICE(TestService2);


TestService2::~TestService2() {
}

} /* namespace design_patterns */
