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
 * longlink_packer.h
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#ifndef STN_SRC_LONGLINK_PACKER_H_
#define STN_SRC_LONGLINK_PACKER_H_

#include <stdlib.h>
#include <stdint.h>
#include <functional>

#define LONGLINK_UNPACK_CONTINUE (-2)
#define LONGLINK_UNPACK_FALSE (-1)
#define LONGLINK_UNPACK_OK (0)

//for HTTP2
#define LONGLINK_UNPACK_STREAM_END LONGLINK_UNPACK_OK
#define LONGLINK_UNPACK_STREAM_PACKAGE (1)

#ifndef __cplusplus
#error "support cpp only"
#endif

class AutoBuffer;

namespace mars {
    namespace stn {
    
class longlink_tracker {
public:
    static longlink_tracker* (*Create)();
    
public:
    virtual ~longlink_tracker(){};
};

class LongLinkEncoder {
 public:
    LongLinkEncoder();

    /**
     * package the request data
     * _cmdid: business identifier
     * _seq: task id
     * _raw: business send buffer
     * _packed: business send buffer + request header
     */
    std::function<void (uint32_t _cmdid, uint32_t _seq, const AutoBuffer& _body, const AutoBuffer& _extension, AutoBuffer& _packed, longlink_tracker* _tracker)> longlink_pack;

    /**
     * unpackage the response data
     * _packed: data received from server
     * _cmdid: business identifier
     * _seq: task id
     * _package_len:
     * _body: business receive buffer
     * return: 0 if unpackage succ
     */
    std::function<int (const AutoBuffer& _packed, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, AutoBuffer& _body, AutoBuffer& _extension, longlink_tracker* _tracker)> longlink_unpack;

    //heartbeat signal to keep longlink network alive
    std::function<uint32_t ()> longlink_noop_cmdid;
    std::function<bool (uint32_t _taskid, uint32_t _cmdid, uint32_t _recv_seq, const AutoBuffer& _body, const AutoBuffer& _extend)> longlink_noop_isresp;
    std::function<uint32_t ()> signal_keep_cmdid;
    std::function<void (AutoBuffer& _body, AutoBuffer& _extend)> longlink_noop_req_body;
    std::function<void (const AutoBuffer& _body, const AutoBuffer& _extend)> longlink_noop_resp_body;
    std::function<uint32_t ()> longlink_noop_interval;
    std::function<bool ()> longlink_complexconnect_need_verify;

    /**
     * return: whether the received data is pushing from server or not
     */
    std::function<bool (uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend)> longlink_ispush;
    std::function<bool (uint32_t _sent_seq, uint32_t _cmdid, uint32_t _recv_seq, const AutoBuffer& _body, const AutoBuffer& _extend)> longlink_identify_isresp;
};

extern LongLinkEncoder gDefaultLongLinkEncoder;

/**
 * package the request data
 * _cmdid: business identifier
 * _seq: task id
 * _raw: business send buffer
 * _packed: business send buffer + request header
 */
extern void (*longlink_pack)(uint32_t _cmdid, uint32_t _seq, const AutoBuffer& _body, const AutoBuffer& _extension, AutoBuffer& _packed, longlink_tracker* _tracker);

/**
 * unpackage the response data
 * _packed: data received from server
 * _cmdid: business identifier
 * _seq: task id
 * _package_len:
 * _body: business receive buffer
 * return: 0 if unpackage succ
 */
extern int  (*longlink_unpack)(const AutoBuffer& _packed, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, AutoBuffer& _body, AutoBuffer& _extension, longlink_tracker* _tracker);

//heartbeat signal to keep longlink network alive
extern uint32_t (*longlink_noop_cmdid)();
extern bool  (*longlink_noop_isresp)(uint32_t _taskid, uint32_t _cmdid, uint32_t _recv_seq, const AutoBuffer& _body, const AutoBuffer& _extend);
extern uint32_t (*signal_keep_cmdid)();
extern void (*longlink_noop_req_body)(AutoBuffer& _body, AutoBuffer& _extend);
extern void (*longlink_noop_resp_body)(const AutoBuffer& _body, const AutoBuffer& _extend);

extern uint32_t (*longlink_noop_interval)();

extern bool (*longlink_complexconnect_need_verify)();

/**
 * return: whether the received data is pushing from server or not
 */
extern bool  (*longlink_ispush)(uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend);
extern bool  (*longlink_identify_isresp)(uint32_t _sent_seq, uint32_t _cmdid, uint32_t _recv_seq, const AutoBuffer& _body, const AutoBuffer& _extend);

}
}
#endif // STN_SRC_LONGLINKPACKER_H_
