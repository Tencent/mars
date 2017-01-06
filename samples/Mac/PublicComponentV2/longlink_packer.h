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
 * longlink_packer.cpp
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#ifndef STN_SRC_LONGLINK_PACKER_H_
#define STN_SRC_LONGLINK_PACKER_H_

#include <stdlib.h>

#define LONGLINK_UNPACK_CONTINUE (-2)
#define LONGLINK_UNPACK_FALSE (-1)
#define LONGLINK_UNPACK_OK (0)

#ifndef __cplusplus
#error "support cpp only"
#endif

class AutoBuffer;

void longlink_pack(uint32_t _cmdid, uint32_t _seq, const void* _raw, size_t _raw_len, AutoBuffer& _packed);
int  longlink_unpack(const AutoBuffer& _packed, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, AutoBuffer& _body);

//heartbeat signal to keep longlink network alive
uint32_t longlink_noop_cmdid();
uint32_t longlink_noop_resp_cmdid();
uint32_t signal_keep_cmdid();
void longlink_noop_req_body(AutoBuffer& _body);
void longlink_noop_resp_body(AutoBuffer& _body);

uint32_t longlink_noop_interval();

bool longlink_complexconnect_need_verify();
bool is_push_data(uint32_t _cmdid, uint32_t _taskid);

#endif // STN_SRC_LONGLINKPACKER_H_
