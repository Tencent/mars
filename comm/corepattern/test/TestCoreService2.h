/*
 * TestCoreService2.h
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#ifndef TestCoreService2_H_
#define TestCoreService2_H_

#include "../coreservice_base.h"


namespace design_patterns {

class TestCoreService2: public design_patterns::CoreServiceBase {
public:
	TestCoreService2(TServicesMap& _parentsservices)
	:CoreServiceBase( ServiceName(), _parentsservices)
	{

	}
	virtual ~TestCoreService2();

	DEFINE_SERVICE();
};

} /* namespace design_patterns */
#endif /* TestCoreService2_H_ */
