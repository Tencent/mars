/*
 * TestCoreService.h
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#ifndef TESTCORESERVICE_H_
#define TESTCORESERVICE_H_

#include "../coreservice_base.h"

namespace design_patterns {

class TestCoreService: public design_patterns::CoreServiceBase {
public:
	TestCoreService();
	virtual ~TestCoreService();

	DEFINE_SERVICE();
};

} /* namespace design_patterns */
#endif /* TESTCORESERVICE_H_ */
