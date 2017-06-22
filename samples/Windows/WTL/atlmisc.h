// Windows Template Library - WTL version 8.1
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// This file is a part of the Windows Template Library.
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
// which can be found in the file CPL.TXT at the root of this distribution.
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.

#ifndef __ATLMISC_H__
#define __ATLMISC_H__

#pragma once

#ifndef __ATLAPP_H__
	#error atlmisc.h requires atlapp.h to be included first
#endif


#ifdef _ATL_TMP_NO_CSTRING
  #define _WTL_NO_CSTRING
#endif

#if defined(_WTL_USE_CSTRING) && defined(_WTL_NO_CSTRING)
	#error Conflicting options - both _WTL_USE_CSTRING and _WTL_NO_CSTRING are defined
#endif // defined(_WTL_USE_CSTRING) && defined(_WTL_NO_CSTRING)

#if !defined(_WTL_USE_CSTRING) && !defined(_WTL_NO_CSTRING)
  #define _WTL_USE_CSTRING
#endif // !defined(_WTL_USE_CSTRING) && !defined(_WTL_NO_CSTRING)

#ifndef _WTL_NO_CSTRING
  #if defined(_ATL_USE_CSTRING_FLOAT) && defined(_ATL_MIN_CRT)
	#error Cannot use CString floating point formatting with _ATL_MIN_CRT defined
  #endif // defined(_ATL_USE_CSTRING_FLOAT) && defined(_ATL_MIN_CRT)
#endif // !_WTL_NO_CSTRING


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CSize
// CPoint
// CRect
// CString
//
// CRecentDocumentListBase<T, t_cchItemLen, t_nFirstID, t_nLastID>
// CRecentDocumentList
// CFindFile
//
// Global functions:
//   AtlGetStockPen()
//   AtlGetStockBrush()
//   AtlGetStockFont()
//   AtlGetStockPalette()
//
//   AtlCompactPath()


namespace WTL
{

#ifndef _WTL_NO_WTYPES

// forward declarations
class CSize;
class CPoint;
class CRect;

///////////////////////////////////////////////////////////////////////////////
// CSize - Wrapper for Windows SIZE structure.

class CSize : public SIZE
{
public:
// Constructors
	CSize()
	{
		cx = 0;
		cy = 0;
	}

	CSize(int initCX, int initCY)
	{
		cx = initCX;
		cy = initCY;
	}

	CSize(SIZE initSize)
	{
		*(SIZE*)this = initSize;
	}

	CSize(POINT initPt)
	{
		*(POINT*)this = initPt;
	}

	CSize(DWORD dwSize)
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}

// Operations
	BOOL operator ==(SIZE size) const
	{
		return (cx == size.cx && cy == size.cy);
	}

	BOOL operator !=(SIZE size) const
	{
		return (cx != size.cx || cy != size.cy);
	}

	void operator +=(SIZE size)
	{
		cx += size.cx;
		cy += size.cy;
	}

	void operator -=(SIZE size)
	{
		cx -= size.cx;
		cy -= size.cy;
	}

	void SetSize(int CX, int CY)
	{
		cx = CX;
		cy = CY;
	}

// Operators returning CSize values
	CSize operator +(SIZE size) const
	{
		return CSize(cx + size.cx, cy + size.cy);
	}

	CSize operator -(SIZE size) const
	{
		return CSize(cx - size.cx, cy - size.cy);
	}

	CSize operator -() const
	{
		return CSize(-cx, -cy);
	}

// Operators returning CPoint values
	CPoint operator +(POINT point) const;
	CPoint operator -(POINT point) const;

// Operators returning CRect values
	CRect operator +(const RECT* lpRect) const;
	CRect operator -(const RECT* lpRect) const;
};


///////////////////////////////////////////////////////////////////////////////
// CPoint - Wrapper for Windows POINT structure.

class CPoint : public POINT
{
public:
// Constructors
	CPoint()
	{
		x = 0;
		y = 0;
	}

	CPoint(int initX, int initY)
	{
		x = initX;
		y = initY;
	}

	CPoint(POINT initPt)
	{
		*(POINT*)this = initPt;
	}

	CPoint(SIZE initSize)
	{
		*(SIZE*)this = initSize;
	}

	CPoint(DWORD dwPoint)
	{
		x = (short)LOWORD(dwPoint);
		y = (short)HIWORD(dwPoint);
	}

// Operations
	void Offset(int xOffset, int yOffset)
	{
		x += xOffset;
		y += yOffset;
	}

	void Offset(POINT point)
	{
		x += point.x;
		y += point.y;
	}

	void Offset(SIZE size)
	{
		x += size.cx;
		y += size.cy;
	}

	BOOL operator ==(POINT point) const
	{
		return (x == point.x && y == point.y);
	}

	BOOL operator !=(POINT point) const
	{
		return (x != point.x || y != point.y);
	}

	void operator +=(SIZE size)
	{
		x += size.cx;
		y += size.cy;
	}

	void operator -=(SIZE size)
	{
		x -= size.cx;
		y -= size.cy;
	}

	void operator +=(POINT point)
	{
		x += point.x;
		y += point.y;
	}

	void operator -=(POINT point)
	{
		x -= point.x;
		y -= point.y;
	}

	void SetPoint(int X, int Y)
	{
		x = X;
		y = Y;
	}

// Operators returning CPoint values
	CPoint operator +(SIZE size) const
	{
		return CPoint(x + size.cx, y + size.cy);
	}

	CPoint operator -(SIZE size) const
	{
		return CPoint(x - size.cx, y - size.cy);
	}

	CPoint operator -() const
	{
		return CPoint(-x, -y);
	}

	CPoint operator +(POINT point) const
	{
		return CPoint(x + point.x, y + point.y);
	}

// Operators returning CSize values
	CSize operator -(POINT point) const
	{
		return CSize(x - point.x, y - point.y);
	}

// Operators returning CRect values
	CRect operator +(const RECT* lpRect) const;
	CRect operator -(const RECT* lpRect) const;
};


///////////////////////////////////////////////////////////////////////////////
// CRect - Wrapper for Windows RECT structure.

class CRect : public RECT
{
public:
// Constructors
	CRect()
	{
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}

	CRect(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	CRect(const RECT& srcRect)
	{
		::CopyRect(this, &srcRect);
	}

	CRect(LPCRECT lpSrcRect)
	{
		::CopyRect(this, lpSrcRect);
	}

	CRect(POINT point, SIZE size)
	{
		right = (left = point.x) + size.cx;
		bottom = (top = point.y) + size.cy;
	}

	CRect(POINT topLeft, POINT bottomRight)
	{
		left = topLeft.x;
		top = topLeft.y;
		right = bottomRight.x;
		bottom = bottomRight.y;
	}

// Attributes (in addition to RECT members)
	int Width() const
	{
		return right - left;
	}

	int Height() const
	{
		return bottom - top;
	}

	CSize Size() const
	{
		return CSize(right - left, bottom - top);
	}

	CPoint& TopLeft()
	{
		return *((CPoint*)this);
	}

	CPoint& BottomRight()
	{
		return *((CPoint*)this + 1);
	}

	const CPoint& TopLeft() const
	{
		return *((CPoint*)this);
	}

	const CPoint& BottomRight() const
	{
		return *((CPoint*)this + 1);
	}

	CPoint CenterPoint() const
	{
		return CPoint((left + right) / 2, (top + bottom) / 2);
	}

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT()
	{
		return this;
	}

	operator LPCRECT() const
	{
		return this;
	}

	BOOL IsRectEmpty() const
	{
		return ::IsRectEmpty(this);
	}

	BOOL IsRectNull() const
	{
		return (left == 0 && right == 0 && top == 0 && bottom == 0);
	}

	BOOL PtInRect(POINT point) const
	{
		return ::PtInRect(this, point);
	}

// Operations
	void SetRect(int x1, int y1, int x2, int y2)
	{
		::SetRect(this, x1, y1, x2, y2);
	}

	void SetRect(POINT topLeft, POINT bottomRight)
	{
		::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
	}

	void SetRectEmpty()
	{
		::SetRectEmpty(this);
	}

	void CopyRect(LPCRECT lpSrcRect)
	{
		::CopyRect(this, lpSrcRect);
	}

	BOOL EqualRect(LPCRECT lpRect) const
	{
		return ::EqualRect(this, lpRect);
	}

	void InflateRect(int x, int y)
	{
		::InflateRect(this, x, y);
	}

	void InflateRect(SIZE size)
	{
		::InflateRect(this, size.cx, size.cy);
	}

	void InflateRect(LPCRECT lpRect)
	{
		left -= lpRect->left;
		top -= lpRect->top;
		right += lpRect->right;
		bottom += lpRect->bottom;
	}

	void InflateRect(int l, int t, int r, int b)
	{
		left -= l;
		top -= t;
		right += r;
		bottom += b;
	}

	void DeflateRect(int x, int y)
	{
		::InflateRect(this, -x, -y);
	}

	void DeflateRect(SIZE size)
	{
		::InflateRect(this, -size.cx, -size.cy);
	}

	void DeflateRect(LPCRECT lpRect)
	{
		left += lpRect->left;
		top += lpRect->top;
		right -= lpRect->right;
		bottom -= lpRect->bottom;
	}

	void DeflateRect(int l, int t, int r, int b)
	{
		left += l;
		top += t;
		right -= r;
		bottom -= b;
	}

	void OffsetRect(int x, int y)
	{
		::OffsetRect(this, x, y);
	}
	void OffsetRect(SIZE size)
	{
		::OffsetRect(this, size.cx, size.cy);
	}

	void OffsetRect(POINT point)
	{
		::OffsetRect(this, point.x, point.y);
	}

	void NormalizeRect()
	{
		int nTemp;
		if (left > right)
		{
			nTemp = left;
			left = right;
			right = nTemp;
		}
		if (top > bottom)
		{
			nTemp = top;
			top = bottom;
			bottom = nTemp;
		}
	}

	// absolute position of rectangle
	void MoveToY(int y)
	{
		bottom = Height() + y;
		top = y;
	}

	void MoveToX(int x)
	{
		right = Width() + x;
		left = x;
	}

	void MoveToXY(int x, int y)
	{
		MoveToX(x);
		MoveToY(y);
	}

	void MoveToXY(POINT pt)
	{
		MoveToX(pt.x);
		MoveToY(pt.y);
	}

	// operations that fill '*this' with result
	BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{
		return ::IntersectRect(this, lpRect1, lpRect2);
	}

	BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{
		return ::UnionRect(this, lpRect1, lpRect2);
	}

	BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
	{
		return ::SubtractRect(this, lpRectSrc1, lpRectSrc2);
	}

// Additional Operations
	void operator =(const RECT& srcRect)
	{
		::CopyRect(this, &srcRect);
	}

	BOOL operator ==(const RECT& rect) const
	{
		return ::EqualRect(this, &rect);
	}

	BOOL operator !=(const RECT& rect) const
	{
		return !::EqualRect(this, &rect);
	}

	void operator +=(POINT point)
	{
		::OffsetRect(this, point.x, point.y);
	}

	void operator +=(SIZE size)
	{
		::OffsetRect(this, size.cx, size.cy);
	}

	void operator +=(LPCRECT lpRect)
	{
		InflateRect(lpRect);
	}

	void operator -=(POINT point)
	{
		::OffsetRect(this, -point.x, -point.y);
	}

	void operator -=(SIZE size)
	{
		::OffsetRect(this, -size.cx, -size.cy);
	}

	void operator -=(LPCRECT lpRect)
	{
		DeflateRect(lpRect);
	}

	void operator &=(const RECT& rect)
	{
		::IntersectRect(this, this, &rect);
	}

	void operator |=(const RECT& rect)
	{
		::UnionRect(this, this, &rect);
	}

// Operators returning CRect values
	CRect operator +(POINT pt) const
	{
		CRect rect(*this);
		::OffsetRect(&rect, pt.x, pt.y);
		return rect;
	}

	CRect operator -(POINT pt) const
	{
		CRect rect(*this);
		::OffsetRect(&rect, -pt.x, -pt.y);
		return rect;
	}

	CRect operator +(LPCRECT lpRect) const
	{
		CRect rect(this);
		rect.InflateRect(lpRect);
		return rect;
	}

	CRect operator +(SIZE size) const
	{
		CRect rect(*this);
		::OffsetRect(&rect, size.cx, size.cy);
		return rect;
	}

	CRect operator -(SIZE size) const
	{
		CRect rect(*this);
		::OffsetRect(&rect, -size.cx, -size.cy);
		return rect;
	}

	CRect operator -(LPCRECT lpRect) const
	{
		CRect rect(this);
		rect.DeflateRect(lpRect);
		return rect;
	}

	CRect operator &(const RECT& rect2) const
	{
		CRect rect;
		::IntersectRect(&rect, this, &rect2);
		return rect;
	}

	CRect operator |(const RECT& rect2) const
	{
		CRect rect;
		::UnionRect(&rect, this, &rect2);
		return rect;
	}

	CRect MulDiv(int nMultiplier, int nDivisor) const
	{
		return CRect(
			::MulDiv(left, nMultiplier, nDivisor),
			::MulDiv(top, nMultiplier, nDivisor),
			::MulDiv(right, nMultiplier, nDivisor),
			::MulDiv(bottom, nMultiplier, nDivisor));
	}
};


// CSize implementation

inline CPoint CSize::operator +(POINT point) const
{ return CPoint(cx + point.x, cy + point.y); }

inline CPoint CSize::operator -(POINT point) const
{ return CPoint(cx - point.x, cy - point.y); }

inline CRect CSize::operator +(const RECT* lpRect) const
{ return CRect(lpRect) + *this; }

inline CRect CSize::operator -(const RECT* lpRect) const
{ return CRect(lpRect) - *this; }


// CPoint implementation

inline CRect CPoint::operator +(const RECT* lpRect) const
{ return CRect(lpRect) + *this; }

inline CRect CPoint::operator -(const RECT* lpRect) const
{ return CRect(lpRect) - *this; }

#endif // !_WTL_NO_WTYPES


// WTL::CSize or ATL::CSize scalar operators 

#if !defined(_WTL_NO_SIZE_SCALAR) && (!defined(_WTL_NO_WTYPES) || defined(__ATLTYPES_H__))

template <class Num>
inline CSize operator *(SIZE s, Num n) 
{
	return CSize((int)(s.cx * n), (int)(s.cy * n));
};

template <class Num>
inline void operator *=(SIZE & s, Num n)
{
	s = s * n;
};	

template <class Num>
inline CSize operator /(SIZE s, Num n) 
{
	return CSize((int)(s.cx / n), (int)(s.cy / n));
};

template <class Num>
inline void operator /=(SIZE & s, Num n)
{
	s = s / n;
};	

#endif // !defined(_WTL_NO_SIZE_SCALAR) && (!defined(_WTL_NO_WTYPES) || defined(__ATLTYPES_H__))


///////////////////////////////////////////////////////////////////////////////
// CString - String class

#ifndef _WTL_NO_CSTRING

struct CStringData
{
	long nRefs;     // reference count
	int nDataLength;
	int nAllocLength;
	// TCHAR data[nAllocLength]

	TCHAR* data()
	{ return (TCHAR*)(this + 1); }
};

// Globals

// For an empty string, m_pchData will point here
// (note: avoids special case of checking for NULL m_pchData)
// empty string data (and locked)
_declspec(selectany) int rgInitData[] = { -1, 0, 0, 0 };
_declspec(selectany) CStringData* _atltmpDataNil = (CStringData*)&rgInitData;
_declspec(selectany) LPCTSTR _atltmpPchNil = (LPCTSTR)(((BYTE*)&rgInitData) + sizeof(CStringData));


class CString
{
public:
// Constructors
	CString()
	{
		Init();
	}

	CString(const CString& stringSrc)
	{
		ATLASSERT(stringSrc.GetData()->nRefs != 0);
		if (stringSrc.GetData()->nRefs >= 0)
		{
			ATLASSERT(stringSrc.GetData() != _atltmpDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
		else
		{
			Init();
			*this = stringSrc.m_pchData;
		}
	}

	CString(TCHAR ch, int nRepeat = 1)
	{
		ATLASSERT(!_istlead(ch));   // can't create a lead byte string
		Init();
		if (nRepeat >= 1)
		{
			if(AllocBuffer(nRepeat))
			{
#ifdef _UNICODE
				for (int i = 0; i < nRepeat; i++)
					m_pchData[i] = ch;
#else
				memset(m_pchData, ch, nRepeat);
#endif
			}
		}
	}

	CString(LPCTSTR lpsz)
	{
		Init();
		if (lpsz != NULL && HIWORD(lpsz) == NULL)
		{
			UINT nID = LOWORD((DWORD_PTR)lpsz);
			if (!LoadString(nID))
				ATLTRACE2(atlTraceUI, 0, _T("Warning: implicit LoadString(%u) in CString failed\n"), nID);
		}
		else
		{
			int nLen = SafeStrlen(lpsz);
			if (nLen != 0)
			{
				if(AllocBuffer(nLen))
					SecureHelper::memcpy_x(m_pchData, (nLen + 1) * sizeof(TCHAR), lpsz, nLen * sizeof(TCHAR));
			}
		}
	}

#ifdef _UNICODE
	CString(LPCSTR lpsz)
	{
		Init();
#if defined(_WIN32_WCE) && (_ATL_VER >= 0x0800)
		int nSrcLen = (lpsz != NULL) ? ATL::lstrlenA(lpsz) : 0;
#else
		int nSrcLen = (lpsz != NULL) ? lstrlenA(lpsz) : 0;
#endif
		if (nSrcLen != 0)
		{
			if(AllocBuffer(nSrcLen))
			{
				_mbstowcsz(m_pchData, lpsz, nSrcLen + 1);
				ReleaseBuffer();
			}
		}
	}
#else // !_UNICODE
	CString(LPCWSTR lpsz)
	{
		Init();
		int nSrcLen = (lpsz != NULL) ? (int)wcslen(lpsz) : 0;
		if (nSrcLen != 0)
		{
			if(AllocBuffer(nSrcLen * 2))
			{
				_wcstombsz(m_pchData, lpsz, (nSrcLen * 2) + 1);
				ReleaseBuffer();
			}
		}
	}
#endif // !_UNICODE

	CString(LPCTSTR lpch, int nLength)
	{
		Init();
		if (nLength != 0)
		{
			if(AllocBuffer(nLength))
				SecureHelper::memcpy_x(m_pchData, (nLength + 1) * sizeof(TCHAR), lpch, nLength * sizeof(TCHAR));
		}
	}

#ifdef _UNICODE
	CString(LPCSTR lpsz, int nLength)
	{
		Init();
		if (nLength != 0)
		{
			if(AllocBuffer(nLength))
			{
				int n = ::MultiByteToWideChar(CP_ACP, 0, lpsz, nLength, m_pchData, nLength + 1);
				ReleaseBuffer((n >= 0) ? n : -1);
			}
		}
	}
#else // !_UNICODE
	CString(LPCWSTR lpsz, int nLength)
	{
		Init();
		if (nLength != 0)
		{
			if(((nLength * 2) > nLength) && AllocBuffer(nLength * 2))
			{
				int n = ::WideCharToMultiByte(CP_ACP, 0, lpsz, nLength, m_pchData, (nLength * 2) + 1, NULL, NULL);
				ReleaseBuffer((n >= 0) ? n : -1);
			}
		}
	}
#endif // !_UNICODE

	CString(const unsigned char* lpsz)
	{
		Init();
		*this = (LPCSTR)lpsz;
	}

// Attributes & Operations
	int GetLength() const   // as an array of characters
	{
		return GetData()->nDataLength;
	}

	BOOL IsEmpty() const
	{
		return GetData()->nDataLength == 0;
	}

	void Empty()   // free up the data
	{
		if (GetData()->nDataLength == 0)
			return;

		if (GetData()->nRefs >= 0)
			Release();
		else
			*this = _T("");

		ATLASSERT(GetData()->nDataLength == 0);
		ATLASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
	}

	TCHAR GetAt(int nIndex) const   // 0 based
	{
		ATLASSERT(nIndex >= 0);
		ATLASSERT(nIndex < GetData()->nDataLength);
		return m_pchData[nIndex];
	}

	TCHAR operator [](int nIndex) const   // same as GetAt
	{
		// same as GetAt
		ATLASSERT(nIndex >= 0);
		ATLASSERT(nIndex < GetData()->nDataLength);
		return m_pchData[nIndex];
	}

	void SetAt(int nIndex, TCHAR ch)
	{
		ATLASSERT(nIndex >= 0);
		ATLASSERT(nIndex < GetData()->nDataLength);

		CopyBeforeWrite();
		m_pchData[nIndex] = ch;
	}

	operator LPCTSTR() const   // as a C string
	{
		return m_pchData;
	}

	// overloaded assignment
	CString& operator =(const CString& stringSrc)
	{
		if (m_pchData != stringSrc.m_pchData)
		{
			if ((GetData()->nRefs < 0 && GetData() != _atltmpDataNil) || stringSrc.GetData()->nRefs < 0)
			{
				// actual copy necessary since one of the strings is locked
				AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
			}
			else
			{
				// can just copy references around
				Release();
				ATLASSERT(stringSrc.GetData() != _atltmpDataNil);
				m_pchData = stringSrc.m_pchData;
				InterlockedIncrement(&GetData()->nRefs);
			}
		}
		return *this;
	}

	CString& operator =(TCHAR ch)
	{
		ATLASSERT(!_istlead(ch));   // can't set single lead byte
		AssignCopy(1, &ch);
		return *this;
	}

#ifdef _UNICODE
	CString& operator =(char ch)
	{
		*this = (TCHAR)ch;
		return *this;
	}
#endif

	CString& operator =(LPCTSTR lpsz)
	{
		ATLASSERT(lpsz == NULL || _IsValidString(lpsz));
		AssignCopy(SafeStrlen(lpsz), lpsz);
		return *this;
	}

#ifdef _UNICODE
	CString& operator =(LPCSTR lpsz)
	{
#if defined(_WIN32_WCE) && (_ATL_VER >= 0x0800)
		int nSrcLen = (lpsz != NULL) ? ATL::lstrlenA(lpsz) : 0;
#else
		int nSrcLen = (lpsz != NULL) ? lstrlenA(lpsz) : 0;
#endif
		if(AllocBeforeWrite(nSrcLen))
		{
			_mbstowcsz(m_pchData, lpsz, nSrcLen + 1);
			ReleaseBuffer();
		}
		return *this;
	}
#else // !_UNICODE
	CString& operator =(LPCWSTR lpsz)
	{
		int nSrcLen = (lpsz != NULL) ? (int)wcslen(lpsz) : 0;
		if(AllocBeforeWrite(nSrcLen * 2))
		{
			_wcstombsz(m_pchData, lpsz, (nSrcLen * 2) + 1);
			ReleaseBuffer();
		}
		return *this;
	}
#endif  // !_UNICODE

	CString& operator =(const unsigned char* lpsz)
	{
		*this = (LPCSTR)lpsz;
		return *this;
	}

	// string concatenation
	CString& operator +=(const CString& string)
	{
		ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
		return *this;
	}

	CString& operator +=(TCHAR ch)
	{
		ConcatInPlace(1, &ch);
		return *this;
	}

#ifdef _UNICODE
	CString& operator +=(char ch)
	{
		*this += (TCHAR)ch;
		return *this;
	}
#endif

	CString& operator +=(LPCTSTR lpsz)
	{
		ATLASSERT(lpsz == NULL || _IsValidString(lpsz));
		ConcatInPlace(SafeStrlen(lpsz), lpsz);
		return *this;
	}

	friend CString __stdcall operator +(const CString& string1, const CString& string2);
	friend CString __stdcall operator +(const CString& string, TCHAR ch);
	friend CString __stdcall operator +(TCHAR ch, const CString& string);
#ifdef _UNICODE
	friend CString __stdcall operator +(const CString& string, char ch);
	friend CString __stdcall operator +(char ch, const CString& string);
#endif
	friend CString __stdcall operator +(const CString& string, LPCTSTR lpsz);
	friend CString __stdcall operator +(LPCTSTR lpsz, const CString& string);

	// string comparison
	int Compare(LPCTSTR lpsz) const   // straight character (MBCS/Unicode aware)
	{
		return _cstrcmp(m_pchData, lpsz);
	}

	int CompareNoCase(LPCTSTR lpsz) const   // ignore case (MBCS/Unicode aware)
	{
		return _cstrcmpi(m_pchData, lpsz);
	}

#ifndef _WIN32_WCE
	// CString::Collate is often slower than Compare but is MBSC/Unicode
	//  aware as well as locale-sensitive with respect to sort order.
	int Collate(LPCTSTR lpsz) const   // NLS aware
	{
		return _cstrcoll(m_pchData, lpsz);
	}

	int CollateNoCase(LPCTSTR lpsz) const   // ignore case
	{
		return _cstrcolli(m_pchData, lpsz);
	}
#endif // !_WIN32_WCE

	// simple sub-string extraction
	CString Mid(int nFirst, int nCount) const
	{
		// out-of-bounds requests return sensible things
		if (nFirst < 0)
			nFirst = 0;
		if (nCount < 0)
			nCount = 0;

		if (nFirst + nCount > GetData()->nDataLength)
			nCount = GetData()->nDataLength - nFirst;
		if (nFirst > GetData()->nDataLength)
			nCount = 0;

		CString dest;
		AllocCopy(dest, nCount, nFirst, 0);
		return dest;
	}

	CString Mid(int nFirst) const
	{
		return Mid(nFirst, GetData()->nDataLength - nFirst);
	}

	CString Left(int nCount) const
	{
		if (nCount < 0)
			nCount = 0;
		else if (nCount > GetData()->nDataLength)
			nCount = GetData()->nDataLength;

		CString dest;
		AllocCopy(dest, nCount, 0, 0);
		return dest;
	}

	CString Right(int nCount) const
	{
		if (nCount < 0)
			nCount = 0;
		else if (nCount > GetData()->nDataLength)
			nCount = GetData()->nDataLength;

		CString dest;
		AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
		return dest;
	}

	CString SpanIncluding(LPCTSTR lpszCharSet) const   // strspn equivalent
	{
		ATLASSERT(_IsValidString(lpszCharSet));
		return Left(_cstrspn(m_pchData, lpszCharSet));
	}

	CString SpanExcluding(LPCTSTR lpszCharSet) const   // strcspn equivalent
	{
		ATLASSERT(_IsValidString(lpszCharSet));
		return Left(_cstrcspn(m_pchData, lpszCharSet));
	}

	// upper/lower/reverse conversion
	void MakeUpper()
	{
		CopyBeforeWrite();
		CharUpper(m_pchData);
	}

	void MakeLower()
	{
		CopyBeforeWrite();
		CharLower(m_pchData);
	}

	void MakeReverse()
	{
		CopyBeforeWrite();
		_cstrrev(m_pchData);
	}

	// trimming whitespace (either side)
	void TrimRight()
	{
		CopyBeforeWrite();

		// find beginning of trailing spaces by starting at beginning (DBCS aware)
		LPTSTR lpsz = m_pchData;
		LPTSTR lpszLast = NULL;
		while (*lpsz != _T('\0'))
		{
			if (_cstrisspace(*lpsz))
			{
				if (lpszLast == NULL)
					lpszLast = lpsz;
			}
			else
			{
				lpszLast = NULL;
			}
			lpsz = ::CharNext(lpsz);
		}

		if (lpszLast != NULL)
		{
			// truncate at trailing space start
			*lpszLast = _T('\0');
			GetData()->nDataLength = (int)(DWORD_PTR)(lpszLast - m_pchData);
		}
	}

	void TrimLeft()
	{
		CopyBeforeWrite();

		// find first non-space character
		LPCTSTR lpsz = m_pchData;
		while (_cstrisspace(*lpsz))
			lpsz = ::CharNext(lpsz);

		// fix up data and length
		int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
		SecureHelper::memmove_x(m_pchData, (GetData()->nAllocLength + 1) * sizeof(TCHAR), lpsz, (nDataLength + 1) * sizeof(TCHAR));
		GetData()->nDataLength = nDataLength;
	}

	// remove continuous occurrences of chTarget starting from right
	void TrimRight(TCHAR chTarget)
	{
		// find beginning of trailing matches
		// by starting at beginning (DBCS aware)

		CopyBeforeWrite();
		LPTSTR lpsz = m_pchData;
		LPTSTR lpszLast = NULL;

		while (*lpsz != _T('\0'))
		{
			if (*lpsz == chTarget)
			{
				if (lpszLast == NULL)
					lpszLast = lpsz;
			}
			else
				lpszLast = NULL;
			lpsz = ::CharNext(lpsz);
		}

		if (lpszLast != NULL)
		{
			// truncate at left-most matching character
			*lpszLast = _T('\0');
			GetData()->nDataLength = (int)(DWORD_PTR)(lpszLast - m_pchData);
		}
	}

	// remove continuous occcurrences of characters in passed string, starting from right
	void TrimRight(LPCTSTR lpszTargetList)
	{
		// find beginning of trailing matches by starting at beginning (DBCS aware)

		CopyBeforeWrite();
		LPTSTR lpsz = m_pchData;
		LPTSTR lpszLast = NULL;

		while (*lpsz != _T('\0'))
		{
			TCHAR* pNext = ::CharNext(lpsz);
			if(pNext > lpsz + 1)
			{
				if (_cstrchr_db(lpszTargetList, *lpsz, *(lpsz + 1)) != NULL)
				{
					if (lpszLast == NULL)
						lpszLast = lpsz;
				}
				else
				{
					lpszLast = NULL;
				}
			}
			else
			{
				if (_cstrchr(lpszTargetList, *lpsz) != NULL)
				{
					if (lpszLast == NULL)
						lpszLast = lpsz;
				}
				else
				{
					lpszLast = NULL;
				}
			}

			lpsz = pNext;
		}

		if (lpszLast != NULL)
		{
			// truncate at left-most matching character
			*lpszLast = _T('\0');
			GetData()->nDataLength = (int)(DWORD_PTR)(lpszLast - m_pchData);
		}
	}

	// remove continuous occurrences of chTarget starting from left
	void TrimLeft(TCHAR chTarget)
	{
		// find first non-matching character

		CopyBeforeWrite();
		LPCTSTR lpsz = m_pchData;

		while (chTarget == *lpsz)
			lpsz = ::CharNext(lpsz);

		if (lpsz != m_pchData)
		{
			// fix up data and length
			int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
			SecureHelper::memmove_x(m_pchData, (GetData()->nAllocLength + 1) * sizeof(TCHAR), lpsz, (nDataLength + 1) * sizeof(TCHAR));
			GetData()->nDataLength = nDataLength;
		}
	}

	// remove continuous occcurrences of characters in passed string, starting from left
	void TrimLeft(LPCTSTR lpszTargets)
	{
		// if we're not trimming anything, we're not doing any work
		if (SafeStrlen(lpszTargets) == 0)
			return;

		CopyBeforeWrite();
		LPCTSTR lpsz = m_pchData;

		while (*lpsz != _T('\0'))
		{
			TCHAR* pNext = ::CharNext(lpsz);
			if(pNext > lpsz + 1)
			{
				if (_cstrchr_db(lpszTargets, *lpsz, *(lpsz + 1)) == NULL)
					break;
			}
			else
			{
				if (_cstrchr(lpszTargets, *lpsz) == NULL)
					break;
			}
			lpsz = pNext;
		}

		if (lpsz != m_pchData)
		{
			// fix up data and length
			int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
			SecureHelper::memmove_x(m_pchData, (GetData()->nAllocLength + 1) * sizeof(TCHAR), lpsz, (nDataLength + 1) * sizeof(TCHAR));
			GetData()->nDataLength = nDataLength;
		}
	}

	// advanced manipulation
	// replace occurrences of chOld with chNew
	int Replace(TCHAR chOld, TCHAR chNew)
	{
		int nCount = 0;

		// short-circuit the nop case
		if (chOld != chNew)
		{
			// otherwise modify each character that matches in the string
			CopyBeforeWrite();
			LPTSTR psz = m_pchData;
			LPTSTR pszEnd = psz + GetData()->nDataLength;
			while (psz < pszEnd)
			{
				// replace instances of the specified character only
				if (*psz == chOld)
				{
					*psz = chNew;
					nCount++;
				}
				psz = ::CharNext(psz);
			}
		}
		return nCount;
	}

	// replace occurrences of substring lpszOld with lpszNew;
	// empty lpszNew removes instances of lpszOld
	int Replace(LPCTSTR lpszOld, LPCTSTR lpszNew)
	{
		// can't have empty or NULL lpszOld

		int nSourceLen = SafeStrlen(lpszOld);
		if (nSourceLen == 0)
			return 0;
		int nReplacementLen = SafeStrlen(lpszNew);

		// loop once to figure out the size of the result string
		int nCount = 0;
		LPTSTR lpszStart = m_pchData;
		LPTSTR lpszEnd = m_pchData + GetData()->nDataLength;
		LPTSTR lpszTarget = NULL;
		while (lpszStart < lpszEnd)
		{
			while ((lpszTarget = (TCHAR*)_cstrstr(lpszStart, lpszOld)) != NULL)
			{
				nCount++;
				lpszStart = lpszTarget + nSourceLen;
			}
			lpszStart += lstrlen(lpszStart) + 1;
		}

		// if any changes were made, make them
		if (nCount > 0)
		{
			CopyBeforeWrite();

			// if the buffer is too small, just allocate a new buffer (slow but sure)
			int nOldLength = GetData()->nDataLength;
			int nNewLength =  nOldLength + (nReplacementLen - nSourceLen) * nCount;
			if (GetData()->nAllocLength < nNewLength || GetData()->nRefs > 1)
			{
				CStringData* pOldData = GetData();
				LPTSTR pstr = m_pchData;
				if(!AllocBuffer(nNewLength))
					return -1;
				SecureHelper::memcpy_x(m_pchData, (nNewLength + 1) * sizeof(TCHAR), pstr, pOldData->nDataLength * sizeof(TCHAR));
				CString::Release(pOldData);
			}
			// else, we just do it in-place
			lpszStart = m_pchData;
			lpszEnd = m_pchData + GetData()->nDataLength;

			// loop again to actually do the work
			while (lpszStart < lpszEnd)
			{
				while ((lpszTarget = (TCHAR*)_cstrstr(lpszStart, lpszOld)) != NULL)
				{
					int nBalance = nOldLength - ((int)(DWORD_PTR)(lpszTarget - m_pchData) + nSourceLen);
					int cchBuffLen = GetData()->nAllocLength - (int)(DWORD_PTR)(lpszTarget - m_pchData);
					SecureHelper::memmove_x(lpszTarget + nReplacementLen, (cchBuffLen - nReplacementLen + 1) * sizeof(TCHAR), lpszTarget + nSourceLen, nBalance * sizeof(TCHAR));
					SecureHelper::memcpy_x(lpszTarget, (cchBuffLen + 1) * sizeof(TCHAR), lpszNew, nReplacementLen * sizeof(TCHAR));
					lpszStart = lpszTarget + nReplacementLen;
					lpszStart[nBalance] = _T('\0');
					nOldLength += (nReplacementLen - nSourceLen);
				}
				lpszStart += lstrlen(lpszStart) + 1;
			}
			ATLASSERT(m_pchData[nNewLength] == _T('\0'));
			GetData()->nDataLength = nNewLength;
		}

		return nCount;
	}

	// remove occurrences of chRemove
	int Remove(TCHAR chRemove)
	{
		CopyBeforeWrite();

		LPTSTR pstrSource = m_pchData;
		LPTSTR pstrDest = m_pchData;
		LPTSTR pstrEnd = m_pchData + GetData()->nDataLength;

		while (pstrSource < pstrEnd)
		{
			if (*pstrSource != chRemove)
			{
				*pstrDest = *pstrSource;
				pstrDest = ::CharNext(pstrDest);
			}
			pstrSource = ::CharNext(pstrSource);
		}
		*pstrDest = _T('\0');
		int nCount = (int)(DWORD_PTR)(pstrSource - pstrDest);
		GetData()->nDataLength -= nCount;

		return nCount;
	}

	// insert character at zero-based index; concatenates if index is past end of string
	int Insert(int nIndex, TCHAR ch)
	{
		CopyBeforeWrite();

		if (nIndex < 0)
			nIndex = 0;

		int nNewLength = GetData()->nDataLength;
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength++;

		if (GetData()->nAllocLength < nNewLength)
		{
			CStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			if(!AllocBuffer(nNewLength))
				return -1;
			SecureHelper::memcpy_x(m_pchData, (nNewLength + 1) * sizeof(TCHAR), pstr, (pOldData->nDataLength + 1) * sizeof(TCHAR));
			CString::Release(pOldData);
		}

		// move existing bytes down
		SecureHelper::memmove_x(m_pchData + nIndex + 1, (GetData()->nAllocLength - nIndex) * sizeof(TCHAR), m_pchData + nIndex, (nNewLength - nIndex) * sizeof(TCHAR));
		m_pchData[nIndex] = ch;
		GetData()->nDataLength = nNewLength;

		return nNewLength;
	}

	// insert substring at zero-based index; concatenates if index is past end of string
	int Insert(int nIndex, LPCTSTR pstr)
	{
		if (nIndex < 0)
			nIndex = 0;

		int nInsertLength = SafeStrlen(pstr);
		int nNewLength = GetData()->nDataLength;
		if (nInsertLength > 0)
		{
			CopyBeforeWrite();
			if (nIndex > nNewLength)
				nIndex = nNewLength;
			nNewLength += nInsertLength;

			if (GetData()->nAllocLength < nNewLength)
			{
				CStringData* pOldData = GetData();
				LPTSTR pstr = m_pchData;
				if(!AllocBuffer(nNewLength))
					return -1;
				SecureHelper::memcpy_x(m_pchData, (nNewLength + 1) * sizeof(TCHAR), pstr, (pOldData->nDataLength + 1) * sizeof(TCHAR));
				CString::Release(pOldData);
			}

			// move existing bytes down
			SecureHelper::memmove_x(m_pchData + nIndex + nInsertLength, (GetData()->nAllocLength + 1 - nIndex - nInsertLength) * sizeof(TCHAR), m_pchData + nIndex, (nNewLength - nIndex - nInsertLength + 1) * sizeof(TCHAR));
			SecureHelper::memcpy_x(m_pchData + nIndex, (GetData()->nAllocLength + 1 - nIndex) * sizeof(TCHAR), pstr, nInsertLength * sizeof(TCHAR));
			GetData()->nDataLength = nNewLength;
		}

		return nNewLength;
	}

	// delete nCount characters starting at zero-based index
	int Delete(int nIndex, int nCount = 1)
	{
		if (nIndex < 0)
			nIndex = 0;
		int nLength = GetData()->nDataLength;
		if (nCount > 0 && nIndex < nLength)
		{
			if((nIndex + nCount) > nLength)
				nCount = nLength - nIndex;
			CopyBeforeWrite();
			int nBytesToCopy = nLength - (nIndex + nCount) + 1;

			SecureHelper::memmove_x(m_pchData + nIndex, (GetData()->nAllocLength + 1 - nIndex) * sizeof(TCHAR), m_pchData + nIndex + nCount, nBytesToCopy * sizeof(TCHAR));
			nLength -= nCount;
			GetData()->nDataLength = nLength;
		}

		return nLength;
	}

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(TCHAR ch) const   // like "C" strchr
	{
		return Find(ch, 0);
	}

	int ReverseFind(TCHAR ch) const
	{
		// find last single character
		LPCTSTR lpsz = _cstrrchr(m_pchData, (_TUCHAR)ch);

		// return -1 if not found, distance from beginning otherwise
		return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
	}

	int Find(TCHAR ch, int nStart) const   // starting at index
	{
		int nLength = GetData()->nDataLength;
		if (nStart < 0 || nStart >= nLength)
			return -1;

		// find first single character
		LPCTSTR lpsz = _cstrchr(m_pchData + nStart, (_TUCHAR)ch);

		// return -1 if not found and index otherwise
		return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
	}

	int FindOneOf(LPCTSTR lpszCharSet) const
	{
		ATLASSERT(_IsValidString(lpszCharSet));
		LPCTSTR lpsz = _cstrpbrk(m_pchData, lpszCharSet);
		return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
	}

	// look for a specific sub-string
	// find a sub-string (like strstr)
	int Find(LPCTSTR lpszSub) const   // like "C" strstr
	{
		return Find(lpszSub, 0);
	}

	int Find(LPCTSTR lpszSub, int nStart) const   // starting at index
	{
		ATLASSERT(_IsValidString(lpszSub));

		int nLength = GetData()->nDataLength;
		if (nStart < 0 || nStart > nLength)
			return -1;

		// find first matching substring
		LPCTSTR lpsz = _cstrstr(m_pchData + nStart, lpszSub);

		// return -1 for not found, distance from beginning otherwise
		return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
	}

	// Concatentation for non strings
	CString& Append(int n)
	{
		const int cchBuff = 12;
		TCHAR szBuffer[cchBuff] = { 0 };
		SecureHelper::wsprintf_x(szBuffer, cchBuff, _T("%d"), n);
		ConcatInPlace(SafeStrlen(szBuffer), szBuffer);
		return *this;
	}

	// simple formatting
	// formatting (using wsprintf style formatting)
	BOOL __cdecl Format(LPCTSTR lpszFormat, ...)
	{
		ATLASSERT(_IsValidString(lpszFormat));

		va_list argList;
		va_start(argList, lpszFormat);
		BOOL bRet = FormatV(lpszFormat, argList);
		va_end(argList);
		return bRet;
	}

	BOOL __cdecl Format(UINT nFormatID, ...)
	{
		CString strFormat;
		BOOL bRet = strFormat.LoadString(nFormatID);
		ATLASSERT(bRet != 0);

		va_list argList;
		va_start(argList, nFormatID);
		bRet = FormatV(strFormat, argList);
		va_end(argList);
		return bRet;
	}

	BOOL FormatV(LPCTSTR lpszFormat, va_list argList)
	{
		ATLASSERT(_IsValidString(lpszFormat));

		enum _FormatModifiers
		{
			FORCE_ANSI =	0x10000,
			FORCE_UNICODE =	0x20000,
			FORCE_INT64 =	0x40000
		};

		va_list argListSave = argList;

		// make a guess at the maximum length of the resulting string
		int nMaxLen = 0;
		for (LPCTSTR lpsz = lpszFormat; *lpsz != _T('\0'); lpsz = ::CharNext(lpsz))
		{
			// handle '%' character, but watch out for '%%'
			if (*lpsz != _T('%') || *(lpsz = ::CharNext(lpsz)) == _T('%'))
			{
				nMaxLen += (int)(::CharNext(lpsz) - lpsz);
				continue;
			}

			int nItemLen = 0;

			// handle '%' character with format
			int nWidth = 0;
			for (; *lpsz != _T('\0'); lpsz = ::CharNext(lpsz))
			{
				// check for valid flags
				if (*lpsz == _T('#'))
					nMaxLen += 2;   // for '0x'
				else if (*lpsz == _T('*'))
					nWidth = va_arg(argList, int);
				else if (*lpsz == _T('-') || *lpsz == _T('+') || *lpsz == _T('0') || *lpsz == _T(' '))
					;
				else // hit non-flag character
					break;
			}
			// get width and skip it
			if (nWidth == 0)
			{
				// width indicated by
				nWidth = _cstrtoi(lpsz);
				for (; *lpsz != _T('\0') && _cstrisdigit(*lpsz); lpsz = ::CharNext(lpsz))
					;
			}
			ATLASSERT(nWidth >= 0);

			int nPrecision = 0;
			if (*lpsz == _T('.'))
			{
				// skip past '.' separator (width.precision)
				lpsz = ::CharNext(lpsz);

				// get precision and skip it
				if (*lpsz == _T('*'))
				{
					nPrecision = va_arg(argList, int);
					lpsz = ::CharNext(lpsz);
				}
				else
				{
					nPrecision = _cstrtoi(lpsz);
					for (; *lpsz != _T('\0') && _cstrisdigit(*lpsz); lpsz = ::CharNext(lpsz))
						;
				}
				ATLASSERT(nPrecision >= 0);
			}

			// should be on type modifier or specifier
			int nModifier = 0;
			if(lpsz[0] == _T('I') && lpsz[1] == _T('6') && lpsz[2] == _T('4'))
			{
				lpsz += 3;
				nModifier = FORCE_INT64;
			}
			else
			{
				switch (*lpsz)
				{
				// modifiers that affect size
				case _T('h'):
					nModifier = FORCE_ANSI;
					lpsz = ::CharNext(lpsz);
					break;
				case _T('l'):
					nModifier = FORCE_UNICODE;
					lpsz = ::CharNext(lpsz);
					break;

				// modifiers that do not affect size
				case _T('F'):
				case _T('N'):
				case _T('L'):
					lpsz = ::CharNext(lpsz);
					break;
				}
			}

			// now should be on specifier
			switch (*lpsz | nModifier)
			{
			// single characters
			case _T('c'):
			case _T('C'):
				nItemLen = 2;
				va_arg(argList, TCHAR);
				break;
			case _T('c') | FORCE_ANSI:
			case _T('C') | FORCE_ANSI:
				nItemLen = 2;
				va_arg(argList, char);
				break;
			case _T('c') | FORCE_UNICODE:
			case _T('C') | FORCE_UNICODE:
				nItemLen = 2;
				va_arg(argList, WCHAR);
				break;

			// strings
			case _T('s'):
			{
				LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
				if (pstrNextArg == NULL)
				{
					nItemLen = 6;  // "(null)"
				}
				else
				{
					nItemLen = lstrlen(pstrNextArg);
					nItemLen = max(1, nItemLen);
				}
				break;
			}

			case _T('S'):
			{
#ifndef _UNICODE
				LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
				if (pstrNextArg == NULL)
				{
					nItemLen = 6;  // "(null)"
				}
				else
				{
					nItemLen = (int)wcslen(pstrNextArg);
					nItemLen = max(1, nItemLen);
				}
#else // _UNICODE
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				{
					nItemLen = 6; // "(null)"
				}
				else
				{
#if defined(_WIN32_WCE) && (_ATL_VER >= 0x0800)
					nItemLen = ATL::lstrlenA(pstrNextArg);
#else
					nItemLen = lstrlenA(pstrNextArg);
#endif
					nItemLen = max(1, nItemLen);
				}
#endif // _UNICODE
				break;
			}

			case _T('s') | FORCE_ANSI:
			case _T('S') | FORCE_ANSI:
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				{
					nItemLen = 6; // "(null)"
				}
				else
				{
#if defined(_WIN32_WCE) && (_ATL_VER >= 0x0800)
					nItemLen = ATL::lstrlenA(pstrNextArg);
#else
					nItemLen = lstrlenA(pstrNextArg);
#endif
					nItemLen = max(1, nItemLen);
				}
				break;
			}

			case _T('s') | FORCE_UNICODE:
			case _T('S') | FORCE_UNICODE:
			{
				LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
				if (pstrNextArg == NULL)
				{
					nItemLen = 6; // "(null)"
				}
				else
				{
					nItemLen = (int)wcslen(pstrNextArg);
					nItemLen = max(1, nItemLen);
				}
				break;
			}
			}

			// adjust nItemLen for strings
			if (nItemLen != 0)
			{
				nItemLen = max(nItemLen, nWidth);
				if (nPrecision != 0)
					nItemLen = min(nItemLen, nPrecision);
			}
			else
			{
				switch (*lpsz)
				{
				// integers
				case _T('d'):
				case _T('i'):
				case _T('u'):
				case _T('x'):
				case _T('X'):
				case _T('o'):
					if (nModifier & FORCE_INT64)
						va_arg(argList, __int64);
					else
						va_arg(argList, int);
					nItemLen = 32;
					nItemLen = max(nItemLen, nWidth + nPrecision);
					break;

#ifndef _ATL_USE_CSTRING_FLOAT
				case _T('e'):
				case _T('E'):
				case _T('f'):
				case _T('g'):
				case _T('G'):
					ATLASSERT(!"Floating point (%%e, %%E, %%f, %%g, and %%G) is not supported by the WTL::CString class.");
#ifndef _DEBUG
					::OutputDebugString(_T("Floating point (%%e, %%f, %%g, and %%G) is not supported by the WTL::CString class."));
#ifndef _WIN32_WCE
					::DebugBreak();
#else // CE specific
					DebugBreak();
#endif // _WIN32_WCE
#endif // !_DEBUG
					break;
#else // _ATL_USE_CSTRING_FLOAT
				case _T('e'):
				case _T('E'):
				case _T('g'):
				case _T('G'):
					va_arg(argList, double);
					nItemLen = 128;
					nItemLen = max(nItemLen, nWidth + nPrecision);
					break;
				case _T('f'):
					{
						double f = va_arg(argList, double);
						// 312 == strlen("-1+(309 zeroes).")
						// 309 zeroes == max precision of a double
						// 6 == adjustment in case precision is not specified,
						//   which means that the precision defaults to 6
						int cchLen = max(nWidth, 312 + nPrecision + 6);
						CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
						LPTSTR pszTemp = buff.Allocate(cchLen);
						if(pszTemp != NULL)
						{
							SecureHelper::sprintf_x(pszTemp, cchLen, _T("%*.*f"), nWidth, nPrecision + 6, f);
							nItemLen = (int)_tcslen(pszTemp);
						}
						else
						{
							nItemLen = cchLen;
						}
					}
					break;
#endif // _ATL_USE_CSTRING_FLOAT

				case _T('p'):
					va_arg(argList, void*);
					nItemLen = 32;
					nItemLen = max(nItemLen, nWidth + nPrecision);
					break;

				// no output
				case _T('n'):
					va_arg(argList, int*);
					break;

				default:
					ATLASSERT(FALSE);  // unknown formatting option
				}
			}

			// adjust nMaxLen for output nItemLen
			nMaxLen += nItemLen;
		}

		if(GetBuffer(nMaxLen) == NULL)
			return FALSE;
#ifndef _ATL_USE_CSTRING_FLOAT
		int nRet = SecureHelper::wvsprintf_x(m_pchData, GetAllocLength() + 1, lpszFormat, argListSave);
#else // _ATL_USE_CSTRING_FLOAT
		int nRet = SecureHelper::vsprintf_x(m_pchData, GetAllocLength() + 1, lpszFormat, argListSave);
#endif // _ATL_USE_CSTRING_FLOAT
		nRet;   // ref
		ATLASSERT(nRet <= GetAllocLength());
		ReleaseBuffer();

		va_end(argListSave);
		return TRUE;
	}

	// formatting for localization (uses FormatMessage API)
	// formatting (using FormatMessage style formatting)
	BOOL __cdecl FormatMessage(LPCTSTR lpszFormat, ...)
	{
		// format message into temporary buffer lpszTemp
		va_list argList;
		va_start(argList, lpszFormat);
		LPTSTR lpszTemp;
		BOOL bRet = TRUE;

		if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
				lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL)
			bRet = FALSE;

		// assign lpszTemp into the resulting string and free the temporary
		*this = lpszTemp;
		LocalFree(lpszTemp);
		va_end(argList);
		return bRet;
	}

	BOOL __cdecl FormatMessage(UINT nFormatID, ...)
	{
		// get format string from string table
		CString strFormat;
		BOOL bRetTmp = strFormat.LoadString(nFormatID);
		bRetTmp;   // ref
		ATLASSERT(bRetTmp != 0);

		// format message into temporary buffer lpszTemp
		va_list argList;
		va_start(argList, nFormatID);
		LPTSTR lpszTemp;
		BOOL bRet = TRUE;

		if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
				strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL)
			bRet = FALSE;

		// assign lpszTemp into the resulting string and free lpszTemp
		*this = lpszTemp;
		LocalFree(lpszTemp);
		va_end(argList);
		return bRet;
	}

	// Windows support
	BOOL LoadString(UINT nID)   // load from string resource (255 chars max.)
	{
#ifdef _UNICODE
		const int CHAR_FUDGE = 1;   // one TCHAR unused is good enough
#else
		const int CHAR_FUDGE = 2;   // two BYTES unused for case of DBC last char
#endif

		// try fixed buffer first (to avoid wasting space in the heap)
		TCHAR szTemp[256];
		int nCount =  sizeof(szTemp) / sizeof(szTemp[0]);
		int nLen = _LoadString(nID, szTemp, nCount);
		if (nCount - nLen > CHAR_FUDGE)
		{
			*this = szTemp;
			return (nLen > 0);
		}

		// try buffer size of 512, then larger size until entire string is retrieved
		int nSize = 256;
		do
		{
			nSize += 256;
			LPTSTR lpstr = GetBuffer(nSize - 1);
			if(lpstr == NULL)
			{
				nLen = 0;
				break;
			}
			nLen = _LoadString(nID, lpstr, nSize);
		} while (nSize - nLen <= CHAR_FUDGE);
		ReleaseBuffer();

		return (nLen > 0);
	}

#ifndef _UNICODE
	// ANSI <-> OEM support (convert string in place)
	void AnsiToOem()
	{
		CopyBeforeWrite();
		::AnsiToOem(m_pchData, m_pchData);
	}

	void OemToAnsi()
	{
		CopyBeforeWrite();
		::OemToAnsi(m_pchData, m_pchData);
	}
#endif

#ifndef _ATL_NO_COM
	// OLE BSTR support (use for OLE automation)
	BSTR AllocSysString() const
	{
#if defined(_UNICODE) || defined(OLE2ANSI)
		BSTR bstr = ::SysAllocStringLen(m_pchData, GetData()->nDataLength);
#else
		int nLen = MultiByteToWideChar(CP_ACP, 0, m_pchData,
			GetData()->nDataLength, NULL, NULL);
		BSTR bstr = ::SysAllocStringLen(NULL, nLen);
		if(bstr != NULL)
			MultiByteToWideChar(CP_ACP, 0, m_pchData, GetData()->nDataLength, bstr, nLen);
#endif
		return bstr;
	}

	BSTR SetSysString(BSTR* pbstr) const
	{
#if defined(_UNICODE) || defined(OLE2ANSI)
		::SysReAllocStringLen(pbstr, m_pchData, GetData()->nDataLength);
#else
		int nLen = MultiByteToWideChar(CP_ACP, 0, m_pchData,
			GetData()->nDataLength, NULL, NULL);
		if(::SysReAllocStringLen(pbstr, NULL, nLen))
			MultiByteToWideChar(CP_ACP, 0, m_pchData, GetData()->nDataLength, *pbstr, nLen);
#endif
		ATLASSERT(*pbstr != NULL);
		return *pbstr;
	}
#endif // !_ATL_NO_COM

	// Access to string implementation buffer as "C" character array
	LPTSTR GetBuffer(int nMinBufLength)
	{
		ATLASSERT(nMinBufLength >= 0);

		if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
		{
			// we have to grow the buffer
			CStringData* pOldData = GetData();
			int nOldLen = GetData()->nDataLength;   // AllocBuffer will tromp it
			if (nMinBufLength < nOldLen)
				nMinBufLength = nOldLen;

			if(!AllocBuffer(nMinBufLength))
				return NULL;

			SecureHelper::memcpy_x(m_pchData, (nMinBufLength + 1) * sizeof(TCHAR), pOldData->data(), (nOldLen + 1) * sizeof(TCHAR));
			GetData()->nDataLength = nOldLen;
			CString::Release(pOldData);
		}
		ATLASSERT(GetData()->nRefs <= 1);

		// return a pointer to the character storage for this string
		ATLASSERT(m_pchData != NULL);
		return m_pchData;
	}

	void ReleaseBuffer(int nNewLength = -1)
	{
		CopyBeforeWrite();   // just in case GetBuffer was not called

		if (nNewLength == -1)
			nNewLength = lstrlen(m_pchData);   // zero terminated

		ATLASSERT(nNewLength <= GetData()->nAllocLength);
		GetData()->nDataLength = nNewLength;
		m_pchData[nNewLength] = _T('\0');
	}

	LPTSTR GetBufferSetLength(int nNewLength)
	{
		ATLASSERT(nNewLength >= 0);

		if(GetBuffer(nNewLength) == NULL)
			return NULL;

		GetData()->nDataLength = nNewLength;
		m_pchData[nNewLength] = _T('\0');
		return m_pchData;
	}

	void FreeExtra()
	{
		ATLASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
		if (GetData()->nDataLength != GetData()->nAllocLength)
		{
			CStringData* pOldData = GetData();
			if(AllocBuffer(GetData()->nDataLength))
			{
				SecureHelper::memcpy_x(m_pchData, (GetData()->nAllocLength + 1) * sizeof(TCHAR), pOldData->data(), pOldData->nDataLength * sizeof(TCHAR));
				ATLASSERT(m_pchData[GetData()->nDataLength] == _T('\0'));
				CString::Release(pOldData);
			}
		}
		ATLASSERT(GetData() != NULL);
	}

	// Use LockBuffer/UnlockBuffer to turn refcounting off
	LPTSTR LockBuffer()
	{
		LPTSTR lpsz = GetBuffer(0);
		if(lpsz != NULL)
			GetData()->nRefs = -1;
		return lpsz;
	}

	void UnlockBuffer()
	{
		ATLASSERT(GetData()->nRefs == -1);
		if (GetData() != _atltmpDataNil)
			GetData()->nRefs = 1;
	}

// Implementation
public:
	~CString()   //  free any attached data
	{
		if (GetData() != _atltmpDataNil)
		{
			if (InterlockedDecrement(&GetData()->nRefs) <= 0)
				delete[] (BYTE*)GetData();
		}
	}

	int GetAllocLength() const
	{
		return GetData()->nAllocLength;
	}

	static BOOL __stdcall _IsValidString(LPCTSTR lpsz, int /*nLength*/ = -1)
	{
		return (lpsz != NULL) ? TRUE : FALSE;
	}

protected:
	LPTSTR m_pchData;   // pointer to ref counted string data

	// implementation helpers
	CStringData* GetData() const
	{
		ATLASSERT(m_pchData != NULL);
		return ((CStringData*)m_pchData) - 1;
	}

	void Init()
	{
		m_pchData = _GetEmptyString().m_pchData;
	}

	BOOL AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const
	{
		// will clone the data attached to this string
		// allocating 'nExtraLen' characters
		// Places results in uninitialized string 'dest'
		// Will copy the part or all of original data to start of new string

		BOOL bRet = FALSE;
		int nNewLen = nCopyLen + nExtraLen;
		if (nNewLen == 0)
		{
			dest.Init();
			bRet = TRUE;
		}
		else if(nNewLen >= nCopyLen)
		{
			if(dest.AllocBuffer(nNewLen))
			{
				SecureHelper::memcpy_x(dest.m_pchData, (nNewLen + 1) * sizeof(TCHAR), m_pchData + nCopyIndex, nCopyLen * sizeof(TCHAR));
				bRet = TRUE;
			}
		}

		return bRet;
	}

	// always allocate one extra character for '\0' termination
	// assumes [optimistically] that data length will equal allocation length
	BOOL AllocBuffer(int nLen)
	{
		ATLASSERT(nLen >= 0);
		ATLASSERT(nLen <= INT_MAX - 1);   // max size (enough room for 1 extra)

		if (nLen == 0)
		{
			Init();
		}
		else
		{
			CStringData* pData = NULL;
			ATLTRY(pData = (CStringData*)new BYTE[sizeof(CStringData) + (nLen + 1) * sizeof(TCHAR)]);
			if(pData == NULL)
				return FALSE;

			pData->nRefs = 1;
			pData->data()[nLen] = _T('\0');
			pData->nDataLength = nLen;
			pData->nAllocLength = nLen;
			m_pchData = pData->data();
		}

		return TRUE;
	}

	// Assignment operators
	//  All assign a new value to the string
	//      (a) first see if the buffer is big enough
	//      (b) if enough room, copy on top of old buffer, set size and type
	//      (c) otherwise free old string data, and create a new one
	//
	//  All routines return the new string (but as a 'const CString&' so that
	//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
	//
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
	{
		if(AllocBeforeWrite(nSrcLen))
		{
			SecureHelper::memcpy_x(m_pchData, (nSrcLen + 1) * sizeof(TCHAR), lpszSrcData, nSrcLen * sizeof(TCHAR));
			GetData()->nDataLength = nSrcLen;
			m_pchData[nSrcLen] = _T('\0');
		}
	}

	// Concatenation
	// NOTE: "operator +" is done as friend functions for simplicity
	//      There are three variants:
	//          CString + CString
	// and for ? = TCHAR, LPCTSTR
	//          CString + ?
	//          ? + CString
	BOOL ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data)
	{
		// -- master concatenation routine
		// Concatenate two sources
		// -- assume that 'this' is a new CString object

		BOOL bRet = TRUE;
		int nNewLen = nSrc1Len + nSrc2Len;
		if(nNewLen < nSrc1Len || nNewLen < nSrc2Len)
		{
			bRet = FALSE;
		}
		else if(nNewLen != 0)
		{
			bRet = AllocBuffer(nNewLen);
			if (bRet)
			{
				SecureHelper::memcpy_x(m_pchData, (nNewLen + 1) * sizeof(TCHAR), lpszSrc1Data, nSrc1Len * sizeof(TCHAR));
				SecureHelper::memcpy_x(m_pchData + nSrc1Len, (nNewLen + 1 - nSrc1Len) * sizeof(TCHAR), lpszSrc2Data, nSrc2Len * sizeof(TCHAR));
			}
		}
		return bRet;
	}

	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
	{
		//  -- the main routine for += operators

		// concatenating an empty string is a no-op!
		if (nSrcLen == 0)
			return;

		// if the buffer is too small, or we have a width mis-match, just
		//   allocate a new buffer (slow but sure)
		if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
		{
			// we have to grow the buffer, use the ConcatCopy routine
			CStringData* pOldData = GetData();
			if (ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData))
			{
				ATLASSERT(pOldData != NULL);
				CString::Release(pOldData);
			}
		}
		else
		{
			// fast concatenation when buffer big enough
			SecureHelper::memcpy_x(m_pchData + GetData()->nDataLength, (GetData()->nAllocLength + 1) * sizeof(TCHAR), lpszSrcData, nSrcLen * sizeof(TCHAR));
			GetData()->nDataLength += nSrcLen;
			ATLASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
			m_pchData[GetData()->nDataLength] = _T('\0');
		}
	}

	void CopyBeforeWrite()
	{
		if (GetData()->nRefs > 1)
		{
			CStringData* pData = GetData();
			Release();
			if(AllocBuffer(pData->nDataLength))
				SecureHelper::memcpy_x(m_pchData, (GetData()->nAllocLength + 1) * sizeof(TCHAR), pData->data(), (pData->nDataLength + 1) * sizeof(TCHAR));
		}
		ATLASSERT(GetData()->nRefs <= 1);
	}

	BOOL AllocBeforeWrite(int nLen)
	{
		BOOL bRet = TRUE;
		if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
		{
			Release();
			bRet = AllocBuffer(nLen);
		}
		ATLASSERT(GetData()->nRefs <= 1);
		return bRet;
	}

	void Release()
	{
		if (GetData() != _atltmpDataNil)
		{
			ATLASSERT(GetData()->nRefs != 0);
			if (InterlockedDecrement(&GetData()->nRefs) <= 0)
				delete[] (BYTE*)GetData();
			Init();
		}
	}

	static void PASCAL Release(CStringData* pData)
	{
		if (pData != _atltmpDataNil)
		{
			ATLASSERT(pData->nRefs != 0);
			if (InterlockedDecrement(&pData->nRefs) <= 0)
				delete[] (BYTE*)pData;
		}
	}

	static int PASCAL SafeStrlen(LPCTSTR lpsz)
	{
		return (lpsz == NULL) ? 0 : lstrlen(lpsz);
	}

	static int __stdcall _LoadString(UINT nID, LPTSTR lpszBuf, UINT nMaxBuf)
	{
#ifdef _DEBUG
		// LoadString without annoying warning from the Debug kernel if the
		//  segment containing the string is not present
		if (::FindResource(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE((nID >> 4) + 1), RT_STRING) == NULL)
		{
			lpszBuf[0] = _T('\0');
			return 0;   // not found
		}
#endif // _DEBUG

		int nLen = ::LoadString(ModuleHelper::GetResourceInstance(), nID, lpszBuf, nMaxBuf);
		if (nLen == 0)
			lpszBuf[0] = _T('\0');

		return nLen;
	}

	static const CString& __stdcall _GetEmptyString()
	{
		return *(CString*)&_atltmpPchNil;
	}

// CString conversion helpers
	static int __cdecl _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
	{
		if (count == 0 && mbstr != NULL)
			return 0;

		int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1, mbstr, (int)count, NULL, NULL);
		ATLASSERT(mbstr == NULL || result <= (int)count);
		if (result > 0)
			mbstr[result - 1] = 0;
		return result;
	}

	static int __cdecl _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
	{
		if (count == 0 && wcstr != NULL)
			return 0;

		int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1, wcstr, (int)count);
		ATLASSERT(wcstr == NULL || result <= (int)count);
		if (result > 0)
			wcstr[result - 1] = 0;
		return result;
	}

// Helpers to avoid CRT startup code
#ifdef _ATL_MIN_CRT
	static const TCHAR* _cstrchr(const TCHAR* p, TCHAR ch)
	{
		// strchr for '\0' should succeed
		while (*p != 0)
		{
			if (*p == ch)
				break;
			p = ::CharNext(p);
		}
		return (*p == ch) ? p : NULL;
	}

	static TCHAR* _cstrrev(TCHAR* pStr)
	{
		// optimize NULL, zero-length, and single-char case
		if ((pStr == NULL) || (pStr[0] == _T('\0')) || (pStr[1] == _T('\0')))
			return pStr;

		TCHAR* p = pStr;

		while (*p != 0) 
		{
			TCHAR* pNext = ::CharNext(p);
			if(pNext > p + 1)
			{
				char p1 = *(char*)p;
				*(char*)p = *(char*)(p + 1);
				*(char*)(p + 1) = p1;
			}
			p = pNext;
		}

		p--;
		TCHAR* q = pStr;

		while (q < p)
		{
			TCHAR t = *q;
			*q = *p;
			*p = t;
			q++;
			p--;
		}
		return pStr;
	}

	static const TCHAR* _cstrstr(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		int nLen = lstrlen(pCharSet);
		if (nLen == 0)
			return (TCHAR*)pStr;

		const TCHAR* pRet = NULL;
		const TCHAR* pCur = pStr;
		while((pCur = _cstrchr(pCur, *pCharSet)) != NULL)
		{
			if(memcmp(pCur, pCharSet, nLen * sizeof(TCHAR)) == 0)
			{
				pRet = pCur;
				break;
			}
			pCur = ::CharNext(pCur);
		}
		return pRet;
	}

	static int _cstrspn(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		int nRet = 0;
		const TCHAR* p = pStr;
		while (*p != 0)
		{
			const TCHAR* pNext = ::CharNext(p);
			if(pNext > p + 1)
			{
				if(_cstrchr_db(pCharSet, *p, *(p + 1)) == NULL)
					break;
				nRet += 2;
			}
			else
			{
				if(_cstrchr(pCharSet, *p) == NULL)
					break;
				nRet++;
			}
			p = pNext;
		}
		return nRet;
	}

	static int _cstrcspn(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		int nRet = 0;
		TCHAR* p = (TCHAR*)pStr;
		while (*p != 0)
		{
			TCHAR* pNext = ::CharNext(p);
			if(pNext > p + 1)
			{
				if(_cstrchr_db(pCharSet, *p, *(p + 1)) != NULL)
					break;
				nRet += 2;
			}
			else
			{
				if(_cstrchr(pCharSet, *p) != NULL)
					break;
				nRet++;
			}
			p = pNext;
		}
		return nRet;
	}

	static const TCHAR* _cstrpbrk(const TCHAR* p, const TCHAR* lpszCharSet)
	{
		int n = _cstrcspn(p, lpszCharSet);
		return (p[n] != 0) ? &p[n] : NULL;
	}

	static int _cstrcmp(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return lstrcmp(pstrOne, pstrOther);
	}

	static int _cstrcmpi(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return lstrcmpi(pstrOne, pstrOther);
	}

	static int _cstrcoll(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		int nRet = CompareString(GetThreadLocale(), 0, pstrOne, -1, pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet - 2;   // convert to strcmp convention
	}

	static int _cstrcolli(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		int nRet = CompareString(GetThreadLocale(), NORM_IGNORECASE, pstrOne, -1, pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet - 2;   // convert to strcmp convention
	}
#else // !_ATL_MIN_CRT
	static const TCHAR* _cstrchr(const TCHAR* p, TCHAR ch)
	{
		return _tcschr(p, ch);
	}

	static TCHAR* _cstrrev(TCHAR* pStr)
	{
		return _tcsrev(pStr);
	}

	static const TCHAR* _cstrstr(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		return _tcsstr(pStr, pCharSet);
	}

	static int _cstrspn(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		return (int)_tcsspn(pStr, pCharSet);
	}

	static int _cstrcspn(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		return (int)_tcscspn(pStr, pCharSet);
	}

	static const TCHAR* _cstrpbrk(const TCHAR* p, const TCHAR* lpszCharSet)
	{
		return _tcspbrk(p, lpszCharSet);
	}

	static int _cstrcmp(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return _tcscmp(pstrOne, pstrOther);
	}

	static int _cstrcmpi(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return _tcsicmp(pstrOne, pstrOther);
	}

#ifndef _WIN32_WCE
	static int _cstrcoll(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return _tcscoll(pstrOne, pstrOther);
	}

	static int _cstrcolli(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return _tcsicoll(pstrOne, pstrOther);
	}
#endif // !_WIN32_WCE
#endif // !_ATL_MIN_CRT

	static const TCHAR* _cstrrchr(const TCHAR* p, TCHAR ch)
	{
		return MinCrtHelper::_strrchr(p, ch);
	}

	static int _cstrisdigit(TCHAR ch)
	{
		return MinCrtHelper::_isdigit(ch);
	}

	static int _cstrisspace(TCHAR ch)
	{
		return MinCrtHelper::_isspace(ch);
	}

	static int _cstrtoi(const TCHAR* nptr)
	{
		return MinCrtHelper::_atoi(nptr);
	}

	static const TCHAR* _cstrchr_db(const TCHAR* p, TCHAR ch1, TCHAR ch2)
	{
		const TCHAR* lpsz = NULL;
		while (*p != 0)
		{
			if (*p == ch1 && *(p + 1) == ch2)
			{
				lpsz = p;
				break;
			}
			p = ::CharNext(p);
		}
		return lpsz;
	}
};


// Compare helpers

inline bool __stdcall operator ==(const CString& s1, const CString& s2)
{ return s1.Compare(s2) == 0; }

inline bool __stdcall operator ==(const CString& s1, LPCTSTR s2)
{ return s1.Compare(s2) == 0; }

inline bool __stdcall operator ==(LPCTSTR s1, const CString& s2)
{ return s2.Compare(s1) == 0; }

inline bool __stdcall operator !=(const CString& s1, const CString& s2)
{ return s1.Compare(s2) != 0; }

inline bool __stdcall operator !=(const CString& s1, LPCTSTR s2)
{ return s1.Compare(s2) != 0; }

inline bool __stdcall operator !=(LPCTSTR s1, const CString& s2)
{ return s2.Compare(s1) != 0; }

inline bool __stdcall operator <(const CString& s1, const CString& s2)
{ return s1.Compare(s2) < 0; }

inline bool __stdcall operator <(const CString& s1, LPCTSTR s2)
{ return s1.Compare(s2) < 0; }

inline bool __stdcall operator <(LPCTSTR s1, const CString& s2)
{ return s2.Compare(s1) > 0; }

inline bool __stdcall operator >(const CString& s1, const CString& s2)
{ return s1.Compare(s2) > 0; }

inline bool __stdcall operator >(const CString& s1, LPCTSTR s2)
{ return s1.Compare(s2) > 0; }

inline bool __stdcall operator >(LPCTSTR s1, const CString& s2)
{ return s2.Compare(s1) < 0; }

inline bool __stdcall operator <=(const CString& s1, const CString& s2)
{ return s1.Compare(s2) <= 0; }

inline bool __stdcall operator <=(const CString& s1, LPCTSTR s2)
{ return s1.Compare(s2) <= 0; }

inline bool __stdcall operator <=(LPCTSTR s1, const CString& s2)
{ return s2.Compare(s1) >= 0; }

inline bool __stdcall operator >=(const CString& s1, const CString& s2)
{ return s1.Compare(s2) >= 0; }

inline bool __stdcall operator >=(const CString& s1, LPCTSTR s2)
{ return s1.Compare(s2) >= 0; }

inline bool __stdcall operator >=(LPCTSTR s1, const CString& s2)
{ return s2.Compare(s1) <= 0; }


// CString "operator +" functions

inline CString __stdcall operator +(const CString& string1, const CString& string2)
{
	CString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

inline CString __stdcall operator +(const CString& string, TCHAR ch)
{
	CString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData, 1, &ch);
	return s;
}

inline CString __stdcall operator +(TCHAR ch, const CString& string)
{
	CString s;
	s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

#ifdef _UNICODE
inline CString __stdcall operator +(const CString& string, char ch)
{
	return string + (TCHAR)ch;
}

inline CString __stdcall operator +(char ch, const CString& string)
{
	return (TCHAR)ch + string;
}
#endif // _UNICODE

inline CString __stdcall operator +(const CString& string, LPCTSTR lpsz)
{
	ATLASSERT(lpsz == NULL || CString::_IsValidString(lpsz));
	CString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData, CString::SafeStrlen(lpsz), lpsz);
	return s;
}

inline CString __stdcall operator +(LPCTSTR lpsz, const CString& string)
{
	ATLASSERT(lpsz == NULL || CString::_IsValidString(lpsz));
	CString s;
	s.ConcatCopy(CString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

#endif // !_WTL_NO_CSTRING


///////////////////////////////////////////////////////////////////////////////
// CRecentDocumentList - MRU List Support

#ifndef _WIN32_WCE

#ifndef _WTL_MRUEMPTY_TEXT
  #define _WTL_MRUEMPTY_TEXT	_T("(empty)")
#endif

// forward declaration
inline bool AtlCompactPath(LPTSTR lpstrOut, LPCTSTR lpstrIn, int cchLen);

template <class T, int t_cchItemLen = MAX_PATH, int t_nFirstID = ID_FILE_MRU_FIRST, int t_nLastID = ID_FILE_MRU_LAST>
class CRecentDocumentListBase
{
public:
// Declarations
	struct _DocEntry
	{
		TCHAR szDocName[t_cchItemLen];
		bool operator ==(const _DocEntry& de) const
		{ return (lstrcmpi(szDocName, de.szDocName) == 0); }
	};

	enum
	{
		m_nMaxEntries_Min = 2,
		m_nMaxEntries_Max = t_nLastID - t_nFirstID + 1,
		m_cchMaxItemLen_Min = 6,
		m_cchMaxItemLen_Max = t_cchItemLen,
		m_cchItemNameLen = 11
	};

// Data members
	ATL::CSimpleArray<_DocEntry> m_arrDocs;
	int m_nMaxEntries;   // default is 4
	HMENU m_hMenu;

	TCHAR m_szNoEntries[t_cchItemLen];

	int m_cchMaxItemLen;

// Constructor
	CRecentDocumentListBase() : m_hMenu(NULL), m_nMaxEntries(4), m_cchMaxItemLen(-1)
	{
		// These ASSERTs verify values of the template arguments
		ATLASSERT(t_cchItemLen > m_cchMaxItemLen_Min);
		ATLASSERT(m_nMaxEntries_Max > m_nMaxEntries_Min);
	}

// Attributes
	HMENU GetMenuHandle() const
	{
		return m_hMenu;
	}

	void SetMenuHandle(HMENU hMenu)
	{
		ATLASSERT(hMenu == NULL || ::IsMenu(hMenu));
		m_hMenu = hMenu;
		if(m_hMenu == NULL || (::GetMenuString(m_hMenu, t_nFirstID, m_szNoEntries, t_cchItemLen, MF_BYCOMMAND) == 0))
		{
			T* pT = static_cast<T*>(this);
			pT;   // avoid level 4 warning
			SecureHelper::strncpy_x(m_szNoEntries, _countof(m_szNoEntries), pT->GetMRUEmptyText(), _TRUNCATE);
		}
	}

	int GetMaxEntries() const
	{
		return m_nMaxEntries;
	}

	void SetMaxEntries(int nMaxEntries)
	{
		ATLASSERT(nMaxEntries >= m_nMaxEntries_Min && nMaxEntries <= m_nMaxEntries_Max);
		if(nMaxEntries < m_nMaxEntries_Min)
			nMaxEntries = m_nMaxEntries_Min;
		else if(nMaxEntries > m_nMaxEntries_Max)
			nMaxEntries = m_nMaxEntries_Max;
		m_nMaxEntries = nMaxEntries;
	}

	int GetMaxItemLength() const
	{
		return m_cchMaxItemLen;
	}

	void SetMaxItemLength(int cchMaxLen)
	{
		ATLASSERT((cchMaxLen >= m_cchMaxItemLen_Min && cchMaxLen <= m_cchMaxItemLen_Max) || cchMaxLen == -1);
		if(cchMaxLen != -1)
		{
			if(cchMaxLen < m_cchMaxItemLen_Min)
				cchMaxLen = m_cchMaxItemLen_Min;
			else if(cchMaxLen > m_cchMaxItemLen_Max)
				cchMaxLen = m_cchMaxItemLen_Max;
		}
		m_cchMaxItemLen = cchMaxLen;
		T* pT = static_cast<T*>(this);
		pT->UpdateMenu();
	}

// Operations
	BOOL AddToList(LPCTSTR lpstrDocName)
	{
		_DocEntry de;
		errno_t nRet = SecureHelper::strncpy_x(de.szDocName, _countof(de.szDocName), lpstrDocName, _TRUNCATE);
		if(nRet != 0 && nRet != STRUNCATE)
			return FALSE;

		for(int i = 0; i < m_arrDocs.GetSize(); i++)
		{
			if(lstrcmpi(m_arrDocs[i].szDocName, lpstrDocName) == 0)
			{
				m_arrDocs.RemoveAt(i);
				break;
			}
		}

		if(m_arrDocs.GetSize() == m_nMaxEntries)
			m_arrDocs.RemoveAt(0);

		BOOL bRet = m_arrDocs.Add(de);
		if(bRet)
		{
			T* pT = static_cast<T*>(this);
			bRet = pT->UpdateMenu();
		}
		return bRet;
	}

	// This function is deprecated because it is not safe. 
	// Use the version below that accepts the buffer length.
#if (_MSC_VER >= 1300)
	__declspec(deprecated)
#endif
	BOOL GetFromList(int /*nItemID*/, LPTSTR /*lpstrDocName*/)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	BOOL GetFromList(int nItemID, LPTSTR lpstrDocName, int cchLength)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		if(nIndex < 0 || nIndex >= m_arrDocs.GetSize())
			return FALSE;
		if(lstrlen(m_arrDocs[nIndex].szDocName) >= cchLength)
			return FALSE;
		SecureHelper::strcpy_x(lpstrDocName, cchLength, m_arrDocs[nIndex].szDocName);

		return TRUE;
	}

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	BOOL GetFromList(int nItemID, _CSTRING_NS::CAtlString& strDocName)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		if(nIndex < 0 || nIndex >= m_arrDocs.GetSize())
			return FALSE;
		strDocName = m_arrDocs[nIndex].szDocName;
		return TRUE;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	BOOL RemoveFromList(int nItemID)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		BOOL bRet = m_arrDocs.RemoveAt(nIndex);
		if(bRet)
		{
			T* pT = static_cast<T*>(this);
			bRet = pT->UpdateMenu();
		}
		return bRet;
	}

	BOOL MoveToTop(int nItemID)
	{
		int nIndex = m_arrDocs.GetSize() - (nItemID - t_nFirstID) - 1;
		if(nIndex < 0 || nIndex >= m_arrDocs.GetSize())
			return FALSE;
		_DocEntry de;
		de = m_arrDocs[nIndex];
		m_arrDocs.RemoveAt(nIndex);
		BOOL bRet = m_arrDocs.Add(de);
		if(bRet)
		{
			T* pT = static_cast<T*>(this);
			bRet = pT->UpdateMenu();
		}
		return bRet;
	}

	BOOL ReadFromRegistry(LPCTSTR lpstrRegKey)
	{
		T* pT = static_cast<T*>(this);
		CRegKeyEx rkParent;
		CRegKeyEx rk;

		LONG lRet = rkParent.Open(HKEY_CURRENT_USER, lpstrRegKey);
		if(lRet != ERROR_SUCCESS)
			return FALSE;
		lRet = rk.Open(rkParent, pT->GetRegKeyName());
		if(lRet != ERROR_SUCCESS)
			return FALSE;

		DWORD dwRet = 0;
		lRet = rk.QueryDWORDValue(pT->GetRegCountName(), dwRet);
		if(lRet != ERROR_SUCCESS)
			return FALSE;
		SetMaxEntries(dwRet);

		m_arrDocs.RemoveAll();

		TCHAR szRetString[t_cchItemLen] = { 0 };
		_DocEntry de;

		for(int nItem = m_nMaxEntries; nItem > 0; nItem--)
		{
			TCHAR szBuff[m_cchItemNameLen] = { 0 };
			SecureHelper::wsprintf_x(szBuff, m_cchItemNameLen, pT->GetRegItemName(), nItem);
			ULONG ulCount = t_cchItemLen;
			lRet = rk.QueryStringValue(szBuff, szRetString, &ulCount);
			if(lRet == ERROR_SUCCESS)
			{
				SecureHelper::strcpy_x(de.szDocName, _countof(de.szDocName), szRetString);
				m_arrDocs.Add(de);
			}
		}

		rk.Close();
		rkParent.Close();

		return pT->UpdateMenu();
	}

	BOOL WriteToRegistry(LPCTSTR lpstrRegKey)
	{
		T* pT = static_cast<T*>(this);
		pT;   // avoid level 4 warning
		CRegKeyEx rkParent;
		CRegKeyEx rk;

		LONG lRet = rkParent.Create(HKEY_CURRENT_USER, lpstrRegKey);
		if(lRet != ERROR_SUCCESS)
			return FALSE;
		lRet = rk.Create(rkParent, pT->GetRegKeyName());
		if(lRet != ERROR_SUCCESS)
			return FALSE;

		lRet = rk.SetDWORDValue(pT->GetRegCountName(), m_nMaxEntries);
		ATLASSERT(lRet == ERROR_SUCCESS);

		// set new values
		int nItem;
		for(nItem = m_arrDocs.GetSize(); nItem > 0; nItem--)
		{
			TCHAR szBuff[m_cchItemNameLen] = { 0 };
			SecureHelper::wsprintf_x(szBuff, m_cchItemNameLen, pT->GetRegItemName(), nItem);
			TCHAR szDocName[t_cchItemLen] = { 0 };
			GetFromList(t_nFirstID + nItem - 1, szDocName, t_cchItemLen);
			lRet = rk.SetStringValue(szBuff, szDocName);
			ATLASSERT(lRet == ERROR_SUCCESS);
		}

		// delete unused keys
		for(nItem = m_arrDocs.GetSize() + 1; nItem < m_nMaxEntries_Max; nItem++)
		{
			TCHAR szBuff[m_cchItemNameLen] = { 0 };
			SecureHelper::wsprintf_x(szBuff, m_cchItemNameLen, pT->GetRegItemName(), nItem);
			rk.DeleteValue(szBuff);
		}

		rk.Close();
		rkParent.Close();

		return TRUE;
	}

// Implementation
	BOOL UpdateMenu()
	{
		if(m_hMenu == NULL)
			return FALSE;
		ATLASSERT(::IsMenu(m_hMenu));

		int nItems = ::GetMenuItemCount(m_hMenu);
		int nInsertPoint;
		for(nInsertPoint = 0; nInsertPoint < nItems; nInsertPoint++)
		{
			CMenuItemInfo mi;
			mi.fMask = MIIM_ID;
			::GetMenuItemInfo(m_hMenu, nInsertPoint, TRUE, &mi);
			if (mi.wID == t_nFirstID)
				break;
		}
		ATLASSERT(nInsertPoint < nItems && "You need a menu item with an ID = t_nFirstID");

		int nItem;
		for(nItem = t_nFirstID; nItem < t_nFirstID + m_nMaxEntries; nItem++)
		{
			// keep the first one as an insertion point
			if (nItem != t_nFirstID)
				::DeleteMenu(m_hMenu, nItem, MF_BYCOMMAND);
		}

		TCHAR szItemText[t_cchItemLen + 6] = { 0 };   // add space for &, 2 digits, and a space
		int nSize = m_arrDocs.GetSize();
		nItem = 0;
		if(nSize > 0)
		{
			for(nItem = 0; nItem < nSize; nItem++)
			{
				if(m_cchMaxItemLen == -1)
				{
					SecureHelper::wsprintf_x(szItemText, t_cchItemLen + 6, _T("&%i %s"), nItem + 1, m_arrDocs[nSize - 1 - nItem].szDocName);
				}
				else
				{
					TCHAR szBuff[t_cchItemLen] = { 0 };
					T* pT = static_cast<T*>(this);
					pT;   // avoid level 4 warning
					bool bRet = pT->CompactDocumentName(szBuff, m_arrDocs[nSize - 1 - nItem].szDocName, m_cchMaxItemLen);
					bRet;   // avoid level 4 warning
					ATLASSERT(bRet);
					SecureHelper::wsprintf_x(szItemText, t_cchItemLen + 6, _T("&%i %s"), nItem + 1, szBuff);
				}
				::InsertMenu(m_hMenu, nInsertPoint + nItem, MF_BYPOSITION | MF_STRING, t_nFirstID + nItem, szItemText);
			}
		}
		else	// empty
		{
			::InsertMenu(m_hMenu, nInsertPoint, MF_BYPOSITION | MF_STRING, t_nFirstID, m_szNoEntries);
			::EnableMenuItem(m_hMenu, t_nFirstID, MF_GRAYED);
			nItem++;
		}
		::DeleteMenu(m_hMenu, nInsertPoint + nItem, MF_BYPOSITION);

		return TRUE;
	}

// Overrideables
	// override to provide a different method of compacting document names
	static bool CompactDocumentName(LPTSTR lpstrOut, LPCTSTR lpstrIn, int cchLen)
	{
		return AtlCompactPath(lpstrOut, lpstrIn, cchLen);
	}

	static LPCTSTR GetRegKeyName()
	{
		return _T("Recent Document List");
	}

	static LPCTSTR GetRegCountName()
	{
		return _T("DocumentCount");
	}

	static LPCTSTR GetRegItemName()
	{
		// Note: This string is a format string used with wsprintf().
		// Resulting formatted string must be m_cchItemNameLen or less 
		// characters long, including the terminating null character.
		return _T("Document%i");
	}

	static LPCTSTR GetMRUEmptyText()
	{
		return _WTL_MRUEMPTY_TEXT;
	}
};

class CRecentDocumentList : public CRecentDocumentListBase<CRecentDocumentList>
{
public:
// nothing here
};

#endif // _WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CFindFile - file search helper class

class CFindFile
{
public:
// Data members
	WIN32_FIND_DATA m_fd;
	TCHAR m_lpszRoot[MAX_PATH];
	TCHAR m_chDirSeparator;
	HANDLE m_hFind;
	BOOL m_bFound;

// Constructor/destructor
	CFindFile() : m_hFind(NULL), m_chDirSeparator(_T('\\')), m_bFound(FALSE)
	{ }

	~CFindFile()
	{
		Close();
	}

// Attributes
	ULONGLONG GetFileSize() const
	{
		ATLASSERT(m_hFind != NULL);

		ULARGE_INTEGER nFileSize = { 0 };

		if(m_bFound)
		{
			nFileSize.LowPart = m_fd.nFileSizeLow;
			nFileSize.HighPart = m_fd.nFileSizeHigh;
		}
		else
		{
			nFileSize.QuadPart = 0;
		}

		return nFileSize.QuadPart;
	}

	BOOL GetFileName(LPTSTR lpstrFileName, int cchLength) const
	{
		ATLASSERT(m_hFind != NULL);
		if(lstrlen(m_fd.cFileName) >= cchLength)
			return FALSE;

		if(m_bFound)
			SecureHelper::strcpy_x(lpstrFileName, cchLength, m_fd.cFileName);

		return m_bFound;
	}

	BOOL GetFilePath(LPTSTR lpstrFilePath, int cchLength) const
	{
		ATLASSERT(m_hFind != NULL);

		int nLen = lstrlen(m_lpszRoot);
#ifndef _WIN32_WCE
		ATLASSERT(nLen > 0);
		if(nLen == 0)
			return FALSE;

		bool bAddSep = (m_lpszRoot[nLen - 1] != _T('\\') && m_lpszRoot[nLen - 1] !=_T('/'));
#else // CE specific
		// allow diskless devices (nLen == 0)
		bool bAddSep = ((nLen == 0) || (m_lpszRoot[nLen - 1] != _T('\\') && m_lpszRoot[nLen - 1] !=_T('/')));
#endif // _WIN32_WCE

		if((lstrlen(m_lpszRoot) + (bAddSep ?  1 : 0)) >= cchLength)
			return FALSE;

		SecureHelper::strcpy_x(lpstrFilePath, cchLength, m_lpszRoot);

		if(bAddSep)
		{
			TCHAR szSeparator[2] = { m_chDirSeparator, 0 };
			SecureHelper::strcat_x(lpstrFilePath, cchLength, szSeparator);
		}

		SecureHelper::strcat_x(lpstrFilePath, cchLength, m_fd.cFileName);

		return TRUE;
	}

#ifndef _WIN32_WCE
	BOOL GetFileTitle(LPTSTR lpstrFileTitle, int cchLength) const
	{
		ATLASSERT(m_hFind != NULL);

		TCHAR szBuff[MAX_PATH] = { 0 };
		if(!GetFileName(szBuff, MAX_PATH))
			return FALSE;

		if(lstrlen(szBuff) >= cchLength || cchLength < 1)
			return FALSE;

		// find the last dot
		LPTSTR pstrDot  = MinCrtHelper::_strrchr(szBuff, _T('.'));
		if(pstrDot != NULL)
			*pstrDot = 0;

		SecureHelper::strcpy_x(lpstrFileTitle, cchLength, szBuff);

		return TRUE;
	}
#endif // !_WIN32_WCE

	BOOL GetFileURL(LPTSTR lpstrFileURL, int cchLength) const
	{
		ATLASSERT(m_hFind != NULL);

		TCHAR szBuff[MAX_PATH] = { 0 };
		if(!GetFilePath(szBuff, MAX_PATH))
			return FALSE;
		LPCTSTR lpstrFileURLPrefix = _T("file://");
		if(lstrlen(szBuff) + lstrlen(lpstrFileURLPrefix) >= cchLength)
			return FALSE;
		SecureHelper::strcpy_x(lpstrFileURL, cchLength, lpstrFileURLPrefix);
		SecureHelper::strcat_x(lpstrFileURL, cchLength, szBuff);

		return TRUE;
	}

	BOOL GetRoot(LPTSTR lpstrRoot, int cchLength) const
	{
		ATLASSERT(m_hFind != NULL);
		if(lstrlen(m_lpszRoot) >= cchLength)
			return FALSE;

		SecureHelper::strcpy_x(lpstrRoot, cchLength, m_lpszRoot);

		return TRUE;
	}

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	_CSTRING_NS::CAtlString GetFileName() const
	{
		ATLASSERT(m_hFind != NULL);

		_CSTRING_NS::CAtlString ret;

		if(m_bFound)
			ret = m_fd.cFileName;
		return ret;
	}

	_CSTRING_NS::CAtlString GetFilePath() const
	{
		ATLASSERT(m_hFind != NULL);

		_CSTRING_NS::CAtlString strResult = m_lpszRoot;
		int nLen = strResult.GetLength();
#ifndef _WIN32_WCE
		ATLASSERT(nLen > 0);
		if(nLen == 0)
			return strResult;

		if((strResult[nLen - 1] != _T('\\')) && (strResult[nLen - 1] != _T('/')))
#else // CE specific
		// allow diskless devices (nLen == 0)
		if((nLen == 0) || ((strResult[nLen - 1] != _T('\\')) && (strResult[nLen - 1] != _T('/'))))
#endif // _WIN32_WCE
			strResult += m_chDirSeparator;
		strResult += GetFileName();
		return strResult;
	}

#ifndef _WIN32_WCE
	_CSTRING_NS::CAtlString GetFileTitle() const
	{
		ATLASSERT(m_hFind != NULL);

		_CSTRING_NS::CAtlString strResult;
		GetFileTitle(strResult.GetBuffer(MAX_PATH), MAX_PATH);
		strResult.ReleaseBuffer();

		return strResult;
	}
#endif // !_WIN32_WCE

	_CSTRING_NS::CAtlString GetFileURL() const
	{
		ATLASSERT(m_hFind != NULL);

		_CSTRING_NS::CAtlString strResult("file://");
		strResult += GetFilePath();
		return strResult;
	}

	_CSTRING_NS::CAtlString GetRoot() const
	{
		ATLASSERT(m_hFind != NULL);

		_CSTRING_NS::CAtlString str = m_lpszRoot;
		return str;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	BOOL GetLastWriteTime(FILETIME* pTimeStamp) const
	{
		ATLASSERT(m_hFind != NULL);
		ATLASSERT(pTimeStamp != NULL);

		if(m_bFound && pTimeStamp != NULL)
		{
			*pTimeStamp = m_fd.ftLastWriteTime;
			return TRUE;
		}

		return FALSE;
	}

	BOOL GetLastAccessTime(FILETIME* pTimeStamp) const
	{
		ATLASSERT(m_hFind != NULL);
		ATLASSERT(pTimeStamp != NULL);

		if(m_bFound && pTimeStamp != NULL)
		{
			*pTimeStamp = m_fd.ftLastAccessTime;
			return TRUE;
		}

		return FALSE;
	}

	BOOL GetCreationTime(FILETIME* pTimeStamp) const
	{
		ATLASSERT(m_hFind != NULL);

		if(m_bFound && pTimeStamp != NULL)
		{
			*pTimeStamp = m_fd.ftCreationTime;
			return TRUE;
		}

		return FALSE;
	}

	BOOL MatchesMask(DWORD dwMask) const
	{
		ATLASSERT(m_hFind != NULL);

		if(m_bFound)
			return ((m_fd.dwFileAttributes & dwMask) != 0);

		return FALSE;
	}

	BOOL IsDots() const
	{
		ATLASSERT(m_hFind != NULL);

		// return TRUE if the file name is "." or ".." and
		// the file is a directory

		BOOL bResult = FALSE;
		if(m_bFound && IsDirectory())
		{
			if(m_fd.cFileName[0] == _T('.') && (m_fd.cFileName[1] == _T('\0') || (m_fd.cFileName[1] == _T('.') && m_fd.cFileName[2] == _T('\0'))))
				bResult = TRUE;
		}

		return bResult;
	}

	BOOL IsReadOnly() const
	{
		return MatchesMask(FILE_ATTRIBUTE_READONLY);
	}

	BOOL IsDirectory() const
	{
		return MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
	}

	BOOL IsCompressed() const
	{
		return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);
	}

	BOOL IsSystem() const
	{
		return MatchesMask(FILE_ATTRIBUTE_SYSTEM);
	}

	BOOL IsHidden() const
	{
		return MatchesMask(FILE_ATTRIBUTE_HIDDEN);
	}

	BOOL IsTemporary() const
	{
		return MatchesMask(FILE_ATTRIBUTE_TEMPORARY);
	}

	BOOL IsNormal() const
	{
		return MatchesMask(FILE_ATTRIBUTE_NORMAL);
	}

	BOOL IsArchived() const
	{
		return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);
	}

// Operations
	BOOL FindFile(LPCTSTR pstrName = NULL)
	{
		Close();

		if(pstrName == NULL)
		{
			pstrName = _T("*.*");
		}
		else if(lstrlen(pstrName) >= MAX_PATH)
		{
			ATLASSERT(FALSE);
			return FALSE;
		}

		SecureHelper::strcpy_x(m_fd.cFileName, _countof(m_fd.cFileName), pstrName);

		m_hFind = ::FindFirstFile(pstrName, &m_fd);

		if(m_hFind == INVALID_HANDLE_VALUE)
			return FALSE;

#ifndef _WIN32_WCE
		bool bFullPath = (::GetFullPathName(pstrName, MAX_PATH, m_lpszRoot, NULL) != 0);
#else // CE specific
		errno_t nRet = SecureHelper::strncpy_x(m_lpszRoot, _countof(m_lpszRoot), pstrName, _TRUNCATE);
		bool bFullPath = (nRet == 0 || nRet == STRUNCATE);
#endif // _WIN32_WCE

		// passed name isn't a valid path but was found by the API
		ATLASSERT(bFullPath);
		if(!bFullPath)
		{
			Close();
			::SetLastError(ERROR_INVALID_NAME);
			return FALSE;
		}
		else
		{
			// find the last forward or backward whack
			LPTSTR pstrBack  = MinCrtHelper::_strrchr(m_lpszRoot, _T('\\'));
			LPTSTR pstrFront = MinCrtHelper::_strrchr(m_lpszRoot, _T('/'));

			if(pstrFront != NULL || pstrBack != NULL)
			{
				if(pstrFront == NULL)
					pstrFront = m_lpszRoot;
				if(pstrBack == NULL)
					pstrBack = m_lpszRoot;

				// from the start to the last whack is the root

				if(pstrFront >= pstrBack)
					*pstrFront = _T('\0');
				else
					*pstrBack = _T('\0');
			}
		}

		m_bFound = TRUE;

		return TRUE;
	}

	BOOL FindNextFile()
	{
		ATLASSERT(m_hFind != NULL);

		if(m_hFind == NULL)
			return FALSE;

		if(!m_bFound)
			return FALSE;

		m_bFound = ::FindNextFile(m_hFind, &m_fd);

		return m_bFound;
	}

	void Close()
	{
		m_bFound = FALSE;

		if(m_hFind != NULL && m_hFind != INVALID_HANDLE_VALUE)
		{
			::FindClose(m_hFind);
			m_hFind = NULL;
		}
	}
};


///////////////////////////////////////////////////////////////////////////////
// Global functions for stock GDI objects

inline HPEN AtlGetStockPen(int nPen)
{
#if (_WIN32_WINNT >= 0x0500) && !defined(_WIN32_WCE)
	ATLASSERT(nPen == WHITE_PEN || nPen == BLACK_PEN || nPen == NULL_PEN || nPen == DC_PEN);
#else
	ATLASSERT(nPen == WHITE_PEN || nPen == BLACK_PEN || nPen == NULL_PEN);
#endif
	return (HPEN)::GetStockObject(nPen);
}

inline HBRUSH AtlGetStockBrush(int nBrush)
{
#if (_WIN32_WINNT >= 0x0500) && !defined(_WIN32_WCE)
	ATLASSERT((nBrush >= WHITE_BRUSH && nBrush <= HOLLOW_BRUSH) || nBrush == DC_BRUSH);
#else
	ATLASSERT(nBrush >= WHITE_BRUSH && nBrush <= HOLLOW_BRUSH);
#endif
	return (HBRUSH)::GetStockObject(nBrush);
}

inline HFONT AtlGetStockFont(int nFont)
{
#ifndef _WIN32_WCE
	ATLASSERT((nFont >= OEM_FIXED_FONT && nFont <= SYSTEM_FIXED_FONT) || nFont == DEFAULT_GUI_FONT);
#else // CE specific
	ATLASSERT(nFont == SYSTEM_FONT);
#endif // _WIN32_WCE
	return (HFONT)::GetStockObject(nFont);
}

inline HPALETTE AtlGetStockPalette(int nPalette)
{
	ATLASSERT(nPalette == DEFAULT_PALETTE); // the only one supported
	return (HPALETTE)::GetStockObject(nPalette);
}


///////////////////////////////////////////////////////////////////////////////
// Global function for compacting a path by replacing parts with ellipsis

// helper for multi-byte character sets
inline bool _IsDBCSTrailByte(LPCTSTR lpstr, int nChar)
{
#ifndef _UNICODE
	int i = nChar;
	for( ; i > 0; i--)
	{
		if(!::IsDBCSLeadByte(lpstr[i - 1]))
			break;
	}
	return ((nChar > 0) && (((nChar - i) & 1) != 0));
#else // _UNICODE
	lpstr; nChar;
	return false;
#endif // _UNICODE
}

inline bool AtlCompactPath(LPTSTR lpstrOut, LPCTSTR lpstrIn, int cchLen)
{
	ATLASSERT(lpstrOut != NULL);
	ATLASSERT(lpstrIn != NULL);
	ATLASSERT(cchLen > 0);

	LPCTSTR szEllipsis = _T("...");
	const int cchEndEllipsis = 3;
	const int cchMidEllipsis = 4;

	if(lstrlen(lpstrIn) < cchLen)
	{
		SecureHelper::strcpy_x(lpstrOut, cchLen, lpstrIn);
		return true;
	}

	lpstrOut[0] = 0;

	// check if the separator is a slash or a backslash
	TCHAR chSlash = _T('\\');
	for(LPTSTR lpstr = (LPTSTR)lpstrIn; *lpstr != 0; lpstr = ::CharNext(lpstr))
	{
		if((*lpstr == _T('/')) || (*lpstr == _T('\\')))
			chSlash = *lpstr;
	}

	// find the filename portion of the path
	LPCTSTR lpstrFileName = lpstrIn;
	for(LPCTSTR pPath = lpstrIn; *pPath; pPath = ::CharNext(pPath))
	{
		if((pPath[0] == _T('\\') || pPath[0] == _T(':') || pPath[0] == _T('/'))
				&& pPath[1] && pPath[1] != _T('\\') && pPath[1] != _T('/'))
			lpstrFileName = pPath + 1;
	}
	int cchFileName = lstrlen(lpstrFileName);

	// handle just the filename without a path
	if(lpstrFileName == lpstrIn && cchLen > cchEndEllipsis)
	{
		bool bRet = (SecureHelper::strncpy_x(lpstrOut, cchLen, lpstrIn, cchLen - cchEndEllipsis - 1) == 0);
		if(bRet)
		{
#ifndef _UNICODE
			if(_IsDBCSTrailByte(lpstrIn, cchLen - cchEndEllipsis))
				lpstrOut[cchLen - cchEndEllipsis - 1] = 0;
#endif // _UNICODE
			SecureHelper::strcat_x(lpstrOut, cchLen, szEllipsis);
		}
		return bRet;
	}

	// handle just ellipsis
	if((cchLen < (cchMidEllipsis + cchEndEllipsis)))
	{
		for(int i = 0; i < cchLen - 1; i++)
			lpstrOut[i] = ((i + 1) == cchMidEllipsis) ? chSlash : _T('.');
		lpstrOut[cchLen - 1] = 0;
		return true;
	}

	// calc how much we have to copy
	int cchToCopy = cchLen - (cchMidEllipsis + cchFileName) - 1;

	if(cchToCopy < 0)
		cchToCopy = 0;

#ifndef _UNICODE
	if(cchToCopy > 0 && _IsDBCSTrailByte(lpstrIn, cchToCopy))
		cchToCopy--;
#endif // _UNICODE

	bool bRet = (SecureHelper::strncpy_x(lpstrOut, cchLen, lpstrIn, cchToCopy) == 0);
	if(!bRet)
		return false;

	// add ellipsis
	SecureHelper::strcat_x(lpstrOut, cchLen, szEllipsis);
	if(!bRet)
		return false;
	TCHAR szSlash[2] = { chSlash, 0 };
	SecureHelper::strcat_x(lpstrOut, cchLen, szSlash);
	if(!bRet)
		return false;

	// add filename (and ellipsis, if needed)
	if(cchLen > (cchMidEllipsis + cchFileName))
	{
		SecureHelper::strcat_x(lpstrOut, cchLen, lpstrFileName);
	}
	else
	{
		cchToCopy = cchLen - cchMidEllipsis - cchEndEllipsis - 1;
#ifndef _UNICODE
		if(cchToCopy > 0 && _IsDBCSTrailByte(lpstrFileName, cchToCopy))
			cchToCopy--;
#endif // _UNICODE
		bRet = (SecureHelper::strncpy_x(&lpstrOut[cchMidEllipsis], cchLen - cchMidEllipsis, lpstrFileName, cchToCopy) == 0);
		if(bRet)
			SecureHelper::strcat_x(lpstrOut, cchLen, szEllipsis);
	}

	return bRet;
}

}; // namespace WTL

#endif // __ATLMISC_H__
