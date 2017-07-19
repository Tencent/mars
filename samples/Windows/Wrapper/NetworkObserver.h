// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
*  NetworkObserver.h
*
*  Created on: 2017-7-7
*      Author: chenzihao
*/

#ifndef _PUSH_OBSERVER_H_
#define _PUSH_OBSERVER_H_
#include <map>
#include <string>

#include "mars/comm/autobuffer.h"
#include "Wrapper/CGITask.h"

class PushObserver
{
public:
	virtual void OnPush(uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) = 0;

};

#endif