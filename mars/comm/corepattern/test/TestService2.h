// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * TestService2.h
 *
 *  Created on: 2013-6-20
 *      Author: yerungui
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
