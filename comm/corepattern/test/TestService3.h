/*
 * TestService3.h
 *
 *  Created on: 2013-6-20
 *      Author: 叶润桂
 */

#ifndef TestService32_H_
#define TestService32_H_

#include "../service_base.h"

namespace design_patterns {

class TestService3: public design_patterns::ServiceBase {
public:
	TestService3(TServicesMap& _services):ServiceBase(ServiceName(), _services)
	{

	}
	virtual ~TestService3();

	DEFINE_SERVICE();
	int test2(int a, int b) {return a+b;}
};

} /* namespace design_patterns */
#endif /* TestService3_H_ */
