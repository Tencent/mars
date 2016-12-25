//
//  ptrbuffer.h
//
//  Created by yerungui on 13-4-4.
//  Copyright (c) 2013年 Tencent. All rights reserved.
//

#ifndef COMM_PTRBUFFER_H_
#define COMM_PTRBUFFER_H_

#include <sys/types.h>
#include <string.h>

class PtrBuffer {
  public:
    enum TSeek {
        kSeekStart,
        kSeekCur,
        kSeekEnd,
    };
  public:
    PtrBuffer(void* _ptr, size_t _len, size_t _maxlen);
    PtrBuffer(void* _ptr, size_t _len);
    PtrBuffer();
    ~PtrBuffer();

    template<class T> void Write(const T& _val)
    { Write(&_val, sizeof(_val)); }

    template<class T> void Write(int _nPos, const T& _val)
    { Write(&_val, sizeof(_val), _nPos);}

    void Write(const char* const _val)
    { Write(_val, (unsigned int)strlen(_val));}

    void Write(int _nPos, const char* const _val)
    { Write(_val, (unsigned int)strlen(_val), _nPos);}

    void Write(const void* _pBuffer, size_t _nLen);
    void Write(const void* _pBuffer, size_t _nLen, off_t _nPos);

    template<class T> void Read(T& _val)
    { Read(&_val, sizeof(_val)); }

    template<class T> void Read(int _nPos, const T& _val) const
    { Read(&_val, sizeof(_val), _nPos); }

    size_t Read(void* _pBuffer, size_t _nLen);
    size_t Read(void* _pBuffer, size_t _nLen, off_t _nPos) const;

    void Seek(off_t _nOffset,  TSeek _eOrigin = kSeekCur);
    void Length(off_t _nPos, size_t _nLenght);

    void* Ptr();
    void* PosPtr();
    const void* Ptr() const;
    const void* PosPtr() const;

    off_t Pos() const;
    size_t PosLength() const;
    size_t Length() const;
    size_t MaxLength() const;

    void Attach(void* _pBuffer, size_t _nLen, size_t _maxlen);
    void Attach(void* _pBuffer, size_t _nLen);
    void Reset();

  private:
    PtrBuffer(const PtrBuffer& _rhs);
    PtrBuffer& operator = (const PtrBuffer& _rhs);

  private:
    unsigned char* parray_;
    off_t pos_;
    size_t length_;
    size_t max_length_;
};

extern const PtrBuffer KNullPtrBuffer;


#endif	// COMM_PTRBUFFER_H_
