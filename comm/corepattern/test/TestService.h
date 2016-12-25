/*
 * TestService.h
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#ifndef TESTSERVICE_H_
#define TESTSERVICE_H_

#include "../service_base.h"

namespace design_patterns {

class TestService: public design_patterns::ServiceBase {
public:
	TestService(TServicesMap& _services):ServiceBase(ServiceName(), _services)
	{

	}
	virtual ~TestService();


	DEFINE_SERVICE();
	int test(int a, int b) {return a+b;}
};

} /* namespace design_patterns */
#endif /* TESTSERVICE_H_ */
