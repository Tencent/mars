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
 * TestCoreService.h
 *
 *  Created on: 2013-6-20
 *      Author: yerungui
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
