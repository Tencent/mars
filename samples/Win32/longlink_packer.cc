/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * longlink_packer.cc
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */
#include "stdafx.h"
#include "longlink_packer.h"
#include "mars/comm/autobuffer.h"
#include "mars/xlog/xlogger.h"



static uint16_t sg_productID = 0;

#pragma pack(push, 1)
struct __STNetMsgXpHeader
{
    uint32_t    pack_length;
    uint16_t    magic;    // 0x0110
    uint16_t    product_id;
    uint32_t    cmdid;
    uint32_t    seq;
};
#pragma pack(pop)

namespace mars {
namespace stn {
	void SetClientVersion(uint16_t _productID)  {
		sg_productID = _productID;
	}
}
}

static int __unpack_test(const void* _packed, size_t _packed_len, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, size_t& _body_len)
{
    __STNetMsgXpHeader st = {0};
    if (_packed_len < sizeof(__STNetMsgXpHeader)) {
        _package_len = 0;
        _body_len = 0;
        return LONGLINK_UNPACK_CONTINUE;
    }
    
    memcpy(&st, _packed, sizeof(__STNetMsgXpHeader));
    
    uint16_t magic = ntohs(st.magic);
    if ( 0x0110 != magic){
        _package_len = 0;
        _body_len = 0;
        return LONGLINK_UNPACK_FALSE;
    }
    
    _package_len = ntohl(st.pack_length);
    _body_len = _package_len;
    _cmdid = ntohl(st.cmdid);
    _seq = ntohl(st.seq);
    
    if (_package_len > 1024*1024) { return LONGLINK_UNPACK_FALSE; }
    if (_package_len > _packed_len) { return LONGLINK_UNPACK_CONTINUE; }
    
    return LONGLINK_UNPACK_OK;
}

void longlink_pack(uint32_t _cmdid, uint32_t _seq, const void* _raw, size_t _raw_len, AutoBuffer& _packed) {
    uint32_t cmdid = 0;
    uint32_t seq = 0;
    size_t package_len = 0;
    size_t body_len = 0;
    
    if (LONGLINK_UNPACK_OK == __unpack_test(_raw, _raw_len, cmdid,  seq, package_len, body_len)) {
        xassert2(false, "raw buffer had longlink header!!!");
        xassert2(_cmdid == cmdid, TSF"task:%_ _raw:%_", _cmdid, cmdid);
        xassert2(_seq == seq, TSF"task:%_ _raw:%_", _seq, seq);
    }
    
    __STNetMsgXpHeader st = {0};
    st.pack_length = htonl(sizeof(__STNetMsgXpHeader) + _raw_len);
    st.magic = htons(0x0110);
    st.product_id = htons(sg_productID);
    st.cmdid = htonl(_cmdid);
    st.seq = htonl(_seq);

    _packed.AllocWrite(sizeof(__STNetMsgXpHeader)+_raw_len);
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

/**
 * nooping param
 */
#define NOOP_CMDID 6
uint32_t longlink_noop_cmdid() {return NOOP_CMDID;}
void longlink_noop_req_body(AutoBuffer& _body) {}
void longlink_noop_resp_body(AutoBuffer& _body) {}
