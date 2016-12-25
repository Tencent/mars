/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * longlink_packer.cpp
 *
 *  Created on: 2012-7-18
 *      Author: 叶润桂
 */

#ifndef STN_SRC_LONGLINK_PACKER_H_
#define STN_SRC_LONGLINK_PACKER_H_

#include <stdlib.h>
#include <stdint.h>

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
void longlink_noop_req_body(AutoBuffer& _body);
void longlink_noop_resp_body(AutoBuffer& _body);

#endif // STN_SRC_LONGLINKPACKER_H_
