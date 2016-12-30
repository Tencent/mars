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
 * LongLinkPacker.cpp
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#ifndef COMM_BASEPACKER_H_
#define COMM_BASEPACKER_H_

#pragma once

#include <string>

#define SIMPLE_CONTINUE (-2)
#define SIMPLE_CONTINUE_DATA (-1)
#define SIMPLE_OK (0)

#define LONGLINKPACK_CONTINUE (-3)
#define LONGLINKPACK_CONTINUE_HEAD (-2)
#define LONGLINKPACK_CONTINUE_data (-1)
#define LONGLINKPACK_OK (0)

#ifndef __cplusplus
#error "support cpp only"
#endif

class AutoBuffer;
class PtrBuffer;

size_t  SimpleShortPackLength(size_t _datalen);
void SimpleShortPack(const void* _data, size_t _datalen, AutoBuffer& _outbuf);
int SimpleShortUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, AutoBuffer& _data);
int SimpleShortUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, PtrBuffer& _data);

size_t  SimpleIntPackLength(size_t _datalen);
void SimpleIntPack(const void* _data, size_t _datalen, AutoBuffer& _outbuf);
int SimpleIntUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, AutoBuffer& _data);
int SimpleIntUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, PtrBuffer& _data);

void Packer_Pack(const char* _url, unsigned int _sequence, const void* _data, size_t _datalen, AutoBuffer& _outbuf, bool _dohash = true);
int Packer_Unpack(const void* _rawbuf, size_t _rawlen, std::string &_url, unsigned int& _sequence, size_t& _packlen, AutoBuffer& _data);
int Packer_Unpack(const void* _rawbuf, size_t _rawlen, std::string &_url, unsigned int& _sequence, size_t& _packlen, PtrBuffer& _data);

#endif // COMM_BASEPACKER_H_
