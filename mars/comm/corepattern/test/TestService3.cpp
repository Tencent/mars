/*
 * TestService3.cpp
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#include "TestService3.h"
#include "TestCoreService2.h"
#include "TestService2.h"
#include "TestService.h"
#include "ServiceUtil.h"


namespace design_patterns {

SERVICE_REGISTER(TestService3, TestCoreService2::ServiceName(), EPrivate, EFirstGet);
IMPLEMENT_SERVICE(TestService3, TestService2::ServiceName(), TestService::ServiceName());

TestService3::~TestService3() {
}

} /* namespace design_patterns */
