// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef COMM_AUTOBUFFER_H_
#define COMM_AUTOBUFFER_H_

#include <sys/types.h>
#include <string.h>

class AutoBuffer {
  public:
    enum TSeek {
        ESeekStart,
        ESeekCur,
        ESeekEnd,
    };

  public:
    explicit AutoBuffer(size_t _size = 128);
    explicit AutoBuffer(void* _pbuffer, size_t _len, size_t _size = 128);
    explicit AutoBuffer(const void* _pbuffer, size_t _len, size_t _size = 128);
    ~AutoBuffer();

    void AllocWrite(size_t _readytowrite, bool _changelength = true);
    void AddCapacity(size_t _len);

    template<class T> void Write(const T& _val)
    { Write(&_val, sizeof(_val));}

    template<class T> void Write(off_t& _pos, const T& _val)
    { Write(_pos, &_val, sizeof(_val));}

    template<class T> void Write(const off_t& _pos, const T& _val)
    { Write(_pos, &_val, sizeof(_val));}

    void Write(const char* const _val)
    { Write(_val, strlen(_val));}

    void Write(off_t& _pos, const char* const _val)
    { Write(_pos, _val, strlen(_val));}

    void Write(const off_t& _pos, const char* const _val)
    { Write(_pos, _val, strlen(_val));}

    void Write(const AutoBuffer& _buffer);
    void Write(const void* _pbuffer, size_t _len);
    void Write(off_t& _pos, const AutoBuffer& _buffer);
    void Write(off_t& _pos, const void* _pbuffer, size_t _len);
    void Write(const off_t& _pos, const AutoBuffer& _buffer);
    void Write(const off_t& _pos, const void* _pbuffer, size_t _len);
    void Write(TSeek _seek, const void* _pbuffer, size_t _len);

    template<class T> size_t Read(T& _val)
    { return Read(&_val, sizeof(_val)); }

    template<class T> size_t Read(off_t& _pos, T& _val) const
    { return Read(_pos, &_val, sizeof(_val)); }

    template<class T> size_t Read(const off_t& _pos, T& _val) const
    { return Read(_pos, &_val, sizeof(_val)); }

    size_t Read(void* _pbuffer, size_t _len);
    size_t Read(AutoBuffer& _rhs , size_t _len);

    size_t Read(off_t& _pos, void* _pbuffer, size_t _len) const;
    size_t Read(off_t& _pos, AutoBuffer& _rhs, size_t _len) const;

    size_t Read(const off_t& _pos, void* _pbuffer, size_t _len) const;
    size_t Read(const off_t& _pos, AutoBuffer& _rhs, size_t _len) const;

    off_t Move(off_t _move_len);

    void Seek(off_t _offset,  TSeek _eorigin);
    void Length(off_t _pos, size_t _lenght);

    void* Ptr(off_t _offset=0);
    void* PosPtr();
    const void* Ptr(off_t _offset=0) const;
    const void* PosPtr() const;

    off_t Pos() const;
    size_t PosLength() const;
    size_t Length() const;
    size_t Capacity() const;

    void Attach(void* _pbuffer, size_t _len);
    void Attach(AutoBuffer& _rhs);
    void* Detach(size_t* _plen = NULL);

    void Reset();

  private:
    void __FitSize(size_t _len);

  private:
    AutoBuffer(const AutoBuffer& _rhs);
    AutoBuffer& operator = (const AutoBuffer& _rhs);

  private:
    unsigned char* parray_;
    off_t pos_;
    size_t length_;
    size_t capacity_;
    size_t malloc_unitsize_;
};

extern const AutoBuffer KNullAtuoBuffer;

template <class S> class copy_wrapper_helper;

template <>
class copy_wrapper_helper<AutoBuffer> {
  public:
    static void copy_constructor(AutoBuffer& _lhs, AutoBuffer& _rhs)
    { _lhs.Attach(_rhs); }

    static void copy_constructor(AutoBuffer& _lhs, const AutoBuffer& _rhs)
    { _lhs.Attach(const_cast<AutoBuffer&>(_rhs)); }

    static void destructor(AutoBuffer& _delobj) {}
};

#endif	//COMM_AUTOBUFFER_H_

