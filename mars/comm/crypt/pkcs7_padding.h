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
 * pkcs7_padding.h
 *
 *  Created on: 2014-1-3
 *      Author: yerungui
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
