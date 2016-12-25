/*
 * TestService2.h
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#ifndef TestService22_H_
#define TestService22_H_

#include "../service_base.h"

namespace design_patterns {

class TestService2: public design_patterns::ServiceBase {
public:
	TestService2(TServicesMap& _services):ServiceBase(ServiceName(), _services)
	{

	}
	virtual ~TestService2();

	DEFINE_SERVICE();
	int test2(int a, int b) {return a+b;}
};

} /* namespace design_patterns */
#endif /* TestService2_H_ */
