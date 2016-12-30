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
 * sdt_logic.h
 *
 *  Created on: 2016年3月17日
 *      Author: caoshaokun
 */

#ifndef SDT_INTERFACE_SDT_LOGIC_H_
#define SDT_INTERFACE_SDT_LOGIC_H_

#include "mars/sdt/sdt.h"

namespace mars {
namespace sdt {

	class Callback
	{
	public:
		virtual ~Callback() {};
	};

	void SetCallBack(Callback* const callback);

	void SetHttpNetcheckCGI(std::string cgi);

	//active netcheck interface
	void StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout);
	void CancelActiveCheck();

}}

#endif /* SDT_INTERFACE_SDT_LOGIC_H_ */
