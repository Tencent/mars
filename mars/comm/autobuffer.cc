// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#include "autobuffer.h"
#include <stdint.h>
#include <stdlib.h>
#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#include "mars/comm/assert/__assert.h"


const AutoBuffer KNullAtuoBuffer;

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

AutoBuffer::AutoBuffer(size_t _nSize)
    : parray_(NULL)
    , pos_(0)
    , length_(0)
    , capacity_(0)
    , malloc_unitsize_(_nSize)
{}


AutoBuffer::AutoBuffer(void* _pbuffer, size_t _len, size_t _nSize)
    : parray_(NULL)
    , pos_(0)
    , length_(0)
    , capacity_(0)
    , malloc_unitsize_(_nSize) {
    Attach(_pbuffer, _len);
}

AutoBuffer::AutoBuffer(const void* _pbuffer, size_t _len, size_t _nSize)
    : parray_(NULL)
    , pos_(0)
    , length_(0)
    , capacity_(0)
    , malloc_unitsize_(_nSize) {
    Write(0, _pbuffer, _len);
}


AutoBuffer::~AutoBuffer() {
    Reset();
}

void AutoBuffer::AllocWrite(size_t _readytowrite, bool _changelength) {
    size_t nLen = Pos() + _readytowrite;
    __FitSize(nLen);

    if (_changelength) length_ = max(nLen, length_);
}

void AutoBuffer::AddCapacity(size_t _len) {
    __FitSize(Capacity() + _len);
}

void AutoBuffer::Write(const void* _pbuffer, size_t _len) {
    Write(Pos(), _pbuffer, _len);
    Seek(_len, ESeekCur);
}

void AutoBuffer::Write(off_t& _pos, const void* _pbuffer, size_t _len) {
    Write((const off_t&) _pos,  _pbuffer,  _len);
    _pos += _len;
}

void AutoBuffer::Write(const off_t& _pos, const void* _pbuffer, size_t _len) {
    ASSERT(NULL != _pbuffer || 0 == _len);
    ASSERT(0 <= _pos);
    ASSERT((size_t)_pos <= Length());
    size_t nLen = _pos + _len;
    __FitSize(nLen);
    length_ = max(nLen, length_);
    memcpy((unsigned char*)Ptr() + _pos, _pbuffer, _len);
}

void AutoBuffer::Write(TSeek _seek, const void* _pbuffer, size_t _len){
    off_t pos = 0;
    switch (_seek) {
        case ESeekStart:
            pos = 0;
            break;
        case ESeekCur:
            pos = pos_;
            break;
        case ESeekEnd:
            pos = length_;
            break;
        default:
            ASSERT(false);
            break;
    }
    
    Write(pos,  _pbuffer,  _len);
}

size_t AutoBuffer::Read(void* _pbuffer, size_t _len) {
    size_t readlen = Read(Pos(), _pbuffer, _len);
    Seek(readlen, ESeekCur);
    return readlen;
}

size_t AutoBuffer::Read(AutoBuffer& _rhs , size_t _len) {
    size_t readlen = Read(Pos(), _rhs, _len);
    Seek(readlen, ESeekCur);
    return readlen;
}

size_t AutoBuffer::Read(off_t& _pos, void* _pbuffer, size_t _len) const {
    size_t readlen = Read((const off_t&) _pos, _pbuffer,  _len);
    _pos += readlen;
    return readlen;
}

size_t AutoBuffer::Read(off_t& _pos, AutoBuffer& _rhs, size_t _len) const {
    size_t readlen = Read((const off_t&) _pos, _rhs,  _len);
    _pos += readlen;
    return readlen;
}

size_t AutoBuffer::Read(const off_t& _pos, void* _pbuffer, size_t _len) const {
    ASSERT(NULL != _pbuffer);
    ASSERT(0 <= _pos);
    ASSERT((size_t)_pos <= Length());

    size_t readlen = Length() - _pos;
    readlen = min(readlen, _len);
    memcpy(_pbuffer, PosPtr(), readlen);
    return readlen;
}

size_t AutoBuffer::Read(const off_t& _pos, AutoBuffer& _rhs, size_t _len) const {
    size_t readlen = Length() - _pos;
    readlen = min(readlen, _len);
    _rhs.Write(PosPtr(), readlen);
    return readlen;
}

off_t AutoBuffer::Move(off_t _move_len) {
    if (0 < _move_len) {
        __FitSize(Length() + _move_len);
        memmove(parray_ + _move_len, parray_, Length());
        memset(parray_, 0, _move_len);
        Length(Pos() + _move_len, Length() + _move_len);
    } else {
        size_t move_len = -_move_len;

        if (move_len > Length()) move_len = Length();

        memmove(parray_, parray_ + move_len, Length() - move_len);
        Length(move_len < (size_t)Pos() ? Pos() - move_len : 0, Length() - move_len);
    }

    return Length();
}

void  AutoBuffer::Seek(off_t _offset,  TSeek _eorigin) {
    switch (_eorigin) {
    case ESeekStart:
        pos_ = _offset;
        break;

    case ESeekCur:
        pos_ += _offset;
        break;

    case ESeekEnd:
        pos_ = length_ + _offset;
        break;

    default:
        ASSERT(false);
        break;
    }

    if (pos_ < 0)
        pos_ = 0;

    if ((size_t)pos_ > length_)
        pos_ = length_;
}

void AutoBuffer::Length(off_t _pos, size_t _lenght) {
    ASSERT(0 <= _pos);
    ASSERT((size_t)_pos <= _lenght);
    ASSERT(_lenght <= Capacity());
    length_ = _lenght;
    Seek(_pos, ESeekStart);
}

void*  AutoBuffer::Ptr(off_t _offset) {
    return (char*)parray_ + _offset;
}

const void*  AutoBuffer::Ptr(off_t _offset) const {
    return (const char*)parray_ + _offset;
}

void* AutoBuffer::PosPtr() {
    return ((unsigned char*)Ptr()) + Pos();
}

const void* AutoBuffer::PosPtr() const {
    return ((unsigned char*)Ptr()) + Pos();
}

off_t AutoBuffer::Pos() const {
    return pos_;
}

size_t AutoBuffer::PosLength() const {
    return length_ - pos_;
}

size_t AutoBuffer::Length() const {
    return length_;
}

size_t AutoBuffer::Capacity() const {
    return capacity_;
}

void AutoBuffer::Attach(void* _pbuffer, size_t _len) {
    Reset();
    parray_ = (unsigned char*)_pbuffer;
    length_ = _len;
    capacity_ = _len;
}

void AutoBuffer::Attach(AutoBuffer& _rhs) {
    Reset();
    parray_ = _rhs.parray_;
    pos_ = _rhs.pos_;
    length_ = _rhs.length_;
    capacity_ = _rhs.capacity_;

    _rhs.parray_ = NULL;
    _rhs.Reset();
}

void* AutoBuffer::Detach(size_t* _plen) {
    unsigned char* ret = parray_;
    parray_ = NULL;
    size_t nLen = Length();

    if (NULL != _plen)
        *_plen = nLen;

    Reset();
    return ret;
}

void AutoBuffer::Reset() {
    if (NULL != parray_)
        free(parray_);

    parray_ = NULL;
    pos_ = 0;
    length_ = 0;
    capacity_ = 0;
}

void AutoBuffer::__FitSize(size_t _len) {
    if (_len > capacity_) {
        size_t mallocsize = ((_len + malloc_unitsize_ -1)/malloc_unitsize_)*malloc_unitsize_ ;

        void* p = realloc(parray_, mallocsize);

        if (NULL == p) {
            ASSERT2(p, "_len=%" PRIu64 ", m_nMallocUnitSize=%" PRIu64 ", nMallocSize=%" PRIu64", m_nCapacity=%" PRIu64,
                    (uint64_t)_len, (uint64_t)malloc_unitsize_, (uint64_t)mallocsize, (uint64_t)capacity_);
            free(parray_);
        }

        parray_ = (unsigned char*) p;

        ASSERT2(_len <= 10 * 1024 * 1024, "%u", (uint32_t)_len);
        ASSERT(parray_);
        
        memset(parray_+capacity_, 0, mallocsize-capacity_);
        capacity_ = mallocsize;
    }
}
