/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * pkcs7_padding.h
 *
 *  Created on: 2014-1-3
 *      Author: 叶润桂
 */

#ifndef PKCS7_PADDING_H_
#define PKCS7_PADDING_H_

#ifdef  __cplusplus
extern "C" {
#endif

unsigned int  pkcs7_padding(void* _data, unsigned int _datalen, unsigned char _blocksize);
unsigned int  pkcs7_padding_len(unsigned int _datalen, unsigned char _blocksize);
unsigned char pkcs7_padding_plus_len(unsigned int _datalen, unsigned char _blocksize);

unsigned int pkcs7_depadding(const void* _data, unsigned int _datapaddinglen);
unsigned int pkcs7_depadding_blocksize(const void* _data, unsigned int _datapaddinglen, unsigned char _blocksize);

#ifdef  __cplusplus
}
#endif

#endif /* PKCS7_PADDING_H_ */
