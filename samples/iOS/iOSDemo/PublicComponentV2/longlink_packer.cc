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
 * longlink_packer.cc
 *
 *  Created on: 2012-7-18
 *      Author: yerungui, caoshaokun
 */

#include "longlink_packer.h"

#ifdef __APPLE__
#include "mars/comm/autobuffer.h"
#include "mars/xlog/xlogger.h"
#else
#include "comm/autobuffer.h"
#include "comm/xlogger/xlogger.h"
#include "comm/socket/unix_socket.h"
#endif

static uint32_t sg_client_version = 0;

#pragma pack(push, 1)
struct __STNetMsgXpHeader {
    uint32_t    head_length;
    uint32_t    client_version;
    uint32_t    cmdid;
    uint32_t    seq;
    uint32_t	body_length;
};
#pragma pack(pop)

namespace mars {
namespace stn {
	void SetClientVersion(uint32_t _client_version)  {
		sg_client_version = _client_version;
	}
}
}

static int __unpack_test(const void* _packed, size_t _packed_len, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, size_t& _body_len) {
    __STNetMsgXpHeader st = {0};
    if (_packed_len < sizeof(__STNetMsgXpHeader)) {
        _package_len = 0;
        _body_len = 0;
        return LONGLINK_UNPACK_CONTINUE;
    }
    
    memcpy(&st, _packed, sizeof(__STNetMsgXpHeader));
    
    uint32_t head_len = ntohl(st.head_length);
    uint32_t client_version = ntohl(st.client_version);
    if (client_version != sg_client_version) {
        _package_len = 0;
        _body_len = 0;
    	return LONGLINK_UNPACK_FALSE;
    }
    _cmdid = ntohl(st.cmdid);
	_seq = ntohl(st.seq);
	_body_len = ntohl(st.body_length);
	_package_len = head_len + _body_len;

    if (_package_len > 1024*1024) { return LONGLINK_UNPACK_FALSE; }
    if (_package_len > _packed_len) { return LONGLINK_UNPACK_CONTINUE; }
    
    return LONGLINK_UNPACK_OK;
}

void longlink_pack(uint32_t _cmdid, uint32_t _seq, const void* _raw, size_t _raw_len, AutoBuffer& _packed) {
    __STNetMsgXpHeader st = {0};
    st.head_length = htonl(sizeof(__STNetMsgXpHeader));
    st.client_version = htonl(sg_client_version);
    st.cmdid = htonl(_cmdid);
    st.seq = htonl(_seq);
    st.body_length = htonl(_raw_len);

    _packed.AllocWrite(sizeof(__STNetMsgXpHeader) + _raw_len);
    _packed.Write(&st, sizeof(st));
    
    if (NULL != _raw) _packed.Write(_raw, _raw_len);
    
    _packed.Seek(0, AutoBuffer::ESeekStart);
}


int longlink_unpack(const AutoBuffer& _packed, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, AutoBuffer& _body) {
   size_t body_len = 0;
   int ret = __unpack_test(_packed.Ptr(), _packed.Length(), _cmdid,  _seq, _package_len, body_len);
    
    if (LONGLINK_UNPACK_OK != ret) return ret;
    
    _body.Write(AutoBuffer::ESeekCur, _packed.Ptr(_package_len-body_len), body_len);
    
    return ret;
}


#define NOOP_CMDID 6
#define SIGNALKEEP_CMDID 243
#define PUSH_DATA_TASKID 0

uint32_t longlink_noop_cmdid() {
    return NOOP_CMDID;
}


uint32_t longlink_noop_resp_cmdid() {
    return NOOP_CMDID;
}

uint32_t signal_keep_cmdid() {
    return SIGNALKEEP_CMDID;
}

void longlink_noop_req_body(AutoBuffer& _body) {}
void longlink_noop_resp_body(AutoBuffer& _body) {}

uint32_t longlink_noop_interval() {
	return 0;
}

bool longlink_complexconnect_need_verify() {  
    return false;
}

bool is_push_data(uint32_t _cmdid, uint32_t _taskid) {
    return PUSH_DATA_TASKID == _taskid;
}
