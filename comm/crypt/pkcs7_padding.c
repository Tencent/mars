/*
 * pkcs7_padding.c
 *
 *  Created on: 2014-1-3
 *      Author: 叶润桂
 */

#include <string.h>

#include "comm/assert/__assert.h"

unsigned char pkcs7_padding_plus_len(unsigned int _datalen, unsigned char _blocksize)
{
    ASSERT2(0<_blocksize && 0<_datalen, "datalen=%u, blocksize=%u", _datalen, _blocksize);
    if (!(0<_blocksize && 0<_datalen)) return 0;
    return (unsigned char)(_blocksize - _datalen%_blocksize);
}

unsigned int pkcs7_padding_len(unsigned int _datalen, unsigned char _blocksize)
{
    unsigned char plus_len =  0;
    plus_len = pkcs7_padding_plus_len(_datalen, _blocksize);
    if (0==plus_len) return 0;
    return _datalen + plus_len;
}

unsigned int pkcs7_padding(void* _data, unsigned int _datalen, unsigned char _blocksize)
{
    unsigned char plus_len = 0;
    ASSERT(NULL!=_data);
    if (NULL==_data) return 0;

    plus_len = pkcs7_padding_plus_len(_datalen, _blocksize);
    if (0==plus_len) return 0;
    memset((unsigned char*)_data+_datalen, plus_len, plus_len);
    return _datalen + plus_len;
}

unsigned int pkcs7_depadding(const void* _data, unsigned int _datapaddinglen)
{
    unsigned char plus_len = 0;
    unsigned int offset = 0;

    ASSERT2(NULL!=_data && 0<_datapaddinglen, "_data=%p, _datapaddinglen=%u", _data, _datapaddinglen);
    if (!(NULL!=_data && 0<_datapaddinglen)) return 0;

    plus_len = ((const unsigned char*)_data)[_datapaddinglen-1];
    if (_datapaddinglen < plus_len) return 0;

    for (; offset<plus_len; ++offset)
    {
        if (plus_len!=((const unsigned char*)_data)[_datapaddinglen-1-offset]) return 0;
    }
    return _datapaddinglen - plus_len;
}

unsigned int pkcs7_depadding_blocksize(const void* _data, unsigned int _datapaddinglen, unsigned char _blocksize)
{
    unsigned int data_depadding_len = 0;
    unsigned char verify_plus_len = 0;
    unsigned char plus_len = 0;

    data_depadding_len = pkcs7_depadding(_data, _datapaddinglen);
    if (0==data_depadding_len) return 0;

    verify_plus_len = pkcs7_padding_plus_len(data_depadding_len, _blocksize);
    if (0==verify_plus_len) return 0;

    plus_len = ((const unsigned char*)_data)[_datapaddinglen-1];
    if (0==plus_len) return 0;

    if (verify_plus_len!=plus_len) return 0;

    return data_depadding_len;
}
