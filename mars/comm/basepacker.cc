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
 *      Author: yerungui
 */

#include "basepacker.h"

#include <stdlib.h>

#include "mars/comm/autobuffer.h"

#include "socket/unix_socket.h"
#include "ptrbuffer.h"
#include "adler32.h"
#include "assert/__assert.h"

#pragma pack(1)
struct LongLinkPack {
    unsigned char   magic;
    unsigned char   ver;
    unsigned char   head_length;
    unsigned char   url_length;
    unsigned int    total_length;
    unsigned int    sequence;
    unsigned int    hash;
};

template <class T, unsigned int len = sizeof(T)>
class Endian {};

template <class T>
class Endian <T, 1> {
  public:
    inline static T hton(T t) { return t; }
    inline static T ntoh(T t) { return t; }
};

template <class T>
class Endian <T, 2> {
  public:
    static T hton(T t) { return htons(t); }
    static T ntoh(T t) { return ntohs(t); }
};

template <class T>
class Endian <T, 4> {
  public:
    static T hton(T t) { return htonl(t); }
    static T ntoh(T t) { return ntohl(t); }
};

template <class T>
inline T hton(T t) { return Endian<T>::hton(t); }

template <class T>
inline T ntoh(T t) { return Endian<T>::ntoh(t); }

template <class T>
inline T  SimplePackLength(size_t _datalen) {
    return (T)(_datalen + sizeof(T));
}

template <class T>
inline void SimplePack(const void* _data, size_t _datalen, AutoBuffer& _outbuf) {
    _outbuf.Write(hton(SimplePackLength<T>(_datalen)));
    _outbuf.Write(_data, _datalen);
}

template <class T>
inline void SimplePack(const void* _data, size_t _datalen, PtrBuffer& _outbuf) {
    _outbuf.Write(hton(SimplePackLength<T>(_datalen)));
    _outbuf.Write(_data, _datalen);
}

template <class T>
inline int SimpleUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, AutoBuffer& _data) {
    if (sizeof(T) > _rawlen) return SIMPLE_CONTINUE;

    _packlen = ntoh(*(T*)_rawbuf);

    if (_packlen > _rawlen) return SIMPLE_CONTINUE_DATA;

    _data.Write((const unsigned char*)_rawbuf + sizeof(T), _packlen - sizeof(T));
    return SIMPLE_OK;
}

template <class T>
inline int SimpleUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, PtrBuffer& _data) {
    if (sizeof(T) > _rawlen) return SIMPLE_CONTINUE;

    _packlen = ntoh(*(T*)_rawbuf);

    if (_packlen > _rawlen) return SIMPLE_CONTINUE_DATA;

    _data.Attach((unsigned char*)_rawbuf + sizeof(T), _packlen - sizeof(T), _packlen - sizeof(T));
    return SIMPLE_OK;
}


size_t  SimpleShortPackLength(size_t _datalen) {
    return SimplePackLength<uint16_t>(_datalen);
}

void SimpleShortPack(const void* _data, size_t _datalen, AutoBuffer& _outbuf) {
    SimplePack<uint16_t>(_data, _datalen, _outbuf);
}

int SimpleShortUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, AutoBuffer& _data) {
    return SimpleUnpack<uint16_t>(_rawbuf, _rawlen, _packlen, _data);
}

int SimpleShortUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, PtrBuffer& _data) {
    return SimpleUnpack<uint16_t>(_rawbuf, _rawlen, _packlen, _data);
}

size_t  SimpleIntPackLength(size_t _datalen) {
    return SimplePackLength<unsigned int>(_datalen);
}

void SimpleIntPack(const void* _data, size_t _datalen, AutoBuffer& _outbuf) {
    SimplePack<unsigned int>(_data, _datalen, _outbuf);
}

int SimpleIntUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, AutoBuffer& _data) {
    return SimpleUnpack<unsigned int>(_rawbuf, _rawlen, _packlen, _data);
}

int SimpleIntUnpack(const void* _rawbuf, size_t _rawlen, size_t& _packlen, PtrBuffer& _data) {
    return SimpleUnpack<unsigned int>(_rawbuf, _rawlen, _packlen, _data);
}

void Packer_Pack(const char* _url, unsigned int _sequence, const void* _data, size_t _datalen, AutoBuffer& _outbuf, bool _dohash) {
    ASSERT(_url);
    size_t url_size = strnlen(_url, 128);
    ASSERT(0xFF >= sizeof(LongLinkPack) + url_size);

    LongLinkPack st = {0};
    st.ver = 0x1;
    st.head_length = (unsigned char)sizeof(st);
    st.url_length = url_size;
    st.total_length = (unsigned int)(st.head_length + st.url_length  + _datalen);
    st.sequence = _sequence;
    st.magic = (st.head_length + st.url_length + st.total_length) & 0xFF;

    _outbuf.AllocWrite(st.total_length);

    if (_dohash) {
        st.hash = (unsigned int)adler32(0, (const unsigned char*)_url, (unsigned int)url_size);

        if (NULL != _data && 0 < _datalen) { st.hash = (unsigned int)adler32(st.hash, (const unsigned char*)_data, (unsigned int)_datalen);}
    }

    st.total_length = htonl(st.total_length);
    st.sequence = htonl(st.sequence);
    st.hash = htonl(st.hash);

    _outbuf.Write(&st, sizeof(st));
    _outbuf.Write(_url, url_size);
    _outbuf.Write(_data, _datalen);
}

int Packer_Unpack(const void* _rawbuf, size_t _rawlen, std::string& _url, unsigned int& _sequence, size_t& _packlen, AutoBuffer& _data) {
    if (_rawlen < sizeof(LongLinkPack)) return LONGLINKPACK_CONTINUE;

    LongLinkPack st = {0};
    memcpy(&st, _rawbuf, sizeof(LongLinkPack));

    st.total_length = ntohl(st.total_length);
    st.sequence = ntohl(st.sequence);
    st.hash = ntohl(st.hash);

    if (((st.head_length + st.url_length + st.total_length) & 0xFF) != st.magic) return __LINE__;

    if (st.url_length + st.head_length > st.total_length) return __LINE__;

    if (1024 * 1024 < st.total_length) return __LINE__;

    if (st.url_length + st.head_length > _rawlen) return LONGLINKPACK_CONTINUE_HEAD;

    _url.assign((const char*)_rawbuf + st.head_length, st.url_length);
    _sequence = st.sequence;
    _packlen = st.total_length;

    if (st.total_length > _rawlen) return LONGLINKPACK_CONTINUE_data;

    if (0 != st.hash && st.hash != adler32(0, (const unsigned char*)_rawbuf + st.head_length,  st.total_length - st.head_length)) return __LINE__;

    _data.Write((const char*)_rawbuf + st.head_length + st.url_length, st.total_length - (st.head_length + st.url_length));
    return LONGLINKPACK_OK;
}

int Packer_Unpack(const void* _rawbuf, size_t _rawlen, std::string& _url, unsigned int& _sequence, size_t& _packlen, PtrBuffer& _data) {
    if (_rawlen < sizeof(LongLinkPack)) return LONGLINKPACK_CONTINUE;

    LongLinkPack st = {0};
    memcpy(&st, _rawbuf, sizeof(LongLinkPack));

    st.total_length = ntohl(st.total_length);
    st.sequence = ntohl(st.sequence);
    st.hash = ntohl(st.hash);

    if (((st.head_length + st.url_length + st.total_length) & 0xFF) != st.magic) return __LINE__;

    if (st.url_length + st.head_length > st.total_length) return __LINE__;

    if (1024 * 1024 < st.total_length) return __LINE__;

    if (st.url_length + st.head_length > _rawlen) return LONGLINKPACK_CONTINUE_HEAD;

    _url.assign((const char*)_rawbuf + st.head_length, st.url_length);
    _sequence = st.sequence;
    _packlen = st.total_length;

    if (st.total_length > _rawlen) return LONGLINKPACK_CONTINUE_data;

    if (0 != st.hash && st.hash != adler32(0, (const unsigned char*)_rawbuf + st.head_length, st.total_length - st.head_length)) { return __LINE__; }

    _data.Attach((char*)_rawbuf + st.head_length + st.url_length, st.total_length - (st.head_length + st.url_length));
    return LONGLINKPACK_OK;
}
