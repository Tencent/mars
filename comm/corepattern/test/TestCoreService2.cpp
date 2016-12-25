/*
 * TestCoreService2.cpp
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#include "TestCoreService2.h"
#include "TestCoreService.h"
#include "ServiceUtil.h"

namespace design_patterns {

SERVICE_REGISTER(TestCoreService2, TestCoreService::ServiceName(), EPublic, EFirstGet);
IMPLEMENT_CORE_SERVICE(TestCoreService2);

TestCoreService2::~TestCoreService2() {
}

} /* namespace design_patterns */
