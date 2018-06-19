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

#ifndef __ATLCTRLS_H__
#define __ATLCTRLS_H__

#pragma once

#ifndef __ATLAPP_H__
	#error atlctrls.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlctrls.h requires atlwin.h to be included first
#endif

#ifndef _WIN32_WCE
  #include <richedit.h>
  #include <richole.h>
#elif defined(WIN32_PLATFORM_WFSP) && !defined(_WINUSERM_H_)
  #include <winuserm.h>
#endif // !_WIN32_WCE

// protect template members from windowsx.h macros
#ifdef _INC_WINDOWSX
  #undef GetNextSibling
  #undef GetPrevSibling
#endif // _INC_WINDOWSX


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CStaticT<TBase> - CStatic
// CButtonT<TBase> - CButton
// CListBoxT<TBase> - CListBox
// CComboBoxT<TBase> - CComboBox
// CEditT<TBase> - CEdit
// CEditCommands<T>
// CScrollBarT<TBase> - CScrollBar
//
// CImageList
// CListViewCtrlT<TBase> - CListViewCtrl
// CTreeViewCtrlT<TBase> - CTreeViewCtrl
// CTreeItemT<TBase> - CTreeItem
// CTreeViewCtrlExT<TBase> - CTreeViewCtrlEx
// CHeaderCtrlT<TBase> - CHeaderCtrl
// CToolBarCtrlT<TBase> - CToolBarCtrl
// CStatusBarCtrlT<TBase> - CStatusBarCtrl
// CTabCtrlT<TBase> - CTabCtrl
// CToolInfo
// CToolTipCtrlT<TBase> - CToolTipCtrl
// CTrackBarCtrlT<TBase> - CTrackBarCtrl
// CUpDownCtrlT<TBase> - CUpDownCtrl
// CProgressBarCtrlT<TBase> - CProgressBarCtrl
// CHotKeyCtrlT<TBase> - CHotKeyCtrl
// CAnimateCtrlT<TBase> - CAnimateCtrl
// CRichEditCtrlT<TBase> - CRichEditCtrl
// CRichEditCommands<T>
// CDragListBoxT<TBase> - CDragListBox
// CDragListNotifyImpl<T>
// CReBarCtrlT<TBase> - CReBarCtrl
// CComboBoxExT<TBase> - CComboBoxEx
// CDateTimePickerCtrlT<TBase> - CDateTimePickerCtrl
// CMonthCalendarCtrlT<TBase> - CMonthCalendarCtrl
// CFlatScrollBarImpl<T>
// CFlatScrollBarT<TBase> - CFlatScrollBar
// CIPAddressCtrlT<TBase> - CIPAddressCtrl
// CPagerCtrlT<TBase> - CPagerCtrl
// CLinkCtrlT<TBase> - CLinkCtrl
//
// CCustomDraw<T>
//
// CCECommandBarCtrlT<TBase> - CCECommandBarCtrl
// CCECommandBandsCtrlT<TBase> - CCECommandBandsCtrl


namespace WTL
{

// These are wrapper classes for Windows standard and common controls.
// To implement a window based on a control, use following:
// Example: Implementing a window based on a list box
//
// class CMyListBox : CWindowImpl<CMyListBox, CListBox>
// {
// public:
//      BEGIN_MSG_MAP(CMyListBox)
//          // put your message handler entries here
//      END_MSG_MAP()
// };



// --- Standard Windows controls ---

///////////////////////////////////////////////////////////////////////////////
// CStatic - client side for a Windows STATIC control

template <class TBase>
class CStaticT : public TBase
{
public:
// Constructors
	CStaticT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CStaticT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return _T("STATIC");
	}

#ifndef _WIN32_WCE
	HICON GetIcon() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, STM_GETICON, 0, 0L);
	}

	HICON SetIcon(HICON hIcon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, STM_SETICON, (WPARAM)hIcon, 0L);
	}

	HENHMETAFILE GetEnhMetaFile() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HENHMETAFILE)::SendMessage(m_hWnd, STM_GETIMAGE, IMAGE_ENHMETAFILE, 0L);
	}

	HENHMETAFILE SetEnhMetaFile(HENHMETAFILE hMetaFile)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HENHMETAFILE)::SendMessage(m_hWnd, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)hMetaFile);
	}
#else // CE specific
	HICON GetIcon() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, STM_GETIMAGE, IMAGE_ICON, 0L);
	}

	HICON SetIcon(HICON hIcon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	}
#endif // _WIN32_WCE

	CBitmapHandle GetBitmap() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CBitmapHandle((HBITMAP)::SendMessage(m_hWnd, STM_GETIMAGE, IMAGE_BITMAP, 0L));
	}

	CBitmapHandle SetBitmap(HBITMAP hBitmap)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CBitmapHandle((HBITMAP)::SendMessage(m_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap));
	}

	HCURSOR GetCursor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HCURSOR)::SendMessage(m_hWnd, STM_GETIMAGE, IMAGE_CURSOR, 0L);
	}

	HCURSOR SetCursor(HCURSOR hCursor)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HCURSOR)::SendMessage(m_hWnd, STM_SETIMAGE, IMAGE_CURSOR, (LPARAM)hCursor);
	}
};

typedef CStaticT<ATL::CWindow>   CStatic;


///////////////////////////////////////////////////////////////////////////////
// CButton - client side for a Windows BUTTON control

template <class TBase>
class CButtonT : public TBase
{
public:
// Constructors
	CButtonT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CButtonT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return _T("BUTTON");
	}

	UINT GetState() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, BM_GETSTATE, 0, 0L);
	}

	void SetState(BOOL bHighlight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, BM_SETSTATE, bHighlight, 0L);
	}

	int GetCheck() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, BM_GETCHECK, 0, 0L);
	}

	void SetCheck(int nCheck)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, BM_SETCHECK, nCheck, 0L);
	}

	UINT GetButtonStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::GetWindowLong(m_hWnd, GWL_STYLE) & 0xFFFF;
	}

	void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, BM_SETSTYLE, nStyle, (LPARAM)bRedraw);
	}

#ifndef _WIN32_WCE
	HICON GetIcon() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, BM_GETIMAGE, IMAGE_ICON, 0L);
	}

	HICON SetIcon(HICON hIcon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	}

	CBitmapHandle GetBitmap() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CBitmapHandle((HBITMAP)::SendMessage(m_hWnd, BM_GETIMAGE, IMAGE_BITMAP, 0L));
	}

	CBitmapHandle SetBitmap(HBITMAP hBitmap)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CBitmapHandle((HBITMAP)::SendMessage(m_hWnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap));
	}
#endif // !_WIN32_WCE

#if (_WIN32_WINNT >= 0x0501)
	BOOL GetIdealSize(LPSIZE lpSize) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, BCM_GETIDEALSIZE, 0, (LPARAM)lpSize);
	}

	BOOL GetImageList(PBUTTON_IMAGELIST pButtonImagelist) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, BCM_GETIMAGELIST, 0, (LPARAM)pButtonImagelist);
	}

	BOOL SetImageList(PBUTTON_IMAGELIST pButtonImagelist)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, BCM_SETIMAGELIST, 0, (LPARAM)pButtonImagelist);
	}

	BOOL GetTextMargin(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, BCM_GETTEXTMARGIN, 0, (LPARAM)lpRect);
	}

	BOOL SetTextMargin(LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, BCM_SETTEXTMARGIN, 0, (LPARAM)lpRect);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (WINVER >= 0x0600)
	void SetDontClick(BOOL bDontClick)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, BM_SETDONTCLICK, (WPARAM)bDontClick, 0L);
	}
#endif // (WINVER >= 0x0600)

#if (_WIN32_WINNT >= 0x0600)
	BOOL SetDropDownState(BOOL bDropDown)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (BS_SPLITBUTTON | BS_DEFSPLITBUTTON)) != 0);
		return (BOOL)::SendMessage(m_hWnd, BCM_SETDROPDOWNSTATE, (WPARAM)bDropDown, 0L);
	}

	BOOL GetSplitInfo(PBUTTON_SPLITINFO pSplitInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (BS_SPLITBUTTON | BS_DEFSPLITBUTTON)) != 0);
		return (BOOL)::SendMessage(m_hWnd, BCM_GETSPLITINFO, 0, (LPARAM)pSplitInfo);
	}

	BOOL SetSplitInfo(PBUTTON_SPLITINFO pSplitInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (BS_SPLITBUTTON | BS_DEFSPLITBUTTON)) != 0);
		return (BOOL)::SendMessage(m_hWnd, BCM_SETSPLITINFO, 0, (LPARAM)pSplitInfo);
	}

	int GetNoteLength() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (BS_COMMANDLINK | BS_DEFCOMMANDLINK)) != 0);
		return (int)::SendMessage(m_hWnd, BCM_GETNOTELENGTH, 0, 0L);
	}

	BOOL GetNote(LPWSTR lpstrNoteText, int cchNoteText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (BS_COMMANDLINK | BS_DEFCOMMANDLINK)) != 0);
		return (BOOL)::SendMessage(m_hWnd, BCM_GETNOTE, cchNoteText, (LPARAM)lpstrNoteText);
	}

	BOOL SetNote(LPCWSTR lpstrNoteText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (BS_COMMANDLINK | BS_DEFCOMMANDLINK)) != 0);
		return (BOOL)::SendMessage(m_hWnd, BCM_SETNOTE, 0, (LPARAM)lpstrNoteText);
	}

	LRESULT SetElevationRequiredState(BOOL bSet)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendMessage(m_hWnd, BCM_SETSHIELD, 0, (LPARAM)bSet);
	}
#endif // (_WIN32_WINNT >= 0x0600)

// Operations
	void Click()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, BM_CLICK, 0, 0L);
	}
};

typedef CButtonT<ATL::CWindow>   CButton;


///////////////////////////////////////////////////////////////////////////////
// CListBox - client side for a Windows LISTBOX control

template <class TBase>
class CListBoxT : public TBase
{
public:
// Constructors
	CListBoxT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CListBoxT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return _T("LISTBOX");
	}

	// for entire listbox
	int GetCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETCOUNT, 0, 0L);
	}

#ifndef _WIN32_WCE
	int SetCount(int cItems)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(((GetStyle() & LBS_NODATA) != 0) && ((GetStyle() & LBS_HASSTRINGS) == 0));
		return (int)::SendMessage(m_hWnd, LB_SETCOUNT, cItems, 0L);
	}
#endif // !_WIN32_WCE

	int GetHorizontalExtent() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETHORIZONTALEXTENT, 0, 0L);
	}

	void SetHorizontalExtent(int cxExtent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LB_SETHORIZONTALEXTENT, cxExtent, 0L);
	}

	int GetTopIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETTOPINDEX, 0, 0L);
	}

	int SetTopIndex(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_SETTOPINDEX, nIndex, 0L);
	}

	LCID GetLocale() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LCID)::SendMessage(m_hWnd, LB_GETLOCALE, 0, 0L);
	}

	LCID SetLocale(LCID nNewLocale)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LCID)::SendMessage(m_hWnd, LB_SETLOCALE, (WPARAM)nNewLocale, 0L);
	}

#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
	DWORD GetListBoxInfo() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
#if (_WIN32_WINNT >= 0x0501)
		return (DWORD)::SendMessage(m_hWnd, LB_GETLISTBOXINFO, 0, 0L);
#else // !(_WIN32_WINNT >= 0x0501)
		return ::GetListBoxInfo(m_hWnd);
#endif // !(_WIN32_WINNT >= 0x0501)
	}
#endif // (WINVER >= 0x0500) && !defined(_WIN32_WCE)

	// for single-selection listboxes
	int GetCurSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) == 0);
		return (int)::SendMessage(m_hWnd, LB_GETCURSEL, 0, 0L);
	}

	int SetCurSel(int nSelect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) == 0);
		return (int)::SendMessage(m_hWnd, LB_SETCURSEL, nSelect, 0L);
	}

	// for multiple-selection listboxes
	int GetSel(int nIndex) const           // also works for single-selection
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETSEL, nIndex, 0L);
	}

	int SetSel(int nIndex, BOOL bSelect = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0);
		return (int)::SendMessage(m_hWnd, LB_SETSEL, bSelect, nIndex);
	}

	int GetSelCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0);
		return (int)::SendMessage(m_hWnd, LB_GETSELCOUNT, 0, 0L);
	}

	int GetSelItems(int nMaxItems, LPINT rgIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0);
		return (int)::SendMessage(m_hWnd, LB_GETSELITEMS, nMaxItems, (LPARAM)rgIndex);
	}

	int GetAnchorIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0);
		return (int)::SendMessage(m_hWnd, LB_GETANCHORINDEX, 0, 0L);
	}

	void SetAnchorIndex(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0);
		::SendMessage(m_hWnd, LB_SETANCHORINDEX, nIndex, 0L);
	}

	int GetCaretIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETCARETINDEX, 0, 0);
	}

	int SetCaretIndex(int nIndex, BOOL bScroll = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_SETCARETINDEX, nIndex, MAKELONG(bScroll, 0));
	}

	// for listbox items
	DWORD_PTR GetItemData(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD_PTR)::SendMessage(m_hWnd, LB_GETITEMDATA, nIndex, 0L);
	}

	int SetItemData(int nIndex, DWORD_PTR dwItemData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_SETITEMDATA, nIndex, (LPARAM)dwItemData);
	}

	void* GetItemDataPtr(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (void*)::SendMessage(m_hWnd, LB_GETITEMDATA, nIndex, 0L);
	}

	int SetItemDataPtr(int nIndex, void* pData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItemData(nIndex, (DWORD_PTR)pData);
	}

	int GetItemRect(int nIndex, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETITEMRECT, nIndex, (LPARAM)lpRect);
	}

	int GetText(int nIndex, LPTSTR lpszBuffer) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)lpszBuffer);
	}

#ifndef _ATL_NO_COM
#ifdef _OLEAUTO_H_
	BOOL GetTextBSTR(int nIndex, BSTR& bstrText) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrText == NULL);

		int nLen = GetTextLen(nIndex);
		if(nLen == LB_ERR)
			return FALSE;

		CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPTSTR lpstrText = buff.Allocate(nLen + 1);
		if(lpstrText == NULL)
			return FALSE;

		if(GetText(nIndex, lpstrText) == LB_ERR)
			return FALSE;

		bstrText = ::SysAllocString(T2OLE(lpstrText));
		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // _OLEAUTO_H_
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	int GetText(int nIndex, _CSTRING_NS::CAtlString& strText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		int cchLen = GetTextLen(nIndex);
		if(cchLen == LB_ERR)
			return LB_ERR;
		int nRet = LB_ERR;
		LPTSTR lpstr = strText.GetBufferSetLength(cchLen);
		if(lpstr != NULL)
		{
			nRet = GetText(nIndex, lpstr);
			strText.ReleaseBuffer();
		}
		return nRet;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	int GetTextLen(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETTEXTLEN, nIndex, 0L);
	}

	int GetItemHeight(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_GETITEMHEIGHT, nIndex, 0L);
	}

	int SetItemHeight(int nIndex, UINT cyItemHeight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_SETITEMHEIGHT, nIndex, MAKELONG(cyItemHeight, 0));
	}

	// Settable only attributes
	void SetColumnWidth(int cxWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LB_SETCOLUMNWIDTH, cxWidth, 0L);
	}

	BOOL SetTabStops(int nTabStops, LPINT rgTabStops)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LBS_USETABSTOPS) != 0);
		return (BOOL)::SendMessage(m_hWnd, LB_SETTABSTOPS, nTabStops, (LPARAM)rgTabStops);
	}

	BOOL SetTabStops()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LBS_USETABSTOPS) != 0);
		return (BOOL)::SendMessage(m_hWnd, LB_SETTABSTOPS, 0, 0L);
	}

	BOOL SetTabStops(const int& cxEachStop)    // takes an 'int'
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LBS_USETABSTOPS) != 0);
		return (BOOL)::SendMessage(m_hWnd, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)&cxEachStop);
	}

// Operations
	int InitStorage(int nItems, UINT nBytes)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_INITSTORAGE, (WPARAM)nItems, nBytes);
	}

	void ResetContent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LB_RESETCONTENT, 0, 0L);
	}

	UINT ItemFromPoint(POINT pt, BOOL& bOutside) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dw = (DWORD)::SendMessage(m_hWnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
		bOutside = (BOOL)HIWORD(dw);
		return (UINT)LOWORD(dw);
	}

	// manipulating listbox items
	int AddString(LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_ADDSTRING, 0, (LPARAM)lpszItem);
	}

	int DeleteString(UINT nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_DELETESTRING, nIndex, 0L);
	}

	int InsertString(int nIndex, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_INSERTSTRING, nIndex, (LPARAM)lpszItem);
	}

#ifndef _WIN32_WCE
	int Dir(UINT attr, LPCTSTR lpszWildCard)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_DIR, attr, (LPARAM)lpszWildCard);
	}

	int AddFile(LPCTSTR lpstrFileName)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_ADDFILE, 0, (LPARAM)lpstrFileName);
	}
#endif // !_WIN32_WCE

	// selection helpers
	int FindString(int nStartAfter, LPCTSTR lpszItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_FINDSTRING, nStartAfter, (LPARAM)lpszItem);
	}

	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind);
	}

	int SelectString(int nStartAfter, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LB_SELECTSTRING, nStartAfter, (LPARAM)lpszItem);
	}

	int SelItemRange(BOOL bSelect, int nFirstItem, int nLastItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0);
		ATLASSERT(nFirstItem <= nLastItem);
		return bSelect ? (int)::SendMessage(m_hWnd, LB_SELITEMRANGEEX, nFirstItem, nLastItem) : (int)::SendMessage(m_hWnd, LB_SELITEMRANGEEX, nLastItem, nFirstItem);
	}

#ifdef WIN32_PLATFORM_WFSP   // SmartPhone only messages
	DWORD GetInputMode(BOOL bCurrentMode = TRUE)
	{
		return SendMessage(LB_GETINPUTMODE, 0, (LPARAM)bCurrentMode);
	}

	BOOL SetInputMode(DWORD dwMode)
	{
		return SendMessage(LB_SETINPUTMODE, 0, (LPARAM)dwMode);
	}
#endif // WIN32_PLATFORM_WFSP
};

typedef CListBoxT<ATL::CWindow>   CListBox;


///////////////////////////////////////////////////////////////////////////////
// CComboBox - client side for a Windows COMBOBOX control

#ifndef WIN32_PLATFORM_WFSP   // No COMBOBOX on SmartPhones

template <class TBase>
class CComboBoxT : public TBase
{
public:
// Constructors
	CComboBoxT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CComboBoxT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return _T("COMBOBOX");
	}

	// for entire combo box
	int GetCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0L);
	}

	int GetCurSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0L);
	}

	int SetCurSel(int nSelect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SETCURSEL, nSelect, 0L);
	}

	LCID GetLocale() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LCID)::SendMessage(m_hWnd, CB_GETLOCALE, 0, 0L);
	}

	LCID SetLocale(LCID nNewLocale)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LCID)::SendMessage(m_hWnd, CB_SETLOCALE, (WPARAM)nNewLocale, 0L);
	}

	int GetTopIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETTOPINDEX, 0, 0L);
	}

	int SetTopIndex(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SETTOPINDEX, nIndex, 0L);
	}

	UINT GetHorizontalExtent() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, CB_GETHORIZONTALEXTENT, 0, 0L);
	}

	void SetHorizontalExtent(UINT nExtent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, CB_SETHORIZONTALEXTENT, nExtent, 0L);
	}

	int GetDroppedWidth() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETDROPPEDWIDTH, 0, 0L);
	}

	int SetDroppedWidth(UINT nWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SETDROPPEDWIDTH, nWidth, 0L);
	}

#if ((WINVER >= 0x0500) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 420))
	BOOL GetComboBoxInfo(PCOMBOBOXINFO pComboBoxInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
#if ((_WIN32_WINNT >= 0x0501) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 420))
		return (BOOL)::SendMessage(m_hWnd, CB_GETCOMBOBOXINFO, 0, (LPARAM)pComboBoxInfo);
#else // !((_WIN32_WINNT >= 0x0501) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 420))
		return ::GetComboBoxInfo(m_hWnd, pComboBoxInfo);
#endif // !((_WIN32_WINNT >= 0x0501) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 420))
	}
#endif // ((WINVER >= 0x0500) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 420))

	// for edit control
	DWORD GetEditSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, CB_GETEDITSEL, 0, 0L);
	}

	BOOL SetEditSel(int nStartChar, int nEndChar)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_SETEDITSEL, 0, MAKELONG(nStartChar, nEndChar));
	}

	// for combobox item
	DWORD_PTR GetItemData(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD_PTR)::SendMessage(m_hWnd, CB_GETITEMDATA, nIndex, 0L);
	}

	int SetItemData(int nIndex, DWORD_PTR dwItemData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SETITEMDATA, nIndex, (LPARAM)dwItemData);
	}

	void* GetItemDataPtr(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (void*)GetItemData(nIndex);
	}

	int SetItemDataPtr(int nIndex, void* pData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItemData(nIndex, (DWORD_PTR)pData);
	}

	int GetLBText(int nIndex, LPTSTR lpszText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)lpszText);
	}

#ifndef _ATL_NO_COM
	BOOL GetLBTextBSTR(int nIndex, BSTR& bstrText) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrText == NULL);

		int nLen = GetLBTextLen(nIndex);
		if(nLen == CB_ERR)
			return FALSE;

		CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPTSTR lpstrText = buff.Allocate(nLen + 1);
		if(lpstrText == NULL)
			return FALSE;

		if(GetLBText(nIndex, lpstrText) == CB_ERR)
			return FALSE;

		bstrText = ::SysAllocString(T2OLE(lpstrText));
		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	int GetLBText(int nIndex, _CSTRING_NS::CAtlString& strText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		int cchLen = GetLBTextLen(nIndex);
		if(cchLen == CB_ERR)
			return CB_ERR;
		int nRet = CB_ERR;
		LPTSTR lpstr = strText.GetBufferSetLength(cchLen);
		if(lpstr != NULL)
		{
			nRet = GetLBText(nIndex, lpstr);
			strText.ReleaseBuffer();
		}
		return nRet;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	int GetLBTextLen(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETLBTEXTLEN, nIndex, 0L);
	}

	int GetItemHeight(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETITEMHEIGHT, nIndex, 0L);
	}

	int SetItemHeight(int nIndex, UINT cyItemHeight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SETITEMHEIGHT, nIndex, MAKELONG(cyItemHeight, 0));
	}

	BOOL GetExtendedUI() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_GETEXTENDEDUI, 0, 0L);
	}

	int SetExtendedUI(BOOL bExtended = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SETEXTENDEDUI, bExtended, 0L);
	}

	void GetDroppedControlRect(LPRECT lprect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)lprect);
	}

	BOOL GetDroppedState() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_GETDROPPEDSTATE, 0, 0L);
	}

#if (_WIN32_WINNT >= 0x0501)
	int GetMinVisible() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_GETMINVISIBLE, 0, 0L);
	}

	BOOL SetMinVisible(int nMinVisible)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_SETMINVISIBLE, nMinVisible, 0L);
	}

	// Vista only
	BOOL GetCueBannerText(LPWSTR lpwText, int cchText) const
	{
#ifndef CB_GETCUEBANNER
		const UINT CB_GETCUEBANNER = (CBM_FIRST + 4);
#endif
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_GETCUEBANNER, (WPARAM)lpwText, cchText);
	}

	// Vista only
	BOOL SetCueBannerText(LPCWSTR lpcwText)
	{
#ifndef CB_SETCUEBANNER
		const UINT CB_SETCUEBANNER = (CBM_FIRST + 3);
#endif
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_SETCUEBANNER, 0, (LPARAM)lpcwText);
	}
#endif // (_WIN32_WINNT >= 0x0501)

// Operations
	int InitStorage(int nItems, UINT nBytes)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_INITSTORAGE, (WPARAM)nItems, nBytes);
	}

	void ResetContent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, CB_RESETCONTENT, 0, 0L);
	}

	// for edit control
	BOOL LimitText(int nMaxChars)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CB_LIMITTEXT, nMaxChars, 0L);
	}

	// for drop-down combo boxes
	void ShowDropDown(BOOL bShowIt = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, CB_SHOWDROPDOWN, bShowIt, 0L);
	}

	// manipulating listbox items
	int AddString(LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_ADDSTRING, 0, (LPARAM)lpszString);
	}

	int DeleteString(UINT nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_DELETESTRING, nIndex, 0L);
	}

	int InsertString(int nIndex, LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_INSERTSTRING, nIndex, (LPARAM)lpszString);
	}

#ifndef _WIN32_WCE
	int Dir(UINT attr, LPCTSTR lpszWildCard)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_DIR, attr, (LPARAM)lpszWildCard);
	}
#endif // !_WIN32_WCE

	// selection helpers
	int FindString(int nStartAfter, LPCTSTR lpszString) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_FINDSTRING, nStartAfter, (LPARAM)lpszString);
	}

	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind);
	}

	int SelectString(int nStartAfter, LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CB_SELECTSTRING, nStartAfter, (LPARAM)lpszString);
	}

	// Clipboard operations
	void Clear()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_CLEAR, 0, 0L);
	}

	void Copy()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_COPY, 0, 0L);
	}

	void Cut()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_CUT, 0, 0L);
	}

	void Paste()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_PASTE, 0, 0L);
	}
};

typedef CComboBoxT<ATL::CWindow>   CComboBox;

#endif // !WIN32_PLATFORM_WFSP

///////////////////////////////////////////////////////////////////////////////
// CEdit - client side for a Windows EDIT control

template <class TBase>
class CEditT : public TBase
{
public:
// Constructors
	CEditT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CEditT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return _T("EDIT");
	}

	BOOL CanUndo() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_CANUNDO, 0, 0L);
	}

	int GetLineCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0L);
	}

	BOOL GetModify() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_GETMODIFY, 0, 0L);
	}

	void SetModify(BOOL bModified = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETMODIFY, bModified, 0L);
	}

	void GetRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_GETRECT, 0, (LPARAM)lpRect);
	}

	DWORD GetSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_GETSEL, 0, 0L);
	}

	void GetSel(int& nStartChar, int& nEndChar) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_GETSEL, (WPARAM)&nStartChar, (LPARAM)&nEndChar);
	}

#ifndef _WIN32_WCE
	HLOCAL GetHandle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HLOCAL)::SendMessage(m_hWnd, EM_GETHANDLE, 0, 0L);
	}

	void SetHandle(HLOCAL hBuffer)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETHANDLE, (WPARAM)hBuffer, 0L);
	}
#endif // !_WIN32_WCE

	DWORD GetMargins() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_GETMARGINS, 0, 0L);
	}

	void SetMargins(UINT nLeft, UINT nRight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(nLeft, nRight));
	}

	UINT GetLimitText() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, EM_GETLIMITTEXT, 0, 0L);
	}

	void SetLimitText(UINT nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETLIMITTEXT, nMax, 0L);
	}

	POINT PosFromChar(UINT nChar) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, EM_POSFROMCHAR, nChar, 0);
		POINT point = { GET_X_LPARAM(dwRet), GET_Y_LPARAM(dwRet) };
		return point;
	}

	int CharFromPos(POINT pt, int* pLine = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));
		if(pLine != NULL)
			*pLine = (int)(short)HIWORD(dwRet);
		return (int)(short)LOWORD(dwRet);
	}

	// NOTE: first word in lpszBuffer must contain the size of the buffer!
	int GetLine(int nIndex, LPTSTR lpszBuffer) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETLINE, nIndex, (LPARAM)lpszBuffer);
	}

	int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		*(LPWORD)lpszBuffer = (WORD)nMaxLength;
		return (int)::SendMessage(m_hWnd, EM_GETLINE, nIndex, (LPARAM)lpszBuffer);
	}

	TCHAR GetPasswordChar() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (TCHAR)::SendMessage(m_hWnd, EM_GETPASSWORDCHAR, 0, 0L);
	}

	void SetPasswordChar(TCHAR ch)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETPASSWORDCHAR, ch, 0L);
	}

#ifndef _WIN32_WCE
	EDITWORDBREAKPROC GetWordBreakProc() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (EDITWORDBREAKPROC)::SendMessage(m_hWnd, EM_GETWORDBREAKPROC, 0, 0L);
	}

	void SetWordBreakProc(EDITWORDBREAKPROC ewbprc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETWORDBREAKPROC, 0, (LPARAM)ewbprc);
	}
#endif // !_WIN32_WCE

	int GetFirstVisibleLine() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
	}

#ifndef _WIN32_WCE
	int GetThumb() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & ES_MULTILINE) != 0);
		return (int)::SendMessage(m_hWnd, EM_GETTHUMB, 0, 0L);
	}
#endif // !_WIN32_WCE

	BOOL SetReadOnly(BOOL bReadOnly = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETREADONLY, bReadOnly, 0L);
	}

#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
	UINT GetImeStatus(UINT uStatus) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, EM_GETIMESTATUS, uStatus, 0L);
	}

	UINT SetImeStatus(UINT uStatus, UINT uData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, EM_SETIMESTATUS, uStatus, uData);
	}
#endif // (WINVER >= 0x0500) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0501)
	BOOL GetCueBannerText(LPCWSTR lpstrText, int cchText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_GETCUEBANNER, (WPARAM)lpstrText, cchText);
	}

	// bKeepWithFocus - Vista only
	BOOL SetCueBannerText(LPCWSTR lpstrText, BOOL bKeepWithFocus = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETCUEBANNER, (WPARAM)bKeepWithFocus, (LPARAM)(lpstrText));
	}
#endif // (_WIN32_WINNT >= 0x0501)

// Operations
	void EmptyUndoBuffer()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_EMPTYUNDOBUFFER, 0, 0L);
	}

	BOOL FmtLines(BOOL bAddEOL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_FMTLINES, bAddEOL, 0L);
	}

	void LimitText(int nChars = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_LIMITTEXT, nChars, 0L);
	}

	int LineFromChar(int nIndex = -1) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_LINEFROMCHAR, nIndex, 0L);
	}

	int LineIndex(int nLine = -1) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_LINEINDEX, nLine, 0L);
	}

	int LineLength(int nLine = -1) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_LINELENGTH, nLine, 0L);
	}

	void LineScroll(int nLines, int nChars = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_LINESCROLL, nChars, nLines);
	}

	void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpszNewText);
	}

	void SetRect(LPCRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETRECT, 0, (LPARAM)lpRect);
	}

	void SetRectNP(LPCRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETRECTNP, 0, (LPARAM)lpRect);
	}

	void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETSEL, LOWORD(dwSelection), HIWORD(dwSelection));
		if(!bNoScroll)
			::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
	}

	void SetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETSEL, nStartChar, nEndChar);
		if(!bNoScroll)
			::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
	}

	void SetSelAll(BOOL bNoScroll = FALSE)
	{
		SetSel(0, -1, bNoScroll);
	}

	void SetSelNone(BOOL bNoScroll = FALSE)
	{
		SetSel(-1, 0, bNoScroll);
	}

	BOOL SetTabStops(int nTabStops, LPINT rgTabStops)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTABSTOPS, nTabStops, (LPARAM)rgTabStops);
	}

	BOOL SetTabStops()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTABSTOPS, 0, 0L);
	}

	BOOL SetTabStops(const int& cxEachStop)    // takes an 'int'
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTABSTOPS, 1, (LPARAM)(LPINT)&cxEachStop);
	}

	void ScrollCaret()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
	}

	int Scroll(int nScrollAction)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & ES_MULTILINE) != 0);
		LRESULT lRet = ::SendMessage(m_hWnd, EM_SCROLL, nScrollAction, 0L);
		if(!(BOOL)HIWORD(lRet))
			return -1;   // failed
		return (int)(short)LOWORD(lRet);
		
	}

	void InsertText(int nInsertAfterChar, LPCTSTR lpstrText, BOOL bNoScroll = FALSE, BOOL bCanUndo = FALSE)
	{
		SetSel(nInsertAfterChar, nInsertAfterChar, bNoScroll);
		ReplaceSel(lpstrText, bCanUndo);
	}

	void AppendText(LPCTSTR lpstrText, BOOL bNoScroll = FALSE, BOOL bCanUndo = FALSE)
	{
		InsertText(GetWindowTextLength(), lpstrText, bNoScroll, bCanUndo);
	}

#if (_WIN32_WINNT >= 0x0501)
	BOOL ShowBalloonTip(PEDITBALLOONTIP pEditBaloonTip)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SHOWBALLOONTIP, 0, (LPARAM)pEditBaloonTip);
	}

	BOOL HideBalloonTip()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_HIDEBALLOONTIP, 0, 0L);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_WINNT >= 0x0600)
	DWORD GetHilite() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_GETHILITE, 0, 0L);
	}

	void GetHilite(int& nStartChar, int& nEndChar) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, EM_GETHILITE, 0, 0L);
		nStartChar = (int)(short)LOWORD(dwRet);
		nEndChar = (int)(short)HIWORD(dwRet);
	}

	void SetHilite(int nStartChar, int nEndChar)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETHILITE, nStartChar, nEndChar);
	}
#endif // (_WIN32_WINNT >= 0x0600)

	// Clipboard operations
	BOOL Undo()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_UNDO, 0, 0L);
	}

	void Clear()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_CLEAR, 0, 0L);
	}

	void Copy()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_COPY, 0, 0L);
	}

	void Cut()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_CUT, 0, 0L);
	}

	void Paste()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_PASTE, 0, 0L);
	}

#ifdef WIN32_PLATFORM_WFSP   // SmartPhone only messages
	DWORD GetExtendedStyle()
	{
		return SendMessage(EM_GETEXTENDEDSTYLE);
	}

	DWORD SetExtendedStyle(DWORD dwMask, DWORD dwExStyle)
	{
		return SendMessage(EM_SETEXTENDEDSTYLE, (WPARAM)dwMask, (LPARAM)dwExStyle);
	}

	DWORD GetInputMode(BOOL bCurrentMode = TRUE)
	{
		return SendMessage(EM_GETINPUTMODE, 0, (LPARAM)bCurrentMode);
	}

	BOOL SetInputMode(DWORD dwMode)
	{
		return SendMessage(EM_SETINPUTMODE, 0, (LPARAM)dwMode);
	}

	BOOL SetSymbols(LPCTSTR szSymbols)
	{
		return SendMessage(EM_SETSYMBOLS, 0, (LPARAM)szSymbols);
	}

	BOOL ResetSymbols()
	{
		return SendMessage(EM_SETSYMBOLS);
	}
#endif // WIN32_PLATFORM_WFSP
};

typedef CEditT<ATL::CWindow>   CEdit;


///////////////////////////////////////////////////////////////////////////////
// CEditCommands - message handlers for standard EDIT commands

// Chain to CEditCommands message map. Your class must also derive from CEdit.
// Example:
// class CMyEdit : public CWindowImpl<CMyEdit, CEdit>,
//                 public CEditCommands<CMyEdit>
// {
// public:
//      BEGIN_MSG_MAP(CMyEdit)
//              // your handlers...
//              CHAIN_MSG_MAP_ALT(CEditCommands<CMyEdit>, 1)
//      END_MSG_MAP()
//      // other stuff...
// };

template <class T>
class CEditCommands
{
public:
	BEGIN_MSG_MAP(CEditCommands< T >)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnEditClear)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR_ALL, OnEditClearAll)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnEditCut)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
		COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, OnEditSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnEditUndo)
	END_MSG_MAP()

	LRESULT OnEditClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Clear();
		return 0;
	}

	LRESULT OnEditClearAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->SetSel(0, -1);
		pT->Clear();
		return 0;
	}

	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Copy();
		return 0;
	}

	LRESULT OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Cut();
		return 0;
	}

	LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Paste();
		return 0;
	}

	LRESULT OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->SetSel(0, -1);
		return 0;
	}

	LRESULT OnEditUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Undo();
		return 0;
	}

// State (update UI) helpers
	BOOL CanCut() const
	{ return HasSelection(); }

	BOOL CanCopy() const
	{ return HasSelection(); }

	BOOL CanClear() const
	{ return HasSelection(); }

	BOOL CanSelectAll() const
	{ return HasText(); }

	BOOL CanFind() const
	{ return HasText(); }

	BOOL CanRepeat() const
	{ return HasText(); }

	BOOL CanReplace() const
	{ return HasText(); }

	BOOL CanClearAll() const
	{ return HasText(); }

// Implementation
	BOOL HasSelection() const
	{
		const T* pT = static_cast<const T*>(this);
		int nMin, nMax;
		::SendMessage(pT->m_hWnd, EM_GETSEL, (WPARAM)&nMin, (LPARAM)&nMax);
		return (nMin != nMax);
	}

	BOOL HasText() const
	{
		const T* pT = static_cast<const T*>(this);
		return (pT->GetWindowTextLength() > 0);
	}
};


///////////////////////////////////////////////////////////////////////////////
// CScrollBar - client side for a Windows SCROLLBAR control

template <class TBase>
class CScrollBarT : public TBase
{
public:
// Constructors
	CScrollBarT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CScrollBarT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return _T("SCROLLBAR");
	}

#ifndef _WIN32_WCE
	int GetScrollPos() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetScrollPos(m_hWnd, SB_CTL);
	}
#endif // !_WIN32_WCE

	int SetScrollPos(int nPos, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetScrollPos(m_hWnd, SB_CTL, nPos, bRedraw);
	}

#ifndef _WIN32_WCE
	void GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::GetScrollRange(m_hWnd, SB_CTL, lpMinPos, lpMaxPos);
	}
#endif // !_WIN32_WCE

	void SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SetScrollRange(m_hWnd, SB_CTL, nMinPos, nMaxPos, bRedraw);
	}

	BOOL GetScrollInfo(LPSCROLLINFO lpScrollInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetScrollInfo(m_hWnd, SB_CTL, lpScrollInfo);
	}

	int SetScrollInfo(LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetScrollInfo(m_hWnd, SB_CTL, lpScrollInfo, bRedraw);
	}

#ifndef _WIN32_WCE
	int GetScrollLimit() const
	{
		int nMin = 0, nMax = 0;
		::GetScrollRange(m_hWnd, SB_CTL, &nMin, &nMax);
		SCROLLINFO info = { 0 };
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_PAGE;
		if(::GetScrollInfo(m_hWnd, SB_CTL, &info))
			nMax -= ((info.nPage - 1) > 0) ? (info.nPage - 1) : 0;

		return nMax;
	}

#if (WINVER >= 0x0500)
	BOOL GetScrollBarInfo(PSCROLLBARINFO pScrollBarInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
#if (_WIN32_WINNT >= 0x0501)
		return (BOOL)::SendMessage(m_hWnd, SBM_GETSCROLLBARINFO, 0, (LPARAM)pScrollBarInfo);
#else // !(_WIN32_WINNT >= 0x0501)
		return ::GetScrollBarInfo(m_hWnd, OBJID_CLIENT, pScrollBarInfo);
#endif // !(_WIN32_WINNT >= 0x0501)
	}
#endif // (WINVER >= 0x0500)

// Operations
	void ShowScrollBar(BOOL bShow = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::ShowScrollBar(m_hWnd, SB_CTL, bShow);
	}

	BOOL EnableScrollBar(UINT nArrowFlags = ESB_ENABLE_BOTH)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::EnableScrollBar(m_hWnd, SB_CTL, nArrowFlags);
	}
#endif // !_WIN32_WCE
};

typedef CScrollBarT<ATL::CWindow>   CScrollBar;


// --- Windows Common Controls ---

///////////////////////////////////////////////////////////////////////////////
// CImageList

class CImageList
{
public:
	HIMAGELIST m_hImageList;

// Constructor
	CImageList(HIMAGELIST hImageList = NULL) : m_hImageList(hImageList)
	{ }

// Operators, etc.
	CImageList& operator =(HIMAGELIST hImageList)
	{
		m_hImageList = hImageList;
		return *this;
	}

	operator HIMAGELIST() const { return m_hImageList; }

	void Attach(HIMAGELIST hImageList)
	{
		ATLASSERT(m_hImageList == NULL);
		ATLASSERT(hImageList != NULL);
		m_hImageList = hImageList;
	}

	HIMAGELIST Detach()
	{
		HIMAGELIST hImageList = m_hImageList;
		m_hImageList = NULL;
		return hImageList;
	}

	bool IsNull() const { return (m_hImageList == NULL); }

// Attributes
	int GetImageCount() const
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_GetImageCount(m_hImageList);
	}

	COLORREF GetBkColor() const
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_GetBkColor(m_hImageList);
	}

	COLORREF SetBkColor(COLORREF cr)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetBkColor(m_hImageList, cr);
	}

	BOOL GetImageInfo(int nImage, IMAGEINFO* pImageInfo) const
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_GetImageInfo(m_hImageList, nImage, pImageInfo);
	}

	HICON GetIcon(int nIndex, UINT uFlags = ILD_NORMAL) const
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_GetIcon(m_hImageList, nIndex, uFlags);
	}

	BOOL GetIconSize(int& cx, int& cy) const
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_GetIconSize(m_hImageList, &cx, &cy);
	}

	BOOL GetIconSize(SIZE& size) const
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_GetIconSize(m_hImageList, (int*)&size.cx, (int*)&size.cy);
	}

	BOOL SetIconSize(int cx, int cy)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetIconSize(m_hImageList, cx, cy);
	}

	BOOL SetIconSize(SIZE size)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetIconSize(m_hImageList, size.cx, size.cy);
	}

	BOOL SetImageCount(UINT uNewCount)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetImageCount(m_hImageList, uNewCount);
	}

	BOOL SetOverlayImage(int nImage, int nOverlay)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetOverlayImage(m_hImageList, nImage, nOverlay);
	}

// Operations
	BOOL Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow)
	{
		ATLASSERT(m_hImageList == NULL);
		m_hImageList = ImageList_Create(cx, cy, nFlags, nInitial, nGrow);
		return (m_hImageList != NULL) ? TRUE : FALSE;
	}

	BOOL Create(ATL::_U_STRINGorID bitmap, int cx, int nGrow, COLORREF crMask)
	{
		ATLASSERT(m_hImageList == NULL);
		m_hImageList = ImageList_LoadBitmap(ModuleHelper::GetResourceInstance(), bitmap.m_lpstr, cx, nGrow, crMask);
		return (m_hImageList != NULL) ? TRUE : FALSE;
	}

	BOOL CreateFromImage(ATL::_U_STRINGorID image, int cx, int nGrow, COLORREF crMask, UINT uType, UINT uFlags = LR_DEFAULTCOLOR | LR_DEFAULTSIZE)
	{
		ATLASSERT(m_hImageList == NULL);
		m_hImageList = ImageList_LoadImage(ModuleHelper::GetResourceInstance(), image.m_lpstr, cx, nGrow, crMask, uType, uFlags);
		return (m_hImageList != NULL) ? TRUE : FALSE;
	}

	BOOL Merge(HIMAGELIST hImageList1, int nImage1, HIMAGELIST hImageList2, int nImage2, int dx, int dy)
	{
		ATLASSERT(m_hImageList == NULL);
		m_hImageList = ImageList_Merge(hImageList1, nImage1, hImageList2, nImage2, dx, dy);
		return (m_hImageList != NULL) ? TRUE : FALSE;
	}

#ifndef _WIN32_WCE
#ifdef __IStream_INTERFACE_DEFINED__
	BOOL CreateFromStream(LPSTREAM lpStream)
	{
		ATLASSERT(m_hImageList == NULL);
		m_hImageList = ImageList_Read(lpStream);
		return (m_hImageList != NULL) ? TRUE : FALSE;
	}
#endif // __IStream_INTERFACE_DEFINED__
#endif // !_WIN32_WCE

	BOOL Destroy()
	{
		if (m_hImageList == NULL)
			return FALSE;
		BOOL bRet = ImageList_Destroy(m_hImageList);
		if(bRet)
			m_hImageList = NULL;
		return bRet;
	}

	int Add(HBITMAP hBitmap, HBITMAP hBitmapMask = NULL)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_Add(m_hImageList, hBitmap, hBitmapMask);
	}

	int Add(HBITMAP hBitmap, COLORREF crMask)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_AddMasked(m_hImageList, hBitmap, crMask);
	}

	BOOL Remove(int nImage)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_Remove(m_hImageList, nImage);
	}

	BOOL RemoveAll()
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_RemoveAll(m_hImageList);
	}

	BOOL Replace(int nImage, HBITMAP hBitmap, HBITMAP hBitmapMask)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_Replace(m_hImageList, nImage, hBitmap, hBitmapMask);
	}

	int AddIcon(HICON hIcon)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_AddIcon(m_hImageList, hIcon);
	}

	int ReplaceIcon(int nImage, HICON hIcon)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_ReplaceIcon(m_hImageList, nImage, hIcon);
	}

	HICON ExtractIcon(int nImage)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_ExtractIcon(NULL, m_hImageList, nImage);
	}

	BOOL Draw(HDC hDC, int nImage, int x, int y, UINT nStyle)
	{
		ATLASSERT(m_hImageList != NULL);
		ATLASSERT(hDC != NULL);
		return ImageList_Draw(m_hImageList, nImage, hDC, x, y, nStyle);
	}

	BOOL Draw(HDC hDC, int nImage, POINT pt, UINT nStyle)
	{
		ATLASSERT(m_hImageList != NULL);
		ATLASSERT(hDC != NULL);
		return ImageList_Draw(m_hImageList, nImage, hDC, pt.x, pt.y, nStyle);
	}

	BOOL DrawEx(int nImage, HDC hDC, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
	{
		ATLASSERT(m_hImageList != NULL);
		ATLASSERT(hDC != NULL);
		return ImageList_DrawEx(m_hImageList, nImage, hDC, x, y, dx, dy, rgbBk, rgbFg, fStyle);
	}

	BOOL DrawEx(int nImage, HDC hDC, RECT& rect, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
	{
		ATLASSERT(m_hImageList != NULL);
		ATLASSERT(hDC != NULL);
		return ImageList_DrawEx(m_hImageList, nImage, hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, rgbBk, rgbFg, fStyle);
	}

	static BOOL DrawIndirect(IMAGELISTDRAWPARAMS* pimldp)
	{
		return ImageList_DrawIndirect(pimldp);
	}

	BOOL Copy(int nSrc, int nDst, UINT uFlags = ILCF_MOVE)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_Copy(m_hImageList, nDst, m_hImageList, nSrc, uFlags);
	}

#ifdef __IStream_INTERFACE_DEFINED__
#ifndef _WIN32_WCE
	static HIMAGELIST Read(LPSTREAM lpStream)
	{
		return ImageList_Read(lpStream);
	}

	BOOL Write(LPSTREAM lpStream)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_Write(m_hImageList, lpStream);
	}
#endif // !_WIN32_WCE

#if (_WIN32_WINNT >= 0x0501)
	static HRESULT ReadEx(DWORD dwFlags, LPSTREAM lpStream, REFIID riid, PVOID* ppv)
	{
		return ImageList_ReadEx(dwFlags, lpStream, riid, ppv);
	}

	HRESULT WriteEx(DWORD dwFlags, LPSTREAM lpStream)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_WriteEx(m_hImageList, dwFlags, lpStream);
	}
#endif // (_WIN32_WINNT >= 0x0501)
#endif // __IStream_INTERFACE_DEFINED__

	// Drag operations
	BOOL BeginDrag(int nImage, POINT ptHotSpot)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_BeginDrag(m_hImageList, nImage, ptHotSpot.x, ptHotSpot.y);
	}

	BOOL BeginDrag(int nImage, int xHotSpot, int yHotSpot)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_BeginDrag(m_hImageList, nImage, xHotSpot, yHotSpot);
	}

	static void EndDrag()
	{
		ImageList_EndDrag();
	}

	static BOOL DragMove(POINT pt)
	{
		return ImageList_DragMove(pt.x, pt.y);
	}

	static BOOL DragMove(int x, int y)
	{
		return ImageList_DragMove(x, y);
	}

	BOOL SetDragCursorImage(int nDrag, POINT ptHotSpot)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetDragCursorImage(m_hImageList, nDrag, ptHotSpot.x, ptHotSpot.y);
	}

	BOOL SetDragCursorImage(int nDrag, int xHotSpot, int yHotSpot)
	{
		ATLASSERT(m_hImageList != NULL);
		return ImageList_SetDragCursorImage(m_hImageList, nDrag, xHotSpot, yHotSpot);
	}

	static BOOL DragShowNolock(BOOL bShow = TRUE)
	{
		return ImageList_DragShowNolock(bShow);
	}

	static CImageList GetDragImage(LPPOINT lpPoint, LPPOINT lpPointHotSpot)
	{
		return CImageList(ImageList_GetDragImage(lpPoint, lpPointHotSpot));
	}

	static BOOL DragEnter(HWND hWnd, POINT point)
	{
		return ImageList_DragEnter(hWnd, point.x, point.y);
	}

	static BOOL DragEnter(HWND hWnd, int x, int y)
	{
		return ImageList_DragEnter(hWnd, x, y);
	}

	static BOOL DragLeave(HWND hWnd)
	{
		return ImageList_DragLeave(hWnd);
	}

#if (_WIN32_IE >= 0x0400)
	CImageList Duplicate() const
	{
		ATLASSERT(m_hImageList != NULL);
		return CImageList(ImageList_Duplicate(m_hImageList));
	}

	static CImageList Duplicate(HIMAGELIST hImageList)
	{
		ATLASSERT(hImageList != NULL);
		return CImageList(ImageList_Duplicate(hImageList));
	}
#endif // (_WIN32_IE >= 0x0400)
};


///////////////////////////////////////////////////////////////////////////////
// CToolTipCtrl

#ifndef _WIN32_WCE

class CToolInfo : public TOOLINFO
{
public:
	CToolInfo(UINT nFlags, HWND hWnd, UINT nIDTool = 0, LPRECT lpRect = NULL, LPTSTR lpstrText = LPSTR_TEXTCALLBACK, LPARAM lUserParam = NULL)
	{
		Init(nFlags, hWnd, nIDTool, lpRect, lpstrText, lUserParam);
	}

	operator LPTOOLINFO() { return this; }

	operator LPARAM() { return (LPARAM)this; }

	void Init(UINT nFlags, HWND hWnd, UINT nIDTool = 0, LPRECT lpRect = NULL, LPTSTR lpstrText = LPSTR_TEXTCALLBACK, LPARAM lUserParam = NULL)
	{
		ATLASSERT(::IsWindow(hWnd));
		memset(this, 0, sizeof(TOOLINFO));
		cbSize = sizeof(TOOLINFO);
		uFlags = nFlags;
		if(nIDTool == 0)
		{
			hwnd = ::GetParent(hWnd);
			uFlags |= TTF_IDISHWND;
			uId = (UINT_PTR)hWnd;
		}
		else
		{
			hwnd = hWnd;
			uId = nIDTool;
		}
		if(lpRect != NULL)
			rect = *lpRect;
		hinst = ModuleHelper::GetResourceInstance();
		lpszText = lpstrText;
		lParam = lUserParam;
	}
};

template <class TBase>
class CToolTipCtrlT : public TBase
{
public:
// Constructors
	CToolTipCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CToolTipCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return TOOLTIPS_CLASS;
	}

	void GetText(LPTOOLINFO lpToolInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_GETTEXT, 0, (LPARAM)&lpToolInfo);
	}

	void GetText(LPTSTR lpstrText, HWND hWnd, UINT nIDTool = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);
		CToolInfo ti(0, hWnd, nIDTool, NULL, lpstrText);
		::SendMessage(m_hWnd, TTM_GETTEXT, 0, ti);
	}

	BOOL GetToolInfo(LPTOOLINFO lpToolInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_GETTOOLINFO, 0, (LPARAM)lpToolInfo);
	}

	BOOL GetToolInfo(HWND hWnd, UINT nIDTool, UINT* puFlags, LPRECT lpRect, LPTSTR lpstrText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);
		ATLASSERT(puFlags != NULL);
		ATLASSERT(lpRect != NULL);
		CToolInfo ti(0, hWnd, nIDTool, NULL, lpstrText);
		BOOL bRet = (BOOL)::SendMessage(m_hWnd, TTM_GETTOOLINFO, 0, ti);
		if(bRet != FALSE)
		{
			*puFlags = ti.uFlags;
			*lpRect = ti.rect;
		}
		return bRet;
	}

	void SetToolInfo(LPTOOLINFO lpToolInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_SETTOOLINFO, 0, (LPARAM)lpToolInfo);
	}

	void SetToolRect(LPTOOLINFO lpToolInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_NEWTOOLRECT, 0, (LPARAM)lpToolInfo);
	}

	void SetToolRect(HWND hWnd, UINT nIDTool, LPCRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);
		ATLASSERT(nIDTool != 0);

		CToolInfo ti(0, hWnd, nIDTool, (LPRECT)lpRect, NULL);
		::SendMessage(m_hWnd, TTM_NEWTOOLRECT, 0, ti);
	}

	int GetToolCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TTM_GETTOOLCOUNT, 0, 0L);
	}

	int GetDelayTime(DWORD dwType) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TTM_GETDELAYTIME, dwType, 0L);
	}

	void SetDelayTime(DWORD dwType, int nTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_SETDELAYTIME, dwType, MAKELPARAM(nTime, 0));
	}

	void GetMargin(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_GETMARGIN, 0, (LPARAM)lpRect);
	}

	void SetMargin(LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_SETMARGIN, 0, (LPARAM)lpRect);
	}

	int GetMaxTipWidth() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TTM_GETMAXTIPWIDTH, 0, 0L);
	}

	int SetMaxTipWidth(int nWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TTM_SETMAXTIPWIDTH, 0, nWidth);
	}

	COLORREF GetTipBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TTM_GETTIPBKCOLOR, 0, 0L);
	}

	void SetTipBkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_SETTIPBKCOLOR, (WPARAM)clr, 0L);
	}

	COLORREF GetTipTextColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TTM_GETTIPTEXTCOLOR, 0, 0L);
	}

	void SetTipTextColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_SETTIPTEXTCOLOR, (WPARAM)clr, 0L);
	}

	BOOL GetCurrentTool(LPTOOLINFO lpToolInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_GETCURRENTTOOL, 0, (LPARAM)lpToolInfo);
	}

#if (_WIN32_IE >= 0x0500)
	SIZE GetBubbleSize(LPTOOLINFO lpToolInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, TTM_GETBUBBLESIZE, 0, (LPARAM)lpToolInfo);
		SIZE size = { GET_X_LPARAM(dwRet), GET_Y_LPARAM(dwRet) };
		return size;
	}

	BOOL SetTitle(UINT uIcon, LPCTSTR lpstrTitle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_SETTITLE, uIcon, (LPARAM)lpstrTitle);
	}
#endif // (_WIN32_IE >= 0x0500)

#if (_WIN32_WINNT >= 0x0501)
	void GetTitle(PTTGETTITLE pTTGetTitle) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_GETTITLE, 0, (LPARAM)pTTGetTitle);
	}

	void SetWindowTheme(LPCWSTR lpstrTheme)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_SETWINDOWTHEME, 0, (LPARAM)lpstrTheme);
	}
#endif // (_WIN32_WINNT >= 0x0501)

// Operations
	void Activate(BOOL bActivate)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_ACTIVATE, bActivate, 0L);
	}

	BOOL AddTool(LPTOOLINFO lpToolInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM)lpToolInfo);
	}

	BOOL AddTool(HWND hWnd, ATL::_U_STRINGorID text = LPSTR_TEXTCALLBACK, LPCRECT lpRectTool = NULL, UINT nIDTool = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);
		// the toolrect and toolid must both be zero or both valid
		ATLASSERT((lpRectTool != NULL && nIDTool != 0) || (lpRectTool == NULL && nIDTool == 0));

		CToolInfo ti(0, hWnd, nIDTool, (LPRECT)lpRectTool, (LPTSTR)text.m_lpstr);
		return (BOOL)::SendMessage(m_hWnd, TTM_ADDTOOL, 0, ti);
	}

	void DelTool(LPTOOLINFO lpToolInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM)lpToolInfo);
	}

	void DelTool(HWND hWnd, UINT nIDTool = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);

		CToolInfo ti(0, hWnd, nIDTool, NULL, NULL);
		::SendMessage(m_hWnd, TTM_DELTOOL, 0, ti);
	}

	BOOL HitTest(LPTTHITTESTINFO lpHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_HITTEST, 0, (LPARAM)lpHitTestInfo);
	}

	BOOL HitTest(HWND hWnd, POINT pt, LPTOOLINFO lpToolInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);
		ATLASSERT(lpToolInfo != NULL);

		TTHITTESTINFO hti = { 0 };
		hti.ti.cbSize = sizeof(TOOLINFO);
		hti.hwnd = hWnd;
		hti.pt.x = pt.x;
		hti.pt.y = pt.y;
		if((BOOL)::SendMessage(m_hWnd, TTM_HITTEST, 0, (LPARAM)&hti) != FALSE)
		{
			*lpToolInfo = hti.ti;
			return TRUE;
		}
		return FALSE;
	}

	void RelayEvent(LPMSG lpMsg)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_RELAYEVENT, 0, (LPARAM)lpMsg);
	}

	void UpdateTipText(LPTOOLINFO lpToolInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)lpToolInfo);
	}

	void UpdateTipText(ATL::_U_STRINGorID text, HWND hWnd, UINT nIDTool = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hWnd != NULL);

		CToolInfo ti(0, hWnd, nIDTool, NULL, (LPTSTR)text.m_lpstr);
		::SendMessage(m_hWnd, TTM_UPDATETIPTEXT, 0, ti);
	}

	BOOL EnumTools(UINT nTool, LPTOOLINFO lpToolInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_ENUMTOOLS, nTool, (LPARAM)lpToolInfo);
	}

	void Pop()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_POP, 0, 0L);
	}

	void TrackActivate(LPTOOLINFO lpToolInfo, BOOL bActivate)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_TRACKACTIVATE, bActivate, (LPARAM)lpToolInfo);
	}

	void TrackPosition(int xPos, int yPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_TRACKPOSITION, 0, MAKELPARAM(xPos, yPos));
	}

#if (_WIN32_IE >= 0x0400)
	void Update()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_UPDATE, 0, 0L);
	}
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0500)
	BOOL AdjustRect(LPRECT lpRect, BOOL bLarger /*= TRUE*/)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TTM_ADJUSTRECT, bLarger, (LPARAM)lpRect);
	}
#endif // (_WIN32_IE >= 0x0500)

#if (_WIN32_WINNT >= 0x0501)
	void Popup()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TTM_POPUP, 0, 0L);
	}
#endif // (_WIN32_WINNT >= 0x0501)
};

typedef CToolTipCtrlT<ATL::CWindow>   CToolTipCtrl;

#endif // !_WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CHeaderCtrl

template <class TBase>
class CHeaderCtrlT : public TBase
{
public:
// Constructors
	CHeaderCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CHeaderCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_HEADER;
	}

	int GetItemCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_GETITEMCOUNT, 0, 0L);
	}

	BOOL GetItem(int nIndex, LPHDITEM pHeaderItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_GETITEM, nIndex, (LPARAM)pHeaderItem);
	}

	BOOL SetItem(int nIndex, LPHDITEM pHeaderItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_SETITEM, nIndex, (LPARAM)pHeaderItem);
	}

	CImageList GetImageList() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, HDM_GETIMAGELIST, 0, 0L));
	}

	CImageList SetImageList(HIMAGELIST hImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, HDM_SETIMAGELIST, 0, (LPARAM)hImageList));
	}

	BOOL GetOrderArray(int nSize, int* lpnArray) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_GETORDERARRAY, nSize, (LPARAM)lpnArray);
	}

	BOOL SetOrderArray(int nSize, int* lpnArray)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_SETORDERARRAY, nSize, (LPARAM)lpnArray);
	}

	BOOL GetItemRect(int nIndex, LPRECT lpItemRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_GETITEMRECT, nIndex, (LPARAM)lpItemRect);
	}

	int SetHotDivider(BOOL bPos, DWORD dwInputValue)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_SETHOTDIVIDER, bPos, dwInputValue);
	}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	int GetBitmapMargin() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_GETBITMAPMARGIN, 0, 0L);
	}

	int SetBitmapMargin(int nWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_SETBITMAPMARGIN, nWidth, 0L);
	}

	int SetFilterChangeTimeout(DWORD dwTimeOut)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_SETFILTERCHANGETIMEOUT, 0, dwTimeOut);
	}
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0600)
	BOOL GetItemDropDownRect(int nIndex, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_GETITEMDROPDOWNRECT, nIndex, (LPARAM)lpRect);
	}

	BOOL GetOverflowRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_GETOVERFLOWRECT, 0, (LPARAM)lpRect);
	}

	int GetFocusedItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_GETFOCUSEDITEM, 0, 0L);
	}

	BOOL SetFocusedItem(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_SETFOCUSEDITEM, 0, nIndex);
	}
#endif // (_WIN32_WINNT >= 0x0600)

// Operations
	int InsertItem(int nIndex, LPHDITEM phdi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_INSERTITEM, nIndex, (LPARAM)phdi);
	}

	int AddItem(LPHDITEM phdi)
	{
		return InsertItem(GetItemCount(), phdi);
	}

	BOOL DeleteItem(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_DELETEITEM, nIndex, 0L);
	}

	BOOL Layout(HD_LAYOUT* pHeaderLayout)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, HDM_LAYOUT, 0, (LPARAM)pHeaderLayout);
	}

	int HitTest(LPHDHITTESTINFO lpHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_HITTEST, 0, (LPARAM)lpHitTestInfo);
	}

	int OrderToIndex(int nOrder)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_ORDERTOINDEX, nOrder, 0L);
	}

	CImageList CreateDragImage(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, HDM_CREATEDRAGIMAGE, nIndex, 0L));
	}

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	int EditFilter(int nColumn, BOOL bDiscardChanges)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_EDITFILTER, nColumn, MAKELPARAM(bDiscardChanges, 0));
	}

	int ClearFilter(int nColumn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_CLEARFILTER, nColumn, 0L);
	}

	int ClearAllFilters()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, HDM_CLEARFILTER, (WPARAM)-1, 0L);
	}
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
};

typedef CHeaderCtrlT<ATL::CWindow>   CHeaderCtrl;


///////////////////////////////////////////////////////////////////////////////
// CListViewCtrl

template <class TBase>
class CListViewCtrlT : public TBase
{
public:
// Constructors
	CListViewCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CListViewCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LISTVIEW;
	}

	COLORREF GetBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_GETBKCOLOR, 0, 0L);
	}

	BOOL SetBkColor(COLORREF cr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETBKCOLOR, 0, cr);
	}

	CImageList GetImageList(int nImageListType) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, LVM_GETIMAGELIST, nImageListType, 0L));
	}

	CImageList SetImageList(HIMAGELIST hImageList, int nImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, LVM_SETIMAGELIST, nImageList, (LPARAM)hImageList));
	}

	int GetItemCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0L);
	}

	BOOL SetItemCount(int nItems)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMCOUNT, nItems, 0L);
	}

	BOOL GetItem(LPLVITEM pItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEM, 0, (LPARAM)pItem);
	}

	BOOL SetItem(const LVITEM* pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEM, 0, (LPARAM)pItem);
	}

	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
		int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvi = { 0 };
		lvi.mask = nMask;
		lvi.iItem = nItem;
		lvi.iSubItem = nSubItem;
		lvi.stateMask = nStateMask;
		lvi.state = nState;
		lvi.pszText = (LPTSTR) lpszItem;
		lvi.iImage = nImage;
		lvi.lParam = lParam;
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvi);
	}

	UINT GetItemState(int nItem, UINT nMask) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, LVM_GETITEMSTATE, nItem, nMask);
	}

	BOOL SetItemState(int nItem, UINT nState, UINT nStateMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvi = { 0 };
		lvi.state = nState;
		lvi.stateMask = nStateMask;
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
	}

	BOOL SetItemState(int nItem, LPLVITEM pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)pItem);
	}

#ifndef _ATL_NO_COM
	BOOL GetItemText(int nItem, int nSubItem, BSTR& bstrText) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrText == NULL);
		LVITEM lvi = { 0 };
		lvi.iSubItem = nSubItem;

		LPTSTR lpstrText = NULL;
		int nRes = 0;
		for(int nLen = 256; ; nLen *= 2)
		{
			ATLTRY(lpstrText = new TCHAR[nLen]);
			if(lpstrText == NULL)
				break;
			lpstrText[0] = NULL;
			lvi.cchTextMax = nLen;
			lvi.pszText = lpstrText;
			nRes  = (int)::SendMessage(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi);
			if(nRes < nLen - 1)
				break;
			delete [] lpstrText;
			lpstrText = NULL;
		}

		if(lpstrText != NULL)
		{
			if(nRes != 0)
				bstrText = ::SysAllocString(T2OLE(lpstrText));
			delete [] lpstrText;
		}

		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	int GetItemText(int nItem, int nSubItem, _CSTRING_NS::CAtlString& strText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvi = { 0 };
		lvi.iSubItem = nSubItem;

		strText.Empty();
		int nRes = 0;
		for(int nLen = 256; ; nLen *= 2)
		{
			lvi.cchTextMax = nLen;
			lvi.pszText = strText.GetBufferSetLength(nLen);
			if(lvi.pszText == NULL)
			{
				nRes = 0;
				break;
			}
			nRes  = (int)::SendMessage(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi);
			if(nRes < nLen - 1)
				break;
		}
		strText.ReleaseBuffer();
		return nRes;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvi = { 0 };
		lvi.iSubItem = nSubItem;
		lvi.cchTextMax = nLen;
		lvi.pszText = lpszText;
		return (int)::SendMessage(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi);
	}

	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(nItem, nSubItem, LVIF_TEXT, lpszText, 0, 0, 0, 0);
	}

	DWORD_PTR GetItemData(int nItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvi = { 0 };
		lvi.iItem = nItem;
		lvi.mask = LVIF_PARAM;
		BOOL bRet = (BOOL)::SendMessage(m_hWnd, LVM_GETITEM, 0, (LPARAM)&lvi);
		return (DWORD_PTR)(bRet ? lvi.lParam : NULL);
	}

	BOOL SetItemData(int nItem, DWORD_PTR dwData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(nItem, 0, LVIF_PARAM, NULL, 0, 0, 0, (LPARAM)dwData);
	}

	UINT GetCallbackMask() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, LVM_GETCALLBACKMASK, 0, 0L);
	}

	BOOL SetCallbackMask(UINT nMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETCALLBACKMASK, nMask, 0L);
	}

	BOOL GetItemPosition(int nItem, LPPOINT lpPoint) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEMPOSITION, nItem, (LPARAM)lpPoint);
	}

	BOOL SetItemPosition(int nItem, POINT pt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(((GetStyle() & LVS_TYPEMASK) == LVS_ICON) || ((GetStyle() & LVS_TYPEMASK) == LVS_SMALLICON));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMPOSITION32, nItem, (LPARAM)&pt);
	}

	BOOL SetItemPosition(int nItem, int x, int y)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(((GetStyle() & LVS_TYPEMASK) == LVS_ICON) || ((GetStyle() & LVS_TYPEMASK) == LVS_SMALLICON));
		POINT pt = { x, y };
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMPOSITION32, nItem, (LPARAM)&pt);
	}

	int GetStringWidth(LPCTSTR lpsz) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETSTRINGWIDTH, 0, (LPARAM)lpsz);
	}

	CEdit GetEditControl() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CEdit((HWND)::SendMessage(m_hWnd, LVM_GETEDITCONTROL, 0, 0L));
	}

	BOOL GetColumn(int nCol, LVCOLUMN* pColumn) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETCOLUMN, nCol, (LPARAM)pColumn);
	}

	BOOL SetColumn(int nCol, const LVCOLUMN* pColumn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETCOLUMN, nCol, (LPARAM)pColumn);
	}

	int GetColumnWidth(int nCol) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETCOLUMNWIDTH, nCol, 0L);
	}

	BOOL SetColumnWidth(int nCol, int cx)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETCOLUMNWIDTH, nCol, MAKELPARAM(cx, 0));
	}

	BOOL GetViewRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETVIEWRECT, 0, (LPARAM)lpRect);
	}

	COLORREF GetTextColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_GETTEXTCOLOR, 0, 0L);
	}

	BOOL SetTextColor(COLORREF cr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETTEXTCOLOR, 0, cr);
	}

	COLORREF GetTextBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_GETTEXTBKCOLOR, 0, 0L);
	}

	BOOL SetTextBkColor(COLORREF cr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETTEXTBKCOLOR, 0, cr);
	}

	int GetTopIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETTOPINDEX, 0, 0L);
	}

	int GetCountPerPage() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETCOUNTPERPAGE, 0, 0L);
	}

	BOOL GetOrigin(LPPOINT lpPoint) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETORIGIN, 0, (LPARAM)lpPoint);
	}

	UINT GetSelectedCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, LVM_GETSELECTEDCOUNT, 0, 0L);
	}

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		lpRect->left = nCode;
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEMRECT, (WPARAM)nItem, (LPARAM)lpRect);
	}

#ifndef _WIN32_WCE
	HCURSOR GetHotCursor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HCURSOR)::SendMessage(m_hWnd, LVM_GETHOTCURSOR, 0, 0L);
	}

	HCURSOR SetHotCursor(HCURSOR hHotCursor)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HCURSOR)::SendMessage(m_hWnd, LVM_SETHOTCURSOR, 0, (LPARAM)hHotCursor);
	}

	int GetHotItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETHOTITEM, 0, 0L);
	}

	int SetHotItem(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SETHOTITEM, nIndex, 0L);
	}
#endif // !_WIN32_WCE

	BOOL GetColumnOrderArray(int nCount, int* lpnArray) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETCOLUMNORDERARRAY, nCount, (LPARAM)lpnArray);
	}

	BOOL SetColumnOrderArray(int nCount, int* lpnArray)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETCOLUMNORDERARRAY, nCount, (LPARAM)lpnArray);
	}

	CHeaderCtrl GetHeader() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CHeaderCtrl((HWND)::SendMessage(m_hWnd, LVM_GETHEADER, 0, 0L));
	}

	BOOL GetSubItemRect(int nItem, int nSubItem, int nFlag, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LVS_TYPEMASK) == LVS_REPORT);
		ATLASSERT(lpRect != NULL);
		lpRect->top = nSubItem;
		lpRect->left = nFlag;
		return (BOOL)::SendMessage(m_hWnd, LVM_GETSUBITEMRECT, nItem, (LPARAM)lpRect);
	}

	DWORD SetIconSpacing(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LVS_TYPEMASK) == LVS_ICON);
		return (DWORD)::SendMessage(m_hWnd, LVM_SETICONSPACING, 0, MAKELPARAM(cx, cy));
	}

	int GetISearchString(LPTSTR lpstr) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETISEARCHSTRING, 0, (LPARAM)lpstr);
	}

	void GetItemSpacing(SIZE& sizeSpacing, BOOL bSmallIconView = FALSE) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, LVM_GETITEMSPACING, bSmallIconView, 0L);
		sizeSpacing.cx = GET_X_LPARAM(dwRet);
		sizeSpacing.cy = GET_Y_LPARAM(dwRet);
	}

#if (_WIN32_WCE >= 410)
	void SetItemSpacing(INT cySpacing)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ListView_SetItemSpacing(m_hWnd, cySpacing);
	}
#endif // (_WIN32_WCE >= 410)

	// single-selection only
	int GetSelectedIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LVS_SINGLESEL) != 0);
		return (int)::SendMessage(m_hWnd, LVM_GETNEXTITEM, (WPARAM)-1, MAKELPARAM(LVNI_ALL | LVNI_SELECTED, 0));
	}

	BOOL GetSelectedItem(LPLVITEM pItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & LVS_SINGLESEL) != 0);
		ATLASSERT(pItem != NULL);
		pItem->iItem = (int)::SendMessage(m_hWnd, LVM_GETNEXTITEM, (WPARAM)-1, MAKELPARAM(LVNI_ALL | LVNI_SELECTED, 0));
		if(pItem->iItem == -1)
			return FALSE;
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEM, 0, (LPARAM)pItem);
	}

	// extended list view styles
	DWORD GetExtendedListViewStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0L);
	}

	// dwExMask = 0 means all styles
	DWORD SetExtendedListViewStyle(DWORD dwExStyle, DWORD dwExMask = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, dwExMask, dwExStyle);
	}

	// checkboxes only
	BOOL GetCheckState(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetExtendedListViewStyle() & LVS_EX_CHECKBOXES) != 0);
		UINT uRet = GetItemState(nIndex, LVIS_STATEIMAGEMASK);
		return (uRet >> 12) - 1;
	}

	BOOL SetCheckState(int nItem, BOOL bCheck)
	{
		int nCheck = bCheck ? 2 : 1;   // one based index
		return SetItemState(nItem, INDEXTOSTATEIMAGEMASK(nCheck), LVIS_STATEIMAGEMASK);
	}

	// view type
	DWORD GetViewType() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (GetStyle() & LVS_TYPEMASK);
	}

	DWORD SetViewType(DWORD dwType)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(dwType == LVS_ICON || dwType == LVS_SMALLICON || dwType == LVS_LIST || dwType == LVS_REPORT);
		DWORD dwOldType = GetViewType();
		if(dwType != dwOldType)
			ModifyStyle(LVS_TYPEMASK, (dwType & LVS_TYPEMASK));
		return dwOldType;
	}

#if (_WIN32_IE >= 0x0400)
#ifndef _WIN32_WCE
	BOOL GetBkImage(LPLVBKIMAGE plvbki) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETBKIMAGE, 0, (LPARAM)plvbki);
	}

	BOOL SetBkImage(LPLVBKIMAGE plvbki)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETBKIMAGE, 0, (LPARAM)plvbki);
	}
#endif // !_WIN32_WCE

	int GetSelectionMark() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETSELECTIONMARK, 0, 0L);
	}

	int SetSelectionMark(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SETSELECTIONMARK, 0, nIndex);
	}

#ifndef _WIN32_WCE
	BOOL GetWorkAreas(int nWorkAreas, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETWORKAREAS, nWorkAreas, (LPARAM)lpRect);
	}

	BOOL SetWorkAreas(int nWorkAreas, LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETWORKAREAS, nWorkAreas, (LPARAM)lpRect);
	}

	DWORD GetHoverTime() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetExtendedListViewStyle() & (LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TWOCLICKACTIVATE)) != 0);
		return (DWORD)::SendMessage(m_hWnd, LVM_GETHOVERTIME, 0, 0L);
	}

	DWORD SetHoverTime(DWORD dwHoverTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetExtendedListViewStyle() & (LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TWOCLICKACTIVATE)) != 0);
		return (DWORD)::SendMessage(m_hWnd, LVM_SETHOVERTIME, 0, dwHoverTime);
	}

	BOOL GetNumberOfWorkAreas(int* pnWorkAreas) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETNUMBEROFWORKAREAS, 0, (LPARAM)pnWorkAreas);
	}
#endif // !_WIN32_WCE

	BOOL SetItemCountEx(int nItems, DWORD dwFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(((GetStyle() & LVS_OWNERDATA) != 0) && (((GetStyle() & LVS_TYPEMASK) == LVS_REPORT) || ((GetStyle() & LVS_TYPEMASK) == LVS_LIST)));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMCOUNT, nItems, dwFlags);
	}

#ifndef _WIN32_WCE
	CToolTipCtrl GetToolTips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, LVM_GETTOOLTIPS, 0, 0L));
	}

	CToolTipCtrl SetToolTips(HWND hWndTT)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, LVM_SETTOOLTIPS, (WPARAM)hWndTT, 0L));
	}

	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_WINNT >= 0x0501)
	int GetSelectedColumn() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETSELECTEDCOLUMN, 0, 0L);
	}

	void SetSelectedColumn(int nColumn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_SETSELECTEDCOLUMN, nColumn, 0L);
	}

	DWORD GetView() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, LVM_GETVIEW, 0, 0L);
	}

	int SetView(DWORD dwView)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SETVIEW, dwView, 0L);
	}

	BOOL IsGroupViewEnabled() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_ISGROUPVIEWENABLED, 0, 0L);
	}

	int GetGroupInfo(int nGroupID, PLVGROUP pGroup) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETGROUPINFO, nGroupID, (LPARAM)pGroup);
	}

	int SetGroupInfo(int nGroupID, PLVGROUP pGroup)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SETGROUPINFO, nGroupID, (LPARAM)pGroup);
	}

	void GetGroupMetrics(PLVGROUPMETRICS pGroupMetrics) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_GETGROUPMETRICS, 0, (LPARAM)pGroupMetrics);
	}

	void SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_SETGROUPMETRICS, 0, (LPARAM)pGroupMetrics);
	}

	void GetTileViewInfo(PLVTILEVIEWINFO pTileViewInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_GETTILEVIEWINFO, 0, (LPARAM)pTileViewInfo);
	}

	BOOL SetTileViewInfo(PLVTILEVIEWINFO pTileViewInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETTILEVIEWINFO, 0, (LPARAM)pTileViewInfo);
	}

	void GetTileInfo(PLVTILEINFO pTileInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_GETTILEINFO, 0, (LPARAM)pTileInfo);
	}

	BOOL SetTileInfo(PLVTILEINFO pTileInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETTILEINFO, 0, (LPARAM)pTileInfo);
	}

	BOOL GetInsertMark(LPLVINSERTMARK pInsertMark) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETINSERTMARK, 0, (LPARAM)pInsertMark);
	}

	BOOL SetInsertMark(LPLVINSERTMARK pInsertMark)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETINSERTMARK, 0, (LPARAM)pInsertMark);
	}

	int GetInsertMarkRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETINSERTMARKRECT, 0, (LPARAM)lpRect);
	}

	COLORREF GetInsertMarkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_GETINSERTMARKCOLOR, 0, 0L);
	}

	COLORREF SetInsertMarkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_SETINSERTMARKCOLOR, 0, clr);
	}

	COLORREF GetOutlineColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_GETOUTLINECOLOR, 0, 0L);
	}

	COLORREF SetOutlineColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, LVM_SETOUTLINECOLOR, 0, clr);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_WINNT >= 0x0600)
	int GetGroupCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETGROUPCOUNT, 0, 0L);
	}

	BOOL GetGroupInfoByIndex(int nIndex, PLVGROUP pGroup) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETGROUPINFOBYINDEX, nIndex, (LPARAM)pGroup);
	}

	BOOL GetGroupRect(int nGroupID, int nType, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpRect != NULL);
		if(lpRect != NULL)
			lpRect->top = nType;
		return (BOOL)::SendMessage(m_hWnd, LVM_GETGROUPRECT, nGroupID, (LPARAM)lpRect);
	}

	UINT GetGroupState(int nGroupID, UINT uMask) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, LVM_GETGROUPSTATE, nGroupID, (LPARAM)uMask);
	}

	int GetFocusedGroup() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETFOCUSEDGROUP, 0, 0L);
	}

	BOOL GetEmptyText(LPWSTR lpstrText, int cchText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETEMPTYTEXT, cchText, (LPARAM)lpstrText);
	}

	BOOL GetFooterRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETFOOTERRECT, 0, (LPARAM)lpRect);
	}

	BOOL GetFooterInfo(LPLVFOOTERINFO lpFooterInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETFOOTERINFO, 0, (LPARAM)lpFooterInfo);
	}

	BOOL GetFooterItemRect(int nItem, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETFOOTERITEMRECT, nItem, (LPARAM)lpRect);
	}

	BOOL GetFooterItem(int nItem, LPLVFOOTERITEM lpFooterItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETFOOTERITEM, nItem, (LPARAM)lpFooterItem);
	}

	BOOL GetItemIndexRect(PLVITEMINDEX pItemIndex, int nSubItem, int nType, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pItemIndex != NULL);
		ATLASSERT(lpRect != NULL);
		if(lpRect != NULL)
		{
			lpRect->top = nSubItem;
			lpRect->left = nType;
		}
		return (BOOL)::SendMessage(m_hWnd, LVM_GETITEMINDEXRECT, (WPARAM)pItemIndex, (LPARAM)lpRect);
	}

	BOOL SetItemIndexState(PLVITEMINDEX pItemIndex, UINT uState, UINT dwMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvi = { 0 };
		lvi.state = uState;
		lvi.stateMask = dwMask;
		return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMINDEXSTATE, (WPARAM)pItemIndex, (LPARAM)&lvi);
	}

	BOOL GetNextItemIndex(PLVITEMINDEX pItemIndex, WORD wFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_GETNEXTITEMINDEX, (WPARAM)pItemIndex, MAKELPARAM(wFlags, 0));
	}
#endif // (_WIN32_WINNT >= 0x0600)

// Operations
	int InsertColumn(int nCol, const LVCOLUMN* pColumn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_INSERTCOLUMN, nCol, (LPARAM)pColumn);
	}

	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, 
			int nWidth = -1, int nSubItem = -1, int iImage = -1, int iOrder = -1)
	{
		LVCOLUMN column = { 0 };
		column.mask = LVCF_TEXT|LVCF_FMT;
		column.pszText = (LPTSTR)lpszColumnHeading;
		column.fmt = nFormat;
		if (nWidth != -1)
		{
			column.mask |= LVCF_WIDTH;
			column.cx = nWidth;
		}
		if (nSubItem != -1)
		{
			column.mask |= LVCF_SUBITEM;
			column.iSubItem = nSubItem;
		}
		if (iImage != -1)
		{
			column.mask |= LVCF_IMAGE;
			column.iImage = iImage;
		}
		if (iOrder != -1)
		{
			column.mask |= LVCF_ORDER;
			column.iOrder = iOrder;
		}
		return InsertColumn(nCol, &column);
	}

	BOOL DeleteColumn(int nCol)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_DELETECOLUMN, nCol, 0L);
	}

	int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM item = { 0 };
		item.mask = nMask;
		item.iItem = nItem;
		item.iSubItem = 0;
		item.pszText = (LPTSTR)lpszItem;
		item.state = nState;
		item.stateMask = nStateMask;
		item.iImage = nImage;
		item.lParam = lParam;
		return InsertItem(&item);
	}

	int InsertItem(const LVITEM* pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)pItem);
	}

	int InsertItem(int nItem, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return InsertItem(LVIF_TEXT, nItem, lpszItem, 0, 0, 0, 0);
	}

	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return InsertItem(LVIF_TEXT|LVIF_IMAGE, nItem, lpszItem, 0, 0, nImage, 0);
	}

	int GetNextItem(int nItem, int nFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_GETNEXTITEM, nItem, MAKELPARAM(nFlags, 0));
	}

	BOOL DeleteItem(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_DELETEITEM, nItem, 0L);
	}

	BOOL DeleteAllItems()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0L);
	}

	int FindItem(LVFINDINFO* pFindInfo, int nStart) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_FINDITEM, nStart, (LPARAM)pFindInfo);
	}

	int HitTest(LVHITTESTINFO* pHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_HITTEST, 0, (LPARAM)pHitTestInfo);
	}

	int HitTest(POINT pt, UINT* pFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		int nRes = (int)::SendMessage(m_hWnd, LVM_HITTEST, 0, (LPARAM)&hti);
		if (pFlags != NULL)
			*pFlags = hti.flags;
		return nRes;
	}

	BOOL EnsureVisible(int nItem, BOOL bPartialOK)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_ENSUREVISIBLE, nItem, MAKELPARAM(bPartialOK, 0));
	}

	BOOL Scroll(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SCROLL, size.cx, size.cy);
	}

	BOOL RedrawItems(int nFirst, int nLast)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_REDRAWITEMS, nFirst, nLast);
	}

	BOOL Arrange(UINT nCode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_ARRANGE, nCode, 0L);
	}

	CEdit EditLabel(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CEdit((HWND)::SendMessage(m_hWnd, LVM_EDITLABEL, nItem, 0L));
	}

	BOOL Update(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_UPDATE, nItem, 0L);
	}

	BOOL SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SORTITEMS, (WPARAM)lParamSort, (LPARAM)pfnCompare);
	}

	CImageList RemoveImageList(int nImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, LVM_SETIMAGELIST, (WPARAM)nImageList, NULL));
	}

	CImageList CreateDragImage(int nItem, LPPOINT lpPoint)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, LVM_CREATEDRAGIMAGE, nItem, (LPARAM)lpPoint));
	}

	DWORD ApproximateViewRect(int cx = -1, int cy = -1, int nCount = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, LVM_APPROXIMATEVIEWRECT, nCount, MAKELPARAM(cx, cy));
	}

	int SubItemHitTest(LPLVHITTESTINFO lpInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SUBITEMHITTEST, 0, (LPARAM)lpInfo);
	}

	int AddColumn(LPCTSTR strItem, int nItem, int nSubItem = -1,
			int nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
			int nFmt = LVCFMT_LEFT)
	{
		const int cxOffset = 15;
		ATLASSERT(::IsWindow(m_hWnd));
		LVCOLUMN lvc = { 0 };
		lvc.mask = nMask;
		lvc.fmt = nFmt;
		lvc.pszText = (LPTSTR)strItem;
		lvc.cx = GetStringWidth(lvc.pszText) + cxOffset;
		if(nMask & LVCF_SUBITEM)
			lvc.iSubItem = (nSubItem != -1) ? nSubItem : nItem;
		return InsertColumn(nItem, &lvc);
	}

	int AddItem(int nItem, int nSubItem, LPCTSTR strItem, int nImageIndex = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVITEM lvItem = { 0 };
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = nItem;
		lvItem.iSubItem = nSubItem;
		lvItem.pszText = (LPTSTR)strItem;
		if(nImageIndex != -1)
		{
			lvItem.mask |= LVIF_IMAGE;
			lvItem.iImage = nImageIndex;
		}
		if(nSubItem == 0)
			return InsertItem(&lvItem);
		return SetItem(&lvItem) ? nItem : -1;
	}

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	BOOL SortItemsEx(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SORTITEMSEX, (WPARAM)lParamSort, (LPARAM)pfnCompare);
	}
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0501)
	int InsertGroup(int nItem, PLVGROUP pGroup)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_INSERTGROUP, nItem, (LPARAM)pGroup);
	}

	int AddGroup(PLVGROUP pGroup)
	{
		return InsertGroup(-1, pGroup);
	}

	int RemoveGroup(int nGroupID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_REMOVEGROUP, nGroupID, 0L);
	}

	void MoveGroup(int nGroupID, int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_MOVEGROUP, nGroupID, nItem);
	}

	void MoveItemToGroup(int nItem, int nGroupID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_MOVEITEMTOGROUP, nItem, nGroupID);
	}

	int EnableGroupView(BOOL bEnable)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_ENABLEGROUPVIEW, bEnable, 0L);
	}

	int SortGroups(PFNLVGROUPCOMPARE pCompareFunc, LPVOID lpVoid = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SORTGROUPS, (WPARAM)pCompareFunc, (LPARAM)lpVoid);
	}

	void InsertGroupSorted(PLVINSERTGROUPSORTED pInsertGroupSorted)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_INSERTGROUPSORTED, (WPARAM)pInsertGroupSorted, 0L);
	}

	void RemoveAllGroups()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_REMOVEALLGROUPS, 0, 0L);
	}

	BOOL HasGroup(int nGroupID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_HASGROUP, nGroupID, 0L);
	}

	BOOL InsertMarkHitTest(LPPOINT lpPoint, LPLVINSERTMARK pInsertMark) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_INSERTMARKHITTEST, (WPARAM)lpPoint, (LPARAM)pInsertMark);
	}

	BOOL SetInfoTip(PLVSETINFOTIP pSetInfoTip)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LVM_SETINFOTIP, 0, (LPARAM)pSetInfoTip);
	}

	void CancelEditLabel()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, LVM_CANCELEDITLABEL, 0, 0L);
	}

	UINT MapIndexToID(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, LVM_MAPINDEXTOID, nIndex, 0L);
	}

	int MapIDToIndex(UINT uID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_MAPIDTOINDEX, uID, 0L);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_WINNT >= 0x0600)
	int HitTestEx(LPLVHITTESTINFO lpHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_HITTEST, (WPARAM)-1, (LPARAM)lpHitTestInfo);
	}

	int HitTestEx(POINT pt, UINT* pFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		LVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		int nRes = (int)::SendMessage(m_hWnd, LVM_HITTEST, (WPARAM)-1, (LPARAM)&hti);
		if (pFlags != NULL)
			*pFlags = hti.flags;
		return nRes;
	}

	int SubItemHitTestEx(LPLVHITTESTINFO lpHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LVM_SUBITEMHITTEST, (WPARAM)-1, (LPARAM)lpHitTestInfo);
	}
#endif // (_WIN32_WINNT >= 0x0600)

	// Note: selects only one item
	BOOL SelectItem(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		// multi-selection only: de-select all items
		if((GetStyle() & LVS_SINGLESEL) == 0)
			SetItemState(-1, 0, LVIS_SELECTED);

		BOOL bRet = SetItemState(nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		if(bRet)
			bRet = EnsureVisible(nIndex, FALSE);

		return bRet;
	}
};

typedef CListViewCtrlT<ATL::CWindow>   CListViewCtrl;


///////////////////////////////////////////////////////////////////////////////
// CTreeViewCtrl

template <class TBase>
class CTreeViewCtrlT : public TBase
{
public:
// Constructors
	CTreeViewCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CTreeViewCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_TREEVIEW;
	}

	UINT GetCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, TVM_GETCOUNT, 0, 0L);
	}

	UINT GetIndent() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, TVM_GETINDENT, 0, 0L);
	}

	void SetIndent(UINT nIndent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TVM_SETINDENT, nIndent, 0L);
	}

	CImageList GetImageList(int nImageListType = TVSIL_NORMAL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TVM_GETIMAGELIST, (WPARAM)nImageListType, 0L));
	}

	CImageList SetImageList(HIMAGELIST hImageList, int nImageListType = TVSIL_NORMAL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TVM_SETIMAGELIST, (WPARAM)nImageListType, (LPARAM)hImageList));
	}

	BOOL GetItem(LPTVITEM pItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)pItem);
	}

	BOOL SetItem(LPTVITEM pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)pItem);
	}

	BOOL SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR lpszItem, int nImage,
		int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = nMask;
		item.pszText = (LPTSTR) lpszItem;
		item.iImage = nImage;
		item.iSelectedImage = nSelectedImage;
		item.state = nState;
		item.stateMask = nStateMask;
		item.lParam = lParam;
		return (BOOL)::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)&item);
	}

	BOOL GetItemText(HTREEITEM hItem, LPTSTR lpstrText, int nLen) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpstrText != NULL);

		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_TEXT;
		item.pszText = lpstrText;
		item.cchTextMax = nLen;

		return (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
	}

#ifndef _ATL_NO_COM
	BOOL GetItemText(HTREEITEM hItem, BSTR& bstrText) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrText == NULL);
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_TEXT;

		LPTSTR lpstrText = NULL;
		BOOL bRet = FALSE;
		for(int nLen = 256; ; nLen *= 2)
		{
			ATLTRY(lpstrText = new TCHAR[nLen]);
			if(lpstrText == NULL)
				break;
			lpstrText[0] = NULL;
			item.pszText = lpstrText;
			item.cchTextMax = nLen;
			bRet = (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
			if(!bRet || (lstrlen(item.pszText) < nLen - 1))
				break;
			delete [] lpstrText;
			lpstrText = NULL;
		}

		if(lpstrText != NULL)
		{
			if(bRet)
				bstrText = ::SysAllocString(T2OLE(lpstrText));
			delete [] lpstrText;
		}

		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	BOOL GetItemText(HTREEITEM hItem, _CSTRING_NS::CAtlString& strText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_TEXT;

		strText.Empty();
		BOOL bRet = FALSE;
		for(int nLen = 256; ; nLen *= 2)
		{
			item.pszText = strText.GetBufferSetLength(nLen);
			if(item.pszText == NULL)
			{
				bRet = FALSE;
				break;
			}
			item.cchTextMax = nLen;
			bRet = (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
			if(!bRet || (lstrlen(item.pszText) < nLen - 1))
				break;
		}
		strText.ReleaseBuffer();
		return bRet;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	BOOL SetItemText(HTREEITEM hItem, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(hItem, TVIF_TEXT, lpszItem, 0, 0, 0, 0, NULL);
	}

	BOOL GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		BOOL bRes = (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
		if (bRes)
		{
			nImage = item.iImage;
			nSelectedImage = item.iSelectedImage;
		}
		return bRes;
	}

	BOOL SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(hItem, TVIF_IMAGE|TVIF_SELECTEDIMAGE, NULL, nImage, nSelectedImage, 0, 0, NULL);
	}

	UINT GetItemState(HTREEITEM hItem, UINT nStateMask) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
		return (((UINT)::SendMessage(m_hWnd, TVM_GETITEMSTATE, (WPARAM)hItem, (LPARAM)nStateMask)) & nStateMask);
#else // !((_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE))
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_STATE;
		item.state = 0;
		item.stateMask = nStateMask;
		::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
		return (item.state & nStateMask);
#endif // !((_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE))
	}

	BOOL SetItemState(HTREEITEM hItem, UINT nState, UINT nStateMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(hItem, TVIF_STATE, NULL, 0, 0, nState, nStateMask, NULL);
	}

	DWORD_PTR GetItemData(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_PARAM;
		BOOL bRet = (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
		return (DWORD_PTR)(bRet ? item.lParam : NULL);
	}

	BOOL SetItemData(HTREEITEM hItem, DWORD_PTR dwData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(hItem, TVIF_PARAM, NULL, 0, 0, 0, 0, (LPARAM)dwData);
	}

	CEdit GetEditControl() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CEdit((HWND)::SendMessage(m_hWnd, TVM_GETEDITCONTROL, 0, 0L));
	}

	UINT GetVisibleCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, TVM_GETVISIBLECOUNT, 0, 0L);
	}

	BOOL GetItemRect(HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		*(HTREEITEM*)lpRect = hItem;
		return (BOOL)::SendMessage(m_hWnd, TVM_GETITEMRECT, (WPARAM)bTextOnly, (LPARAM)lpRect);
	}

	BOOL ItemHasChildren(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_CHILDREN;
		::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
		return item.cChildren;
	}

#ifndef _WIN32_WCE
	CToolTipCtrl GetToolTips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, TVM_GETTOOLTIPS, 0, 0L));
	}

	CToolTipCtrl SetToolTips(HWND hWndTT)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, TVM_SETTOOLTIPS, (WPARAM)hWndTT, 0L));
	}
#endif // !_WIN32_WCE

	int GetISearchString(LPTSTR lpstr) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TVM_GETISEARCHSTRING, 0, (LPARAM)lpstr);
	}

	// checkboxes only
	BOOL GetCheckState(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & TVS_CHECKBOXES) != 0);
		UINT uRet = GetItemState(hItem, TVIS_STATEIMAGEMASK);
		return (uRet >> 12) - 1;
	}

	BOOL SetCheckState(HTREEITEM hItem, BOOL bCheck)
	{
		int nCheck = bCheck ? 2 : 1;   // one based index
		return SetItemState(hItem, INDEXTOSTATEIMAGEMASK(nCheck), TVIS_STATEIMAGEMASK);
	}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	COLORREF GetBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_GETBKCOLOR, 0, 0L);
	}

	COLORREF SetBkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_SETBKCOLOR, 0, (LPARAM)clr);
	}

	COLORREF GetInsertMarkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_GETINSERTMARKCOLOR, 0, 0L);
	}

	COLORREF SetInsertMarkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_SETINSERTMARKCOLOR, 0, (LPARAM)clr);
	}

	int GetItemHeight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TVM_GETITEMHEIGHT, 0, 0L);
	}

	int SetItemHeight(int cyHeight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TVM_SETITEMHEIGHT, cyHeight, 0L);
	}

	int GetScrollTime() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TVM_GETSCROLLTIME, 0, 0L);
	}

	int SetScrollTime(int nScrollTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TVM_SETSCROLLTIME, nScrollTime, 0L);
	}

	COLORREF GetTextColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_GETTEXTCOLOR, 0, 0L);
	}

	COLORREF SetTextColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_SETTEXTCOLOR, 0, (LPARAM)clr);
	}

	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	COLORREF GetLineColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_GETLINECOLOR, 0, 0L);
	}

	COLORREF SetLineColor(COLORREF clrNew /*= CLR_DEFAULT*/)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TVM_SETLINECOLOR, 0, (LPARAM)clrNew);
	}
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL GetItem(LPTVITEMEX pItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)pItem);
	}

	BOOL SetItem(LPTVITEMEX pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)pItem);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

	DWORD GetExtendedStyle() const
	{
#ifndef TVM_GETEXTENDEDSTYLE
		const UINT TVM_GETEXTENDEDSTYLE = (TV_FIRST + 45);
#endif
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TVM_GETEXTENDEDSTYLE, 0, 0L);
	}

	DWORD SetExtendedStyle(DWORD dwStyle, DWORD dwMask)
	{
#ifndef TVM_SETEXTENDEDSTYLE
		const UINT TVM_SETEXTENDEDSTYLE = (TV_FIRST + 44);
#endif
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TVM_SETEXTENDEDSTYLE, dwMask, dwStyle);
	}

#if (_WIN32_WINNT >= 0x0600)
	BOOL SetAutoScrollInfo(UINT uPixPerSec, UINT uUpdateTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SETAUTOSCROLLINFO, (WPARAM)uPixPerSec, (LPARAM)uUpdateTime);
	}

	DWORD GetSelectedCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TVM_GETSELECTEDCOUNT, 0, 0L);
	}

	BOOL GetItemPartRect(HTREEITEM hItem, TVITEMPART partID, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVGETITEMPARTRECTINFO gipri = { hItem, lpRect, partID };
		return (BOOL)::SendMessage(m_hWnd, TVM_GETITEMPARTRECT, 0, (LPARAM)&gipri);
	}
#endif // (_WIN32_WINNT >= 0x0600)

// Operations
	HTREEITEM InsertItem(LPTVINSERTSTRUCT lpInsertStruct)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)lpInsertStruct);
	}

	HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage,
		int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, lpszItem, nImage, nSelectedImage, 0, 0, 0, hParent, hInsertAfter); 
	}

	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return InsertItem(TVIF_TEXT, lpszItem, 0, 0, 0, 0, 0, hParent, hInsertAfter);
	}

	HTREEITEM InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage,
		int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,
		HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVINSERTSTRUCT tvis = { 0 };
		tvis.hParent = hParent;
		tvis.hInsertAfter = hInsertAfter;
		tvis.item.mask = nMask;
		tvis.item.pszText = (LPTSTR) lpszItem;
		tvis.item.iImage = nImage;
		tvis.item.iSelectedImage = nSelectedImage;
		tvis.item.state = nState;
		tvis.item.stateMask = nStateMask;
		tvis.item.lParam = lParam;
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvis);
	}

	BOOL DeleteItem(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)hItem);
	}

	BOOL DeleteAllItems()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
	}

	BOOL Expand(HTREEITEM hItem, UINT nCode = TVE_EXPAND)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_EXPAND, nCode, (LPARAM)hItem);
	}

	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, nCode, (LPARAM)hItem);
	}

	HTREEITEM GetChildItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
	}

	HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem); 
	}

	HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUS, (LPARAM)hItem);
	}

	HTREEITEM GetParentItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem); 
	}

	HTREEITEM GetFirstVisibleItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0L);
	}

	HTREEITEM GetNextVisibleItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);
	}

	HTREEITEM GetPrevVisibleItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItem);
	}

	HTREEITEM GetSelectedItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0L);
	}

	HTREEITEM GetDropHilightItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0L);
	}

	HTREEITEM GetRootItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_ROOT, 0L);
	}

#if !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)
	HTREEITEM GetLastVisibleItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_LASTVISIBLE, 0L);
	}
#endif // !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0600)
	HTREEITEM GetNextSelectedItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXTSELECTED, 0L);
	}
#endif // (_WIN32_IE >= 0x0600)

	BOOL Select(HTREEITEM hItem, UINT nCode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, nCode, (LPARAM)hItem);
	}

	BOOL SelectItem(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
	}

	BOOL SelectDropTarget(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem);
	}

	BOOL SelectSetFirstVisible(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_FIRSTVISIBLE, (LPARAM)hItem);
	}

	CEdit EditLabel(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CEdit((HWND)::SendMessage(m_hWnd, TVM_EDITLABEL, 0, (LPARAM)hItem));
	}

	BOOL EndEditLabelNow(BOOL bCancel)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_ENDEDITLABELNOW, bCancel, 0L);
	}

	HTREEITEM HitTest(TVHITTESTINFO* pHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)pHitTestInfo);
	}

	HTREEITEM HitTest(POINT pt, UINT* pFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)&hti);
		if (pFlags != NULL)
			*pFlags = hti.flags;
		return hTreeItem;
	}

	BOOL SortChildren(HTREEITEM hItem, BOOL bRecurse = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SORTCHILDREN, (WPARAM)bRecurse, (LPARAM)hItem);
	}

	BOOL EnsureVisible(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_ENSUREVISIBLE, 0, (LPARAM)hItem);
	}

	BOOL SortChildrenCB(LPTVSORTCB pSort, BOOL bRecurse = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SORTCHILDRENCB, (WPARAM)bRecurse, (LPARAM)pSort);
	}

	CImageList RemoveImageList(int nImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TVM_SETIMAGELIST, (WPARAM)nImageList, NULL));
	}

	CImageList CreateDragImage(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItem));
	}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL SetInsertMark(HTREEITEM hTreeItem, BOOL bAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SETINSERTMARK, bAfter, (LPARAM)hTreeItem);
	}

	BOOL RemoveInsertMark()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TVM_SETINSERTMARK, 0, 0L);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0501)
	HTREEITEM MapAccIDToHTREEITEM(UINT uID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_MAPACCIDTOHTREEITEM, uID, 0L);
	}

	UINT MapHTREEITEMToAccID(HTREEITEM hTreeItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, TVM_MAPHTREEITEMTOACCID, (WPARAM)hTreeItem, 0L);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_WINNT >= 0x0600)
	void ShowInfoTip(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TVM_SHOWINFOTIP, 0, (LPARAM)hItem);
	}
#endif // (_WIN32_WINNT >= 0x0600)
};

typedef CTreeViewCtrlT<ATL::CWindow>   CTreeViewCtrl;


///////////////////////////////////////////////////////////////////////////////
// CTreeViewCtrlEx

// forward declaration
template <class TBase> class CTreeViewCtrlExT;

// Note: TBase here is for CTreeViewCtrlExT, and not for CTreeItemT itself
template <class TBase>
class CTreeItemT
{
public:
	HTREEITEM m_hTreeItem;
	CTreeViewCtrlExT<TBase>* m_pTreeView;

// Construction
	CTreeItemT(HTREEITEM hTreeItem = NULL, CTreeViewCtrlExT<TBase>* pTreeView = NULL) : m_hTreeItem(hTreeItem), m_pTreeView(pTreeView)
	{ }
 
	CTreeItemT(const CTreeItemT<TBase>& posSrc)
	{
		*this = posSrc;
	}

	operator HTREEITEM() { return m_hTreeItem; }

	CTreeItemT<TBase>& operator =(const CTreeItemT<TBase>& itemSrc)
	{
		m_hTreeItem = itemSrc.m_hTreeItem;
		m_pTreeView = itemSrc.m_pTreeView;
		return *this;
	}

// Attributes
	CTreeViewCtrlExT<TBase>* GetTreeView() const { return m_pTreeView; }

	BOOL operator !() const { return m_hTreeItem == NULL; }

	BOOL IsNull() const { return m_hTreeItem == NULL; }
	
	BOOL GetRect(LPRECT lpRect, BOOL bTextOnly) const;
	BOOL GetText(LPTSTR lpstrText, int nLen) const;
#ifndef _ATL_NO_COM
	BOOL GetText(BSTR& bstrText) const;
#endif // !_ATL_NO_COM
#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	BOOL GetText(_CSTRING_NS::CAtlString& strText) const;
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	BOOL SetText(LPCTSTR lpszItem);
	BOOL GetImage(int& nImage, int& nSelectedImage) const;
	BOOL SetImage(int nImage, int nSelectedImage);
	UINT GetState(UINT nStateMask) const;
	BOOL SetState(UINT nState, UINT nStateMask);
	DWORD_PTR GetData() const;
	BOOL SetData(DWORD_PTR dwData);
	BOOL SetItem(UINT nMask, LPCTSTR lpszItem, int nImage, int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam);

// Operations
	CTreeItemT<TBase> InsertAfter(LPCTSTR lpstrItem, HTREEITEM hItemAfter, int nImageIndex)
	{
		return _Insert(lpstrItem, nImageIndex, hItemAfter);
	}

	CTreeItemT<TBase> AddHead(LPCTSTR lpstrItem, int nImageIndex)
	{
		return _Insert(lpstrItem, nImageIndex, TVI_FIRST);
	}

	CTreeItemT<TBase> AddTail(LPCTSTR lpstrItem, int nImageIndex)
	{
		return _Insert(lpstrItem, nImageIndex, TVI_LAST);
	}

	CTreeItemT<TBase> GetChild() const;
	CTreeItemT<TBase> GetNext(UINT nCode) const;
	CTreeItemT<TBase> GetNextSibling() const;
	CTreeItemT<TBase> GetPrevSibling() const;
	CTreeItemT<TBase> GetParent() const;
	CTreeItemT<TBase> GetFirstVisible() const;
	CTreeItemT<TBase> GetNextVisible() const;
	CTreeItemT<TBase> GetPrevVisible() const;
	CTreeItemT<TBase> GetSelected() const;
	CTreeItemT<TBase> GetDropHilight() const;
	CTreeItemT<TBase> GetRoot() const;
#if !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)
	CTreeItemT<TBase> GetLastVisible() const;
#endif // !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)
#if (_WIN32_IE >= 0x0600)
	CTreeItemT<TBase> GetNextSelected() const;
#endif // (_WIN32_IE >= 0x0600)
	BOOL HasChildren() const;
	BOOL Delete();
	BOOL Expand(UINT nCode = TVE_EXPAND);
	BOOL Select(UINT nCode);
	BOOL Select();
	BOOL SelectDropTarget();
	BOOL SelectSetFirstVisible();
	HWND EditLabel();
	HIMAGELIST CreateDragImage();
	BOOL SortChildren(BOOL bRecurse = FALSE);
	BOOL EnsureVisible();
	CTreeItemT<TBase> _Insert(LPCTSTR lpstrItem, int nImageIndex, HTREEITEM hItemAfter);
	int GetImageIndex() const;
#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL SetInsertMark(BOOL bAfter);
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
#if (_WIN32_WINNT >= 0x0501)
	UINT MapHTREEITEMToAccID() const;
#endif // (_WIN32_WINNT >= 0x0501)
#if (_WIN32_WINNT >= 0x0600)
	void ShowInfoTip();
	BOOL GetPartRect(TVITEMPART partID, LPRECT lpRect) const;
#endif // (_WIN32_WINNT >= 0x0600)
};

typedef CTreeItemT<ATL::CWindow>   CTreeItem;


template <class TBase>
class CTreeViewCtrlExT : public CTreeViewCtrlT< TBase >
{
public:
// Constructors
	CTreeViewCtrlExT(HWND hWnd = NULL) : CTreeViewCtrlT< TBase >(hWnd)
	{ }

	CTreeViewCtrlExT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

// Operations (overides that return CTreeItem)
	CTreeItemT<TBase> InsertItem(LPTVINSERTSTRUCT lpInsertStruct)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)lpInsertStruct);
		return CTreeItemT<TBase>(hTreeItem, this);
	}

	CTreeItemT<TBase> InsertItem(LPCTSTR lpszItem, int nImage,
		int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, lpszItem, nImage, nSelectedImage, 0, 0, 0, hParent, hInsertAfter); 
	}

	CTreeItemT<TBase> InsertItem(LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return InsertItem(TVIF_TEXT, lpszItem, 0, 0, 0, 0, 0, hParent, hInsertAfter);
	}

	CTreeItemT<TBase> GetNextItem(HTREEITEM hItem, UINT nCode) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, nCode, (LPARAM)hItem);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetChildItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this); 
	}

	CTreeItemT<TBase> GetNextSiblingItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem); 
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetPrevSiblingItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUS, (LPARAM)hItem);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetParentItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem); 
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetFirstVisibleItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetNextVisibleItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetPrevVisibleItem(HTREEITEM hItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItem);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetSelectedItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetDropHilightItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> GetRootItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_ROOT, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

#if !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)
	CTreeItemT<TBase> GetLastVisibleItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_LASTVISIBLE, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}
#endif // !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0600)
	CTreeItemT<TBase> GetNextSelectedItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXTSELECTED, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}
#endif // (_WIN32_IE >= 0x0600)

	CTreeItemT<TBase> HitTest(TVHITTESTINFO* pHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)pHitTestInfo);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

	CTreeItemT<TBase> InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage,
		int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,
		HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVINSERTSTRUCT tvis = { 0 };
		tvis.hParent = hParent;
		tvis.hInsertAfter = hInsertAfter;
		tvis.item.mask = nMask;
		tvis.item.pszText = (LPTSTR) lpszItem;
		tvis.item.iImage = nImage;
		tvis.item.iSelectedImage = nSelectedImage;
		tvis.item.state = nState;
		tvis.item.stateMask = nStateMask;
		tvis.item.lParam = lParam;
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvis);
		return CTreeItemT<TBase>(hTreeItem, this);
	}

	CTreeItemT<TBase> HitTest(POINT pt, UINT* pFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)&hti);
		if (pFlags != NULL)
			*pFlags = hti.flags;
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}

#if (_WIN32_WINNT >= 0x0501)
	CTreeItemT<TBase> MapAccIDToHTREEITEM(UINT uID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HTREEITEM hTreeItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_MAPACCIDTOHTREEITEM, uID, 0L);
		return CTreeItemT<TBase>(hTreeItem, (CTreeViewCtrlExT<TBase>*)this);
	}
#endif // (_WIN32_WINNT >= 0x0501)
};

typedef CTreeViewCtrlExT<ATL::CWindow>   CTreeViewCtrlEx;


// CTreeItem inline methods
template <class TBase>
inline BOOL CTreeItemT<TBase>::GetRect(LPRECT lpRect, BOOL bTextOnly) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemRect(m_hTreeItem,lpRect,bTextOnly);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetNext(UINT nCode) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetNextItem(m_hTreeItem,nCode);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetChild() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetChildItem(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetNextSibling() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetNextSiblingItem(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetPrevSibling() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetPrevSiblingItem(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetParent() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetParentItem(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetFirstVisible() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetFirstVisibleItem();
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetNextVisible() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetNextVisibleItem(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetPrevVisible() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetPrevVisibleItem(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetSelected() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetSelectedItem();
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetDropHilight() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetDropHilightItem();
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetRoot() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetRootItem();
}

#if !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)
template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetLastVisible() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetLastVisibleItem();
}
#endif // !defined(_WIN32_WCE) && (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0600)
template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::GetNextSelected() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetNextSelectedItem();
}
#endif // (_WIN32_IE >= 0x0600)

template <class TBase>
inline BOOL CTreeItemT<TBase>::GetText(LPTSTR lpstrText, int nLen) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemText(m_hTreeItem, lpstrText, nLen);
}

#ifndef _ATL_NO_COM
#ifdef _OLEAUTO_H_
template <class TBase>
inline BOOL CTreeItemT<TBase>::GetText(BSTR& bstrText) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemText(m_hTreeItem, bstrText);
}
#endif // _OLEAUTO_H_
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
template <class TBase>
inline BOOL CTreeItemT<TBase>::GetText(_CSTRING_NS::CAtlString& strText) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemText(m_hTreeItem, strText);
}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

template <class TBase>
inline BOOL CTreeItemT<TBase>::GetImage(int& nImage, int& nSelectedImage) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemImage(m_hTreeItem,nImage,nSelectedImage);
}

template <class TBase>
inline UINT CTreeItemT<TBase>::GetState(UINT nStateMask) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemState(m_hTreeItem,nStateMask);
}

template <class TBase>
inline DWORD_PTR CTreeItemT<TBase>::GetData() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemData(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SetItem(UINT nMask, LPCTSTR lpszItem, int nImage,
		int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SetItem(m_hTreeItem, nMask, lpszItem, nImage, nSelectedImage, nState, nStateMask, lParam);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SetText(LPCTSTR lpszItem)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SetItemText(m_hTreeItem,lpszItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SetImage(int nImage, int nSelectedImage)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SetItemImage(m_hTreeItem,nImage,nSelectedImage);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SetState(UINT nState, UINT nStateMask)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SetItemState(m_hTreeItem,nState,nStateMask);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SetData(DWORD_PTR dwData)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SetItemData(m_hTreeItem,dwData);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::HasChildren() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->ItemHasChildren(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::Delete()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->DeleteItem(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::Expand(UINT nCode /*= TVE_EXPAND*/)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->Expand(m_hTreeItem,nCode);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::Select(UINT nCode)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->Select(m_hTreeItem,nCode);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::Select()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SelectItem(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SelectDropTarget()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SelectDropTarget(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SelectSetFirstVisible()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SelectSetFirstVisible(m_hTreeItem);
}

template <class TBase>
inline HWND CTreeItemT<TBase>::EditLabel()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->EditLabel(m_hTreeItem);
}

template <class TBase>
inline HIMAGELIST CTreeItemT<TBase>::CreateDragImage()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->CreateDragImage(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::SortChildren(BOOL bRecurse /*= FALSE*/)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SortChildren(m_hTreeItem, bRecurse);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::EnsureVisible()
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->EnsureVisible(m_hTreeItem);
}

template <class TBase>
inline CTreeItemT<TBase> CTreeItemT<TBase>::_Insert(LPCTSTR lpstrItem, int nImageIndex, HTREEITEM hItemAfter)
{
	ATLASSERT(m_pTreeView != NULL);
	TVINSERTSTRUCT ins = { 0 };
	ins.hParent = m_hTreeItem;
	ins.hInsertAfter = hItemAfter;
	ins.item.mask = TVIF_TEXT;
	ins.item.pszText = (LPTSTR)lpstrItem;
	if(nImageIndex != -1)
	{
		ins.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		ins.item.iImage = nImageIndex;
		ins.item.iSelectedImage = nImageIndex;
	}
	return CTreeItemT<TBase>(m_pTreeView->InsertItem(&ins), m_pTreeView);
}

template <class TBase>
inline int CTreeItemT<TBase>::GetImageIndex() const
{
	ATLASSERT(m_pTreeView != NULL);
	TVITEM item = { 0 };
	item.mask = TVIF_HANDLE | TVIF_IMAGE;
	item.hItem = m_hTreeItem;
	m_pTreeView->GetItem(&item);
	return item.iImage;
}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
template <class TBase>
inline BOOL CTreeItemT<TBase>::SetInsertMark(BOOL bAfter)
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->SetInsertMark(m_hTreeItem, bAfter);
}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0501)
template <class TBase>
inline UINT CTreeItemT<TBase>::MapHTREEITEMToAccID() const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->MapHTREEITEMToAccID(m_hTreeItem);
}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_WINNT >= 0x0600)
template <class TBase>
inline void CTreeItemT<TBase>::ShowInfoTip()
{
	ATLASSERT(m_pTreeView != NULL);
	m_pTreeView->ShowInfoTip(m_hTreeItem);
}

template <class TBase>
inline BOOL CTreeItemT<TBase>::GetPartRect(TVITEMPART partID, LPRECT lpRect) const
{
	ATLASSERT(m_pTreeView != NULL);
	return m_pTreeView->GetItemPartRect(m_hTreeItem, partID, lpRect);
}
#endif // (_WIN32_WINNT >= 0x0600)


///////////////////////////////////////////////////////////////////////////////
// CToolBarCtrl

template <class TBase>
class CToolBarCtrlT : public TBase
{
public:
// Construction
	CToolBarCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CToolBarCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return TOOLBARCLASSNAME;
	}

	BOOL IsButtonEnabled(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ISBUTTONENABLED, nID, 0L);
	}

	BOOL IsButtonChecked(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ISBUTTONCHECKED, nID, 0L);
	}

	BOOL IsButtonPressed(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ISBUTTONPRESSED, nID, 0L);
	}

	BOOL IsButtonHidden(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return(BOOL) ::SendMessage(m_hWnd, TB_ISBUTTONHIDDEN, nID, 0L);
	}

	BOOL IsButtonIndeterminate(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ISBUTTONINDETERMINATE, nID, 0L);
	}

	int GetState(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETSTATE, nID, 0L);
	}

	BOOL SetState(int nID, UINT nState)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETSTATE, nID, MAKELPARAM(nState, 0));
	}

	BOOL GetButton(int nIndex, LPTBBUTTON lpButton) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETBUTTON, nIndex, (LPARAM)lpButton);
	}

	int GetButtonCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_BUTTONCOUNT, 0, 0L);
	}

	BOOL GetItemRect(int nIndex, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETITEMRECT, nIndex, (LPARAM)lpRect);
	}

	void SetButtonStructSize(int nSize = sizeof(TBBUTTON))
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_BUTTONSTRUCTSIZE, nSize, 0L);
	}

	BOOL SetButtonSize(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(size.cx, size.cy));
	}

	BOOL SetButtonSize(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(cx, cy));
	}

	BOOL SetBitmapSize(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(size.cx, size.cy));
	}

	BOOL SetBitmapSize(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(cx, cy));
	}

#ifndef _WIN32_WCE
	CToolTipCtrl GetToolTips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, TB_GETTOOLTIPS, 0, 0L));
	}

	void SetToolTips(HWND hWndToolTip)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETTOOLTIPS, (WPARAM)hWndToolTip, 0L);
	}
#endif // !_WIN32_WCE

	void SetNotifyWnd(HWND hWnd)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETPARENT, (WPARAM)hWnd, 0L);
	}

	int GetRows() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETROWS, 0, 0L);
	}

	void SetRows(int nRows, BOOL bLarger, LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETROWS, MAKELPARAM(nRows, bLarger), (LPARAM)lpRect);
	}

	BOOL SetCmdID(int nIndex, UINT nID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETCMDID, nIndex, nID);
	}

	DWORD GetBitmapFlags() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TB_GETBITMAPFLAGS, 0, 0L);
	}

	int GetBitmap(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETBITMAP, nID, 0L);
	}

	int GetButtonText(int nID, LPTSTR lpstrText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETBUTTONTEXT, nID, (LPARAM)lpstrText);
	}

	// nIndex - IE5 or higher only
	CImageList GetImageList(int nIndex = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_GETIMAGELIST, nIndex, 0L));
	}

	// nIndex - IE5 or higher only
	CImageList SetImageList(HIMAGELIST hImageList, int nIndex = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_SETIMAGELIST, nIndex, (LPARAM)hImageList));
	}

	// nIndex - IE5 or higher only
	CImageList GetDisabledImageList(int nIndex = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_GETDISABLEDIMAGELIST, nIndex, 0L));
	}

	// nIndex - IE5 or higher only
	CImageList SetDisabledImageList(HIMAGELIST hImageList, int nIndex = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_SETDISABLEDIMAGELIST, nIndex, (LPARAM)hImageList));
	}

#ifndef _WIN32_WCE
	// nIndex - IE5 or higher only
	CImageList GetHotImageList(int nIndex = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_GETHOTIMAGELIST, nIndex, 0L));
	}

	// nIndex - IE5 or higher only
	CImageList SetHotImageList(HIMAGELIST hImageList, int nIndex = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_SETHOTIMAGELIST, nIndex, (LPARAM)hImageList));
	}
#endif // !_WIN32_WCE

	DWORD GetStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TB_GETSTYLE, 0, 0L);
	}

	void SetStyle(DWORD dwStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETSTYLE, 0, dwStyle);
	}

	DWORD GetButtonSize() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TB_GETBUTTONSIZE, 0, 0L);
	}

	void GetButtonSize(SIZE& size) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, TB_GETBUTTONSIZE, 0, 0L);
		size.cx = LOWORD(dwRet);
		size.cy = HIWORD(dwRet);
	}

	BOOL GetRect(int nID, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETRECT, nID, (LPARAM)lpRect);
	}

	int GetTextRows() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETTEXTROWS, 0, 0L);
	}

	BOOL SetButtonWidth(int cxMin, int cxMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETBUTTONWIDTH, 0, MAKELPARAM(cxMin, cxMax));
	}

	BOOL SetIndent(int nIndent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETINDENT, nIndent, 0L);
	}

	BOOL SetMaxTextRows(int nMaxTextRows)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, nMaxTextRows, 0L);
	}

#if (_WIN32_IE >= 0x0400)
#ifndef _WIN32_WCE
	BOOL GetAnchorHighlight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETANCHORHIGHLIGHT, 0, 0L);
	}

	BOOL SetAnchorHighlight(BOOL bEnable = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETANCHORHIGHLIGHT, bEnable, 0L);
	}
#endif // !_WIN32_WCE

	int GetButtonInfo(int nID, LPTBBUTTONINFO lptbbi) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETBUTTONINFO, nID, (LPARAM)lptbbi);
	}

	BOOL SetButtonInfo(int nID, LPTBBUTTONINFO lptbbi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETBUTTONINFO, nID, (LPARAM)lptbbi);
	}

	BOOL SetButtonInfo(int nID, DWORD dwMask, BYTE Style, BYTE State, LPCTSTR lpszItem, 
	                   int iImage, WORD cx, int iCommand, DWORD_PTR lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TBBUTTONINFO tbbi = { 0 };
		tbbi.cbSize = sizeof(TBBUTTONINFO);
		tbbi.dwMask = dwMask;
		tbbi.idCommand = iCommand;
		tbbi.iImage = iImage;
		tbbi.fsState = State;
		tbbi.fsStyle = Style;
		tbbi.cx = cx;
		tbbi.pszText = (LPTSTR) lpszItem;
		tbbi.lParam = lParam;
		return (BOOL)::SendMessage(m_hWnd, TB_SETBUTTONINFO, nID, (LPARAM)&tbbi);
	}

#ifndef _WIN32_WCE
	int GetHotItem() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETHOTITEM, 0, 0L);
	}

	int SetHotItem(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_SETHOTITEM, nItem, 0L);
	}
#endif // !_WIN32_WCE

	BOOL IsButtonHighlighted(int nButtonID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ISBUTTONHIGHLIGHTED, nButtonID, 0L);
	}

	DWORD SetDrawTextFlags(DWORD dwMask, DWORD dwFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TB_SETDRAWTEXTFLAGS, dwMask, dwFlags);
	}

#ifndef _WIN32_WCE
	BOOL GetColorScheme(LPCOLORSCHEME lpcs) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETCOLORSCHEME, 0, (LPARAM)lpcs);
	}

	void SetColorScheme(LPCOLORSCHEME lpcs)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETCOLORSCHEME, 0, (LPARAM)lpcs);
	}

	DWORD GetExtendedStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TB_GETEXTENDEDSTYLE, 0, 0L);
	}

	DWORD SetExtendedStyle(DWORD dwStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TB_SETEXTENDEDSTYLE, 0, dwStyle);
	}

	void GetInsertMark(LPTBINSERTMARK lptbim) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_GETINSERTMARK, 0, (LPARAM)lptbim);
	}

	void SetInsertMark(LPTBINSERTMARK lptbim)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETINSERTMARK, 0, (LPARAM)lptbim);
	}

	COLORREF GetInsertMarkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TB_GETINSERTMARKCOLOR, 0, 0L);
	}

	COLORREF SetInsertMarkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TB_SETINSERTMARKCOLOR, 0, (LPARAM)clr);
	}

	BOOL GetMaxSize(LPSIZE lpSize) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETMAXSIZE, 0, (LPARAM)lpSize);
	}

	void GetPadding(LPSIZE lpSizePadding) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpSizePadding != NULL);
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, TB_GETPADDING, 0, 0L);
		lpSizePadding->cx = GET_X_LPARAM(dwRet);
		lpSizePadding->cy = GET_Y_LPARAM(dwRet);
	}

	void SetPadding(int cx, int cy, LPSIZE lpSizePadding = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, TB_SETPADDING, 0, MAKELPARAM(cx, cy));
		if(lpSizePadding != NULL)
		{
			lpSizePadding->cx = GET_X_LPARAM(dwRet);
			lpSizePadding->cy = GET_Y_LPARAM(dwRet);
		}
	}

	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	int GetString(int nString, LPTSTR lpstrString, int cchMaxLen) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_GETSTRING, MAKEWPARAM(cchMaxLen, nString), (LPARAM)lpstrString);
	}

	int GetStringBSTR(int nString, BSTR& bstrString) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrString == NULL);
		int nLength = (int)(short)LOWORD(::SendMessage(m_hWnd, TB_GETSTRING, MAKEWPARAM(0, nString), NULL));
		if(nLength != -1)
		{
			CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
			LPTSTR lpstrText = buff.Allocate(nLength + 1);
			if(lpstrText != NULL)
			{
				nLength = (int)::SendMessage(m_hWnd, TB_GETSTRING, MAKEWPARAM(nLength + 1, nString), (LPARAM)lpstrText);
				if(nLength != -1)
					bstrString = ::SysAllocString(T2OLE(lpstrText));
			}
			else
			{
				nLength = -1;
			}
		}

		return nLength;
	}

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	int GetString(int nString, _CSTRING_NS::CAtlString& str) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		int nLength = (int)(short)LOWORD(::SendMessage(m_hWnd, TB_GETSTRING, MAKEWPARAM(0, nString), NULL));
		if(nLength != -1)
		{
			LPTSTR lpstr = str.GetBufferSetLength(nLength + 1);
			if(lpstr != NULL)
				nLength = (int)::SendMessage(m_hWnd, TB_GETSTRING, MAKEWPARAM(nLength + 1, nString), (LPARAM)lpstr);
			else
				nLength = -1;
			str.ReleaseBuffer();
		}
		return nLength;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0501)
	void GetMetrics(LPTBMETRICS lptbm) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_GETMETRICS, 0, (LPARAM)lptbm);
	}

	void SetMetrics(LPTBMETRICS lptbm)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETMETRICS, 0, (LPARAM)lptbm);
	}

	void SetWindowTheme(LPCWSTR lpstrTheme)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_SETWINDOWTHEME, 0, (LPARAM)lpstrTheme);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_WINNT >= 0x0600)
	CImageList GetPressedImageList(int nIndex = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_GETPRESSEDIMAGELIST, nIndex, 0L));
	}

	CImageList SetPressedImageList(HIMAGELIST hImageList, int nIndex = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TB_SETPRESSEDIMAGELIST, nIndex, (LPARAM)hImageList));
	}
#endif // (_WIN32_WINNT >= 0x0600)

// Operations
	BOOL EnableButton(int nID, BOOL bEnable = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ENABLEBUTTON, nID, MAKELPARAM(bEnable, 0));
	}

	BOOL CheckButton(int nID, BOOL bCheck = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_CHECKBUTTON, nID, MAKELPARAM(bCheck, 0));
	}

	BOOL PressButton(int nID, BOOL bPress = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_PRESSBUTTON, nID, MAKELPARAM(bPress, 0));
	}

	BOOL HideButton(int nID, BOOL bHide = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_HIDEBUTTON, nID, MAKELPARAM(bHide, 0));
	}

	BOOL Indeterminate(int nID, BOOL bIndeterminate = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_INDETERMINATE, nID, MAKELPARAM(bIndeterminate, 0));
	}

	int AddBitmap(int nNumButtons, UINT nBitmapID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TBADDBITMAP tbab = { 0 };
		tbab.hInst = ModuleHelper::GetResourceInstance();
		ATLASSERT(tbab.hInst != NULL);
		tbab.nID = nBitmapID;
		return (int)::SendMessage(m_hWnd, TB_ADDBITMAP, (WPARAM)nNumButtons, (LPARAM)&tbab);
	}

	int AddBitmap(int nNumButtons, HBITMAP hBitmap)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TBADDBITMAP tbab = { 0 };
		tbab.hInst = NULL;
		tbab.nID = (UINT_PTR)hBitmap;
		return (int)::SendMessage(m_hWnd, TB_ADDBITMAP, (WPARAM)nNumButtons, (LPARAM)&tbab);
	}

	BOOL AddButtons(int nNumButtons, LPTBBUTTON lpButtons)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_ADDBUTTONS, nNumButtons, (LPARAM)lpButtons);
	}

	BOOL InsertButton(int nIndex, LPTBBUTTON lpButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_INSERTBUTTON, nIndex, (LPARAM)lpButton);
	}

	BOOL InsertButton(int nIndex, int iCommand, BYTE Style, BYTE State, int iBitmap, 
	                  INT_PTR iString, DWORD_PTR lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TBBUTTON tbb = { 0 };
		tbb.fsStyle = Style;
		tbb.fsState = State;
		tbb.idCommand = iCommand;
		tbb.iBitmap = iBitmap;
		tbb.iString = iString;
		tbb.dwData = lParam;
		return (BOOL)::SendMessage(m_hWnd, TB_INSERTBUTTON, nIndex, (LPARAM)&tbb);
	}

	BOOL InsertButton(int nIndex, int iCommand, BYTE Style, BYTE State, int iBitmap, 
	                  LPCTSTR lpszItem, DWORD_PTR lParam)
	{
		return InsertButton(nIndex, iCommand, Style, State, iBitmap, (INT_PTR)lpszItem, lParam);
	}

	BOOL AddButton(LPTBBUTTON lpButton)
	{
		return InsertButton(-1, lpButton);
	}

	BOOL AddButton(int iCommand, BYTE Style, BYTE State, int iBitmap, INT_PTR iString, DWORD_PTR lParam)
	{
		return InsertButton(-1, iCommand, Style, State, iBitmap, iString, lParam);
	}

	BOOL AddButton(int iCommand, BYTE Style, BYTE State, int iBitmap, LPCTSTR lpszItem, DWORD_PTR lParam)
	{
		return InsertButton(-1, iCommand, Style, State, iBitmap, lpszItem, lParam);
	}

	BOOL DeleteButton(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_DELETEBUTTON, nIndex, 0L);
	}

	UINT CommandToIndex(UINT nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, TB_COMMANDTOINDEX, nID, 0L);
	}

#ifndef _WIN32_WCE
	void SaveState(HKEY hKeyRoot, LPCTSTR lpszSubKey, LPCTSTR lpszValueName)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TBSAVEPARAMS tbs = { 0 };
		tbs.hkr = hKeyRoot;
		tbs.pszSubKey = lpszSubKey;
		tbs.pszValueName = lpszValueName;
		::SendMessage(m_hWnd, TB_SAVERESTORE, (WPARAM)TRUE, (LPARAM)&tbs);
	}

	void RestoreState(HKEY hKeyRoot, LPCTSTR lpszSubKey, LPCTSTR lpszValueName)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TBSAVEPARAMS tbs = { 0 };
		tbs.hkr = hKeyRoot;
		tbs.pszSubKey = lpszSubKey;
		tbs.pszValueName = lpszValueName;
		::SendMessage(m_hWnd, TB_SAVERESTORE, (WPARAM)FALSE, (LPARAM)&tbs);
	}

	void Customize()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_CUSTOMIZE, 0, 0L);
	}
#endif // !_WIN32_WCE

	int AddString(UINT nStringID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_ADDSTRING, (WPARAM)ModuleHelper::GetResourceInstance(), (LPARAM)nStringID);
	}

	int AddStrings(LPCTSTR lpszStrings)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_ADDSTRING, 0, (LPARAM)lpszStrings);
	}

	void AutoSize()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TB_AUTOSIZE, 0, 0L);
	}

	BOOL ChangeBitmap(int nID, int nBitmap)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_CHANGEBITMAP, nID, MAKELPARAM(nBitmap, 0));
	}

	int LoadImages(int nBitmapID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_LOADIMAGES, nBitmapID, (LPARAM)ModuleHelper::GetResourceInstance());
	}

	int LoadStdImages(int nBitmapID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_LOADIMAGES, nBitmapID, (LPARAM)HINST_COMMCTRL);
	}

	BOOL ReplaceBitmap(LPTBREPLACEBITMAP ptbrb)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_REPLACEBITMAP, 0, (LPARAM)ptbrb);
	}

#if (_WIN32_IE >= 0x0400)
	int HitTest(LPPOINT lpPoint) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TB_HITTEST, 0, (LPARAM)lpPoint);
	}

#ifndef _WIN32_WCE
	BOOL InsertMarkHitTest(LPPOINT lpPoint, LPTBINSERTMARK lptbim) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_INSERTMARKHITTEST, (WPARAM)lpPoint, (LPARAM)lptbim);
	}

	BOOL InsertMarkHitTest(int x, int y, LPTBINSERTMARK lptbim) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		POINT pt = { x, y };
		return (BOOL)::SendMessage(m_hWnd, TB_INSERTMARKHITTEST, (WPARAM)&pt, (LPARAM)lptbim);
	}

	BOOL MapAccelerator(TCHAR chAccel, int& nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_MAPACCELERATOR, (WPARAM)chAccel, (LPARAM)&nID);
	}

	BOOL MarkButton(int nID, BOOL bHighlight = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_MARKBUTTON, nID, MAKELPARAM(bHighlight, 0));
	}

	BOOL MoveButton(int nOldPos, int nNewPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TB_MOVEBUTTON, nOldPos, nNewPos);
	}

	HRESULT GetObject(REFIID iid, LPVOID* ppvObject)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HRESULT)::SendMessage(m_hWnd, TB_GETOBJECT, (WPARAM)&iid, (LPARAM)ppvObject);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)
};

typedef CToolBarCtrlT<ATL::CWindow>   CToolBarCtrl;


///////////////////////////////////////////////////////////////////////////////
// CStatusBarCtrl

template <class TBase>
class CStatusBarCtrlT : public TBase
{
public:
// Constructors
	CStatusBarCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CStatusBarCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Methods
	static LPCTSTR GetWndClassName()
	{
		return STATUSCLASSNAME;
	}

	int GetParts(int nParts, int* pParts) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SB_GETPARTS, nParts, (LPARAM)pParts);
	}

	BOOL SetParts(int nParts, int* pWidths)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SB_SETPARTS, nParts, (LPARAM)pWidths);
	}

	int GetTextLength(int nPane, int* pType = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, SB_GETTEXTLENGTH, (WPARAM)nPane, 0L);
		if (pType != NULL)
			*pType = (int)(short)HIWORD(dwRet);
		return (int)(short)LOWORD(dwRet);
	}

	int GetText(int nPane, LPTSTR lpszText, int* pType = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, SB_GETTEXT, (WPARAM)nPane, (LPARAM)lpszText);
		if(pType != NULL)
			*pType = (int)(short)HIWORD(dwRet);
		return (int)(short)LOWORD(dwRet);
	}

#ifndef _ATL_NO_COM
	BOOL GetTextBSTR(int nPane, BSTR& bstrText, int* pType = NULL) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		ATLASSERT(bstrText == NULL);
		int nLength = (int)(short)LOWORD(::SendMessage(m_hWnd, SB_GETTEXTLENGTH, (WPARAM)nPane, 0L));
		if(nLength == 0)
			return FALSE;

		CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPTSTR lpstrText = buff.Allocate(nLength + 1);
		if(lpstrText == NULL)
			return FALSE;

		if(!GetText(nPane, lpstrText, pType))
			return FALSE;

		bstrText = ::SysAllocString(T2OLE(lpstrText));
		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	int GetText(int nPane, _CSTRING_NS::CAtlString& strText, int* pType = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		int nLength = (int)(short)LOWORD(::SendMessage(m_hWnd, SB_GETTEXTLENGTH, (WPARAM)nPane, 0L));
		if(nLength == 0)
			return 0;

		LPTSTR lpstr = strText.GetBufferSetLength(nLength);
		if(lpstr == NULL)
			return 0;
		return GetText(nPane, lpstr, pType);
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	BOOL SetText(int nPane, LPCTSTR lpszText, int nType = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		return (BOOL)::SendMessage(m_hWnd, SB_SETTEXT, (nPane | nType), (LPARAM)lpszText);
	}

	BOOL GetRect(int nPane, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		return (BOOL)::SendMessage(m_hWnd, SB_GETRECT, nPane, (LPARAM)lpRect);
	}

	BOOL GetBorders(int* pBorders) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SB_GETBORDERS, 0, (LPARAM)pBorders);
	}

	BOOL GetBorders(int& nHorz, int& nVert, int& nSpacing) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		int borders[3] = { 0, 0, 0 };
		BOOL bResult = (BOOL)::SendMessage(m_hWnd, SB_GETBORDERS, 0, (LPARAM)&borders);
		if(bResult)
		{
			nHorz = borders[0];
			nVert = borders[1];
			nSpacing = borders[2];
		}
		return bResult;
	}

	void SetMinHeight(int nMin)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SB_SETMINHEIGHT, nMin, 0L);
	}

	BOOL SetSimple(BOOL bSimple = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SB_SIMPLE, bSimple, 0L);
	}

	BOOL IsSimple() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SB_ISSIMPLE, 0, 0L);
	}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SB_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SB_SETUNICODEFORMAT, bUnicode, 0L);
	}

	void GetTipText(int nPane, LPTSTR lpstrText, int nSize) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		::SendMessage(m_hWnd, SB_GETTIPTEXT, MAKEWPARAM(nPane, nSize), (LPARAM)lpstrText);
	}

	void SetTipText(int nPane, LPCTSTR lpstrText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		::SendMessage(m_hWnd, SB_SETTIPTEXT, nPane, (LPARAM)lpstrText);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if ((_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 0x0500))
	COLORREF SetBkColor(COLORREF clrBk)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, SB_SETBKCOLOR, 0, (LPARAM)clrBk);
	}

	HICON GetIcon(int nPane) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		return (HICON)::SendMessage(m_hWnd, SB_GETICON, nPane, 0L);
	}

	BOOL SetIcon(int nPane, HICON hIcon)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nPane < 256);
		return (BOOL)::SendMessage(m_hWnd, SB_SETICON, nPane, (LPARAM)hIcon);
	}
#endif // ((_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)) || (defined(_WIN32_WCE) && (_WIN32_WCE >= 0x0500))
};

typedef CStatusBarCtrlT<ATL::CWindow>   CStatusBarCtrl;


///////////////////////////////////////////////////////////////////////////////
// CTabCtrl

template <class TBase>
class CTabCtrlT : public TBase
{
public:
// Constructors
	CTabCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CTabCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_TABCONTROL;
	}

	CImageList GetImageList() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TCM_GETIMAGELIST, 0, 0L));
	}

	CImageList SetImageList(HIMAGELIST hImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, TCM_SETIMAGELIST, 0, (LPARAM)hImageList));
	}

	int GetItemCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETITEMCOUNT, 0, 0L);
	}

	BOOL GetItem(int nItem, LPTCITEM pTabCtrlItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)pTabCtrlItem);
	}

	BOOL SetItem(int nItem, LPTCITEM pTabCtrlItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nItem, (LPARAM)pTabCtrlItem);
	}

	int SetItem(int nItem, UINT mask, LPCTSTR lpszItem, DWORD dwState, DWORD dwStateMask, int iImage, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci = { 0 };
		tci.mask = mask;
		tci.pszText = (LPTSTR) lpszItem;
		tci.dwState = dwState;
		tci.dwStateMask = dwStateMask;
		tci.iImage = iImage;
		tci.lParam = lParam;
		return (int)::SendMessage(m_hWnd, TCM_SETITEM, nItem, (LPARAM)&tci);
	}

	BOOL GetItemRect(int nItem, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_GETITEMRECT, nItem, (LPARAM)lpRect);
	}

	int GetCurSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETCURSEL, 0, 0L);
	}

	int SetCurSel(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_SETCURSEL, nItem, 0L);
	}

	SIZE SetItemSize(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwSize = (DWORD)::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(size.cx, size.cy));
		SIZE sizeRet = { GET_X_LPARAM(dwSize), GET_Y_LPARAM(dwSize) };
		return sizeRet;
	}

	void SetItemSize(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
	}

	void SetPadding(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETPADDING, 0, MAKELPARAM(size.cx, size.cy));
	}

	int GetRowCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETROWCOUNT, 0, 0L);
	}

#ifndef _WIN32_WCE
	CToolTipCtrl GetTooltips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, TCM_GETTOOLTIPS, 0, 0L));
	}

	void SetTooltips(HWND hWndToolTip)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETTOOLTIPS, (WPARAM)hWndToolTip, 0L);
	}
#endif // !_WIN32_WCE

	int GetCurFocus() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETCURFOCUS, 0, 0L);
	}

	void SetCurFocus(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETCURFOCUS, nItem, 0L);
	}

	BOOL SetItemExtra(int cbExtra)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetItemCount() == 0);   // must be empty
		return (BOOL)::SendMessage(m_hWnd, TCM_SETITEMEXTRA, cbExtra, 0L);
	}

	int SetMinTabWidth(int nWidth = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_SETMINTABWIDTH, 0, nWidth);
	}

#if (_WIN32_IE >= 0x0400)
	DWORD GetExtendedStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TCM_GETEXTENDEDSTYLE, 0, 0L);
	}

	DWORD SetExtendedStyle(DWORD dwExMask, DWORD dwExStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TCM_SETEXTENDEDSTYLE, dwExMask, dwExStyle);
	}

#ifndef _WIN32_WCE
	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)

// Operations
	int InsertItem(int nItem, LPTCITEM pTabCtrlItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)pTabCtrlItem);
	}

	int InsertItem(int nItem, UINT mask, LPCTSTR lpszItem, int iImage, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci = { 0 };
		tci.mask = mask;
		tci.pszText = (LPTSTR) lpszItem;
		tci.iImage = iImage;
		tci.lParam = lParam;
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)&tci);
	}

	int InsertItem(int nItem, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = (LPTSTR) lpszItem;
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)&tci);
	}

	int AddItem(LPTCITEM pTabCtrlItem)
	{
		return InsertItem(GetItemCount(), pTabCtrlItem);
	}

	int AddItem(UINT mask, LPCTSTR lpszItem, int iImage, LPARAM lParam)
	{
		return InsertItem(GetItemCount(), mask, lpszItem, iImage, lParam);
	}

	int AddItem(LPCTSTR lpszItem)
	{
		return InsertItem(GetItemCount(), lpszItem);
	}

	BOOL DeleteItem(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_DELETEITEM, nItem, 0L);
	}

	BOOL DeleteAllItems()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_DELETEALLITEMS, 0, 0L);
	}

	void AdjustRect(BOOL bLarger, LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_ADJUSTRECT, bLarger, (LPARAM)lpRect);
	}

	void RemoveImage(int nImage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_REMOVEIMAGE, nImage, 0L);
	}

	int HitTest(TC_HITTESTINFO* pHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_HITTEST, 0, (LPARAM)pHitTestInfo);
	}

	void DeselectAll(BOOL bExcludeFocus = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_DESELECTALL, bExcludeFocus, 0L);
	}

#if (_WIN32_IE >= 0x0400)
	BOOL HighlightItem(int nIndex, BOOL bHighlight = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_HIGHLIGHTITEM, nIndex, MAKELPARAM(bHighlight, 0));
	}
#endif // (_WIN32_IE >= 0x0400)
};

typedef CTabCtrlT<ATL::CWindow>   CTabCtrl;


///////////////////////////////////////////////////////////////////////////////
// CTrackBarCtrl

template <class TBase>
class CTrackBarCtrlT : public TBase
{
public:
// Constructors
	CTrackBarCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CTrackBarCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return TRACKBAR_CLASS;
	}

	int GetLineSize() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETLINESIZE, 0, 0L);
	}

	int SetLineSize(int nSize)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_SETLINESIZE, 0, nSize);
	}

	int GetPageSize() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETPAGESIZE, 0, 0L);
	}

	int SetPageSize(int nSize)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_SETPAGESIZE, 0, nSize);
	}

	int GetRangeMin() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETRANGEMIN, 0, 0L);
	}

	void SetRangeMin(int nMin, BOOL bRedraw = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETRANGEMIN, bRedraw, nMin);
	}

	int GetRangeMax() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETRANGEMAX, 0, 0L);
	}

	void SetRangeMax(int nMax, BOOL bRedraw = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETRANGEMAX, bRedraw, nMax);
	}

	void GetRange(int& nMin, int& nMax) const
	{
		nMin = GetRangeMin();
		nMax = GetRangeMax();
	}

	void SetRange(int nMin, int nMax, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETRANGE, bRedraw, MAKELPARAM(nMin, nMax));
	}

	int GetSelStart() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETSELSTART, 0, 0L);
	}

	void SetSelStart(int nMin)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETSELSTART, 0, (LPARAM)nMin);
	}

	int GetSelEnd() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETSELEND, 0, 0L);
	}

	void SetSelEnd(int nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETSELEND, 0, (LPARAM)nMax);
	}

	void GetSelection(int& nMin, int& nMax) const
	{
		nMin = GetSelStart();
		nMax = GetSelEnd();
	}

	void SetSelection(int nMin, int nMax)
	{
		SetSelStart(nMin);
		SetSelEnd(nMax);
	}

	void GetChannelRect(LPRECT lprc) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_GETCHANNELRECT, 0, (LPARAM)lprc);
	}

	void GetThumbRect(LPRECT lprc) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_GETTHUMBRECT, 0, (LPARAM)lprc);
	}

	int GetPos() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETPOS, 0, 0L);
	}

	void SetPos(int nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETPOS, TRUE, nPos);
	}

	UINT GetNumTics() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, TBM_GETNUMTICS, 0, 0L);
	}

	DWORD* GetTicArray() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD*)::SendMessage(m_hWnd, TBM_GETPTICS, 0, 0L);
	}

	int GetTic(int nTic) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETTIC, nTic, 0L);
	}

	BOOL SetTic(int nTic)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TBM_SETTIC, 0, nTic);
	}

	int GetTicPos(int nTic) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETTICPOS, nTic, 0L);
	}

	void SetTicFreq(int nFreq)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETTICFREQ, nFreq, 0L);
	}

	int GetThumbLength() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_GETTHUMBLENGTH, 0, 0L);
	}

	void SetThumbLength(int nLength)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETTHUMBLENGTH, nLength, 0L);
	}

	void SetSel(int nStart, int nEnd, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & TBS_ENABLESELRANGE) != 0);
		::SendMessage(m_hWnd, TBM_SETSEL, bRedraw, MAKELPARAM(nStart, nEnd));
	}

	ATL::CWindow GetBuddy(BOOL bLeft = TRUE) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ATL::CWindow((HWND)::SendMessage(m_hWnd, TBM_GETBUDDY, bLeft, 0L));
	}

	ATL::CWindow SetBuddy(HWND hWndBuddy, BOOL bLeft = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ATL::CWindow((HWND)::SendMessage(m_hWnd, TBM_SETBUDDY, bLeft, (LPARAM)hWndBuddy));
	}

#ifndef _WIN32_WCE
	CToolTipCtrl GetToolTips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, TBM_GETTOOLTIPS, 0, 0L));
	}

	void SetToolTips(HWND hWndTT)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETTOOLTIPS, (WPARAM)hWndTT, 0L);
	}

	int SetTipSide(int nSide)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TBM_SETTIPSIDE, nSide, 0L);
	}
#endif // !_WIN32_WCE

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TBM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TBM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

// Operations
	void ClearSel(BOOL bRedraw = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_CLEARSEL, bRedraw, 0L);
	}

	void VerifyPos()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_SETPOS, FALSE, 0L);
	}

	void ClearTics(BOOL bRedraw = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TBM_CLEARTICS, bRedraw, 0L);
	}
};

typedef CTrackBarCtrlT<ATL::CWindow>   CTrackBarCtrl;


///////////////////////////////////////////////////////////////////////////////
// CUpDownCtrl

template <class TBase>
class CUpDownCtrlT : public TBase
{
public:
// Constructors
	CUpDownCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CUpDownCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return UPDOWN_CLASS;
	}

	UINT GetAccel(int nAccel, UDACCEL* pAccel) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)LOWORD(::SendMessage(m_hWnd, UDM_GETACCEL, nAccel, (LPARAM)pAccel));
	}

	BOOL SetAccel(int nAccel, UDACCEL* pAccel)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)LOWORD(::SendMessage(m_hWnd, UDM_SETACCEL, nAccel, (LPARAM)pAccel));
	}

	UINT GetBase() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)LOWORD(::SendMessage(m_hWnd, UDM_GETBASE, 0, 0L));
	}

	int SetBase(int nBase)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, UDM_SETBASE, nBase, 0L);
	}

	ATL::CWindow GetBuddy() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ATL::CWindow((HWND)::SendMessage(m_hWnd, UDM_GETBUDDY, 0, 0L));
	}

	ATL::CWindow SetBuddy(HWND hWndBuddy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ATL::CWindow((HWND)::SendMessage(m_hWnd, UDM_SETBUDDY, (WPARAM)hWndBuddy, 0L));
	}

	int GetPos(LPBOOL lpbError = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, UDM_GETPOS, 0, 0L);
		// Note: Seems that Windows always sets error to TRUE if
		// UDS_SETBUDDYINT style is not used
		if(lpbError != NULL)
			*lpbError = (HIWORD(dwRet) != 0) ? TRUE : FALSE;
		return (int)(short)LOWORD(dwRet);
	}

	int SetPos(int nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)(short)LOWORD(::SendMessage(m_hWnd, UDM_SETPOS, 0, MAKELPARAM(nPos, 0)));
	}

	DWORD GetRange() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, UDM_GETRANGE, 0, 0L);
	}

	void GetRange(int& nLower, int& nUpper) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, UDM_GETRANGE, 0, 0L);
		nLower = (int)(short)HIWORD(dwRet);
		nUpper = (int)(short)LOWORD(dwRet);
	}

	void SetRange(int nLower, int nUpper)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, UDM_SETRANGE, 0, MAKELPARAM(nUpper, nLower));
	}

#if (_WIN32_IE >= 0x0400)
	void SetRange32(int nLower, int nUpper)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, UDM_SETRANGE32, nLower, nUpper);
	}

	void GetRange32(int& nLower, int& nUpper) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, UDM_GETRANGE32, (WPARAM)&nLower, (LPARAM)&nUpper);
	}

#ifndef _WIN32_WCE
	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, UDM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, UDM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	int GetPos32(LPBOOL lpbError = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		// Note: Seems that Windows always sets error to TRUE if
		// UDS_SETBUDDYINT style is not used
		return (int)::SendMessage(m_hWnd, UDM_GETPOS32, 0, (LPARAM)lpbError);
	}

	int SetPos32(int nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, UDM_SETPOS32, 0, (LPARAM)nPos);
	}
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
};

typedef CUpDownCtrlT<ATL::CWindow>   CUpDownCtrl;


///////////////////////////////////////////////////////////////////////////////
// CProgressBarCtrl

template <class TBase>
class CProgressBarCtrlT : public TBase
{
public:
// Constructors
	CProgressBarCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CProgressBarCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return PROGRESS_CLASS;
	}

	DWORD SetRange(int nLower, int nUpper)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, PBM_SETRANGE, 0, MAKELPARAM(nLower, nUpper));
	}

	int SetPos(int nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)(short)LOWORD(::SendMessage(m_hWnd, PBM_SETPOS, nPos, 0L));
	}

	int OffsetPos(int nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)(short)LOWORD(::SendMessage(m_hWnd, PBM_DELTAPOS, nPos, 0L));
	}

	int SetStep(int nStep)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)(short)LOWORD(::SendMessage(m_hWnd, PBM_SETSTEP, nStep, 0L));
	}

	UINT GetPos() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, PBM_GETPOS, 0, 0L);
	}

	void GetRange(PPBRANGE pPBRange) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pPBRange != NULL);
		::SendMessage(m_hWnd, PBM_GETRANGE, TRUE, (LPARAM)pPBRange);
	}

	void GetRange(int& nLower, int& nUpper) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		PBRANGE range = { 0 };
		::SendMessage(m_hWnd, PBM_GETRANGE, TRUE, (LPARAM)&range);
		nLower = range.iLow;
		nUpper = range.iHigh;
	}

	int GetRangeLimit(BOOL bLowLimit) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_GETRANGE, bLowLimit, (LPARAM)NULL);
	}

	DWORD SetRange32(int nMin, int nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, PBM_SETRANGE32, nMin, nMax);
	}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	COLORREF SetBarColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PBM_SETBARCOLOR, 0, (LPARAM)clr);
	}

	COLORREF SetBkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PBM_SETBKCOLOR, 0, (LPARAM)clr);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if (_WIN32_WINNT >= 0x0501) && defined(PBM_SETMARQUEE)
	BOOL SetMarquee(BOOL bMarquee, UINT uUpdateTime = 0U)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PBM_SETMARQUEE, (WPARAM)bMarquee, (LPARAM)uUpdateTime);
	}
#endif // (_WIN32_WINNT >= 0x0501) && defined(PBM_SETMARQUEE)

#if (_WIN32_WINNT >= 0x0600)
	int GetStep() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_GETSTEP, 0, 0L);
	}

	COLORREF GetBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PBM_GETBKCOLOR, 0, 0L);
	}

	COLORREF GetBarColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PBM_GETBARCOLOR, 0, 0L);
	}

	int GetState() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_GETSTATE, 0, 0L);
	}

	int SetState(int nState)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PBM_SETSTATE, nState, 0L);
	}
#endif // (_WIN32_WINNT >= 0x0600)

// Operations
	int StepIt()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)(short)LOWORD(::SendMessage(m_hWnd, PBM_STEPIT, 0, 0L));
	}
};

typedef CProgressBarCtrlT<ATL::CWindow>   CProgressBarCtrl;


///////////////////////////////////////////////////////////////////////////////
// CHotKeyCtrl

#ifndef _WIN32_WCE

template <class TBase>
class CHotKeyCtrlT : public TBase
{
public:
// Constructors
	CHotKeyCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CHotKeyCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return HOTKEY_CLASS;
	}

	DWORD GetHotKey() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, HKM_GETHOTKEY, 0, 0L);
	}

	void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dw = (DWORD)::SendMessage(m_hWnd, HKM_GETHOTKEY, 0, 0L);
		wVirtualKeyCode = LOBYTE(LOWORD(dw));
		wModifiers = HIBYTE(LOWORD(dw));
	}

	void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, HKM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0L);
	}

	void SetRules(WORD wInvalidComb, WORD wModifiers)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, HKM_SETRULES, wInvalidComb, MAKELPARAM(wModifiers, 0));
	}
};

typedef CHotKeyCtrlT<ATL::CWindow>   CHotKeyCtrl;

#endif // !_WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CAnimateCtrl

#ifndef _WIN32_WCE

template <class TBase>
class CAnimateCtrlT : public TBase
{
public:
// Constructors
	CAnimateCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CAnimateCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return ANIMATE_CLASS;
	}

// Operations
	BOOL Open(ATL::_U_STRINGorID FileName)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ACM_OPEN, 0, (LPARAM)FileName.m_lpstr);
	}

	BOOL Play(UINT nFrom, UINT nTo, UINT nRep)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ACM_PLAY, nRep, MAKELPARAM(nFrom, nTo));
	}

	BOOL Stop()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ACM_STOP, 0, 0L);
	}

	BOOL Close()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ACM_OPEN, 0, 0L);
	}

	BOOL Seek(UINT nTo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ACM_PLAY, 0, MAKELPARAM(nTo, nTo));
	}

	// Vista only
	BOOL IsPlaying() const
	{
#ifndef ACM_ISPLAYING
		const UINT ACM_ISPLAYING = (WM_USER+104);
#endif
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ACM_ISPLAYING, 0, 0L);
	}
};

typedef CAnimateCtrlT<ATL::CWindow>   CAnimateCtrl;

#endif // !_WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CRichEditCtrl

#ifndef _WIN32_WCE

#ifdef _UNICODE
#if (_RICHEDIT_VER == 0x0100)
#undef RICHEDIT_CLASS
#define RICHEDIT_CLASS	L"RICHEDIT"
#endif // (_RICHEDIT_VER == 0x0100)
#endif // _UNICODE

template <class TBase>
class CRichEditCtrlT : public TBase
{
public:
// Constructors
	CRichEditCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CRichEditCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return RICHEDIT_CLASS;
	}

	static LPCTSTR GetLibraryName()
	{
#if (_RICHEDIT_VER >= 0x0200)
		return _T("RICHED20.DLL");
#else
		return _T("RICHED32.DLL");
#endif
	}

	int GetLineCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0L);
	}

	BOOL GetModify() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_GETMODIFY, 0, 0L);
	}

	void SetModify(BOOL bModified = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETMODIFY, bModified, 0L);
	}

	void GetRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_GETRECT, 0, (LPARAM)lpRect);
	}

	DWORD GetOptions() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_GETOPTIONS, 0, 0L);
	}

	DWORD SetOptions(WORD wOperation, DWORD dwOptions)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_SETOPTIONS, wOperation, dwOptions);
	}

	// NOTE: first word in lpszBuffer must contain the size of the buffer!
	int GetLine(int nIndex, LPTSTR lpszBuffer) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETLINE, nIndex, (LPARAM)lpszBuffer);
	}

	int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		*(LPWORD)lpszBuffer = (WORD)nMaxLength;
		return (int)::SendMessage(m_hWnd, EM_GETLINE, nIndex, (LPARAM)lpszBuffer);
	}

	BOOL CanUndo() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_CANUNDO, 0, 0L);
	}

	BOOL CanPaste(UINT nFormat = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_CANPASTE, nFormat, 0L);
	}

	void GetSel(LONG& nStartChar, LONG& nEndChar) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHARRANGE cr = { 0, 0 };
		::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
		nStartChar = cr.cpMin;
		nEndChar = cr.cpMax;
	}

	void GetSel(CHARRANGE &cr) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
	}

	int SetSel(LONG nStartChar, LONG nEndChar)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHARRANGE cr = { nStartChar, nEndChar };
		return (int)::SendMessage(m_hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	}

	int SetSel(CHARRANGE &cr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	}

	int SetSelAll()
	{
		return SetSel(0, -1);
	}

	int SetSelNone()
	{
		return SetSel(-1, 0);
	}

	DWORD GetDefaultCharFormat(CHARFORMAT& cf) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT);
		return (DWORD)::SendMessage(m_hWnd, EM_GETCHARFORMAT, 0, (LPARAM)&cf);
	}

	DWORD GetSelectionCharFormat(CHARFORMAT& cf) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT);
		return (DWORD)::SendMessage(m_hWnd, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
	}

	DWORD GetEventMask() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_GETEVENTMASK, 0, 0L);
	}

	LONG GetLimitText() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_GETLIMITTEXT, 0, 0L);
	}

	DWORD GetParaFormat(PARAFORMAT& pf) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		pf.cbSize = sizeof(PARAFORMAT);
		return (DWORD)::SendMessage(m_hWnd, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
	}

#if (_RICHEDIT_VER >= 0x0200)
	LONG GetSelText(LPTSTR lpstrBuff) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpstrBuff);
	}
#else // !(_RICHEDIT_VER >= 0x0200)
	// RichEdit 1.0 EM_GETSELTEXT is ANSI only
	LONG GetSelText(LPSTR lpstrBuff) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpstrBuff);
	}
#endif // !(_RICHEDIT_VER >= 0x0200)

#ifndef _ATL_NO_COM
	BOOL GetSelTextBSTR(BSTR& bstrText) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrText == NULL);

		CHARRANGE cr = { 0, 0 };
		::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);

#if (_RICHEDIT_VER >= 0x0200)
		CTempBuffer<TCHAR, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPTSTR lpstrText = buff.Allocate(cr.cpMax - cr.cpMin + 1);
		if(lpstrText == NULL)
			return FALSE;
		if(::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpstrText) == 0)
			return FALSE;

		bstrText = ::SysAllocString(T2W(lpstrText));
#else // !(_RICHEDIT_VER >= 0x0200)
		CTempBuffer<char, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPSTR lpstrText = buff.Allocate(cr.cpMax - cr.cpMin + 1);
		if(lpstrText == NULL)
			return FALSE;
		if(::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpstrText) == 0)
			return FALSE;

		bstrText = ::SysAllocString(A2W(lpstrText));
#endif // !(_RICHEDIT_VER >= 0x0200)

		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	LONG GetSelText(_CSTRING_NS::CAtlString& strText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));

		CHARRANGE cr = { 0, 0 };
		::SendMessage(m_hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);

#if (_RICHEDIT_VER >= 0x0200)
		LONG lLen = 0;
		LPTSTR lpstrText = strText.GetBufferSetLength(cr.cpMax - cr.cpMin);
		if(lpstrText != NULL)
		{
			lLen = (LONG)::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpstrText);
			strText.ReleaseBuffer();
		}
#else // !(_RICHEDIT_VER >= 0x0200)
		CTempBuffer<char, _WTL_STACK_ALLOC_THRESHOLD> buff;
		LPSTR lpstrText = buff.Allocate(cr.cpMax - cr.cpMin + 1);
		if(lpstrText == NULL)
			return 0;
		LONG lLen = (LONG)::SendMessage(m_hWnd, EM_GETSELTEXT, 0, (LPARAM)lpstrText);
		if(lLen == 0)
			return 0;

		USES_CONVERSION;
		strText = A2T(lpstrText);
#endif // !(_RICHEDIT_VER >= 0x0200)

		return lLen;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	WORD GetSelectionType() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (WORD)::SendMessage(m_hWnd, EM_SELECTIONTYPE, 0, 0L);
	}

	COLORREF SetBackgroundColor(COLORREF cr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, EM_SETBKGNDCOLOR, 0, cr);
	}

	COLORREF SetBackgroundColor()   // sets to system background
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, EM_SETBKGNDCOLOR, 1, 0);
	}

	BOOL SetCharFormat(CHARFORMAT& cf, WORD wFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, (WPARAM)wFlags, (LPARAM)&cf);
	}

	BOOL SetDefaultCharFormat(CHARFORMAT& cf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, 0, (LPARAM)&cf);
	}

	BOOL SetSelectionCharFormat(CHARFORMAT& cf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}

	BOOL SetWordCharFormat(CHARFORMAT& cf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
	}

	DWORD SetEventMask(DWORD dwEventMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_SETEVENTMASK, 0, dwEventMask);
	}

	BOOL SetParaFormat(PARAFORMAT& pf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		pf.cbSize = sizeof(PARAFORMAT);
		return (BOOL)::SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	}

	BOOL SetTargetDevice(HDC hDC, int cxLineWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTARGETDEVICE, (WPARAM)hDC, cxLineWidth);
	}

	int GetTextLength() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0L);
	}

	BOOL SetReadOnly(BOOL bReadOnly = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETREADONLY, bReadOnly, 0L);
	}

	int GetFirstVisibleLine() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
	}

	EDITWORDBREAKPROCEX GetWordBreakProcEx() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (EDITWORDBREAKPROCEX)::SendMessage(m_hWnd, EM_GETWORDBREAKPROCEX, 0, 0L);
	}

	EDITWORDBREAKPROCEX SetWordBreakProcEx(EDITWORDBREAKPROCEX pfnEditWordBreakProcEx)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (EDITWORDBREAKPROCEX)::SendMessage(m_hWnd, EM_SETWORDBREAKPROCEX, 0, (LPARAM)pfnEditWordBreakProcEx);
	}

	int GetTextRange(TEXTRANGE* pTextRange) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)pTextRange);
	}

#if (_RICHEDIT_VER >= 0x0200)
	int GetTextRange(LONG nStartChar, LONG nEndChar, LPTSTR lpstrText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TEXTRANGE tr = { 0 };
		tr.chrg.cpMin = nStartChar;
		tr.chrg.cpMax = nEndChar;
		tr.lpstrText = lpstrText;
		return (int)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}
#else // !(_RICHEDIT_VER >= 0x0200)

	int GetTextRange(LONG nStartChar, LONG nEndChar, LPSTR lpstrText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TEXTRANGE tr = { 0 };
		tr.chrg.cpMin = nStartChar;
		tr.chrg.cpMax = nEndChar;
		tr.lpstrText = lpstrText;
		return (int)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}
#endif // !(_RICHEDIT_VER >= 0x0200)

#if (_RICHEDIT_VER >= 0x0200)
	DWORD GetDefaultCharFormat(CHARFORMAT2& cf) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT2);
		return (DWORD)::SendMessage(m_hWnd, EM_GETCHARFORMAT, 0, (LPARAM)&cf);
	}

	BOOL SetCharFormat(CHARFORMAT2& cf, WORD wFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, (WPARAM)wFlags, (LPARAM)&cf);
	}

	BOOL SetDefaultCharFormat(CHARFORMAT2& cf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, 0, (LPARAM)&cf);
	}

	DWORD GetSelectionCharFormat(CHARFORMAT2& cf) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT2);
		return (DWORD)::SendMessage(m_hWnd, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
	}

	BOOL SetSelectionCharFormat(CHARFORMAT2& cf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}

	BOOL SetWordCharFormat(CHARFORMAT2& cf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		cf.cbSize = sizeof(CHARFORMAT2);
		return (BOOL)::SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
	}

	DWORD GetParaFormat(PARAFORMAT2& pf) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		pf.cbSize = sizeof(PARAFORMAT2);
		return (DWORD)::SendMessage(m_hWnd, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
	}

	BOOL SetParaFormat(PARAFORMAT2& pf)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		pf.cbSize = sizeof(PARAFORMAT2);
		return (BOOL)::SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	}

	TEXTMODE GetTextMode() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (TEXTMODE)::SendMessage(m_hWnd, EM_GETTEXTMODE, 0, 0L);
	}

	BOOL SetTextMode(TEXTMODE enumTextMode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return !(BOOL)::SendMessage(m_hWnd, EM_SETTEXTMODE, enumTextMode, 0L);
	}

	UNDONAMEID GetUndoName() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UNDONAMEID)::SendMessage(m_hWnd, EM_GETUNDONAME, 0, 0L);
	}

	UNDONAMEID GetRedoName() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UNDONAMEID)::SendMessage(m_hWnd, EM_GETREDONAME, 0, 0L);
	}

	BOOL CanRedo() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_CANREDO, 0, 0L);
	}

	BOOL GetAutoURLDetect() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_GETAUTOURLDETECT, 0, 0L);
	}

	BOOL SetAutoURLDetect(BOOL bAutoDetect = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return !(BOOL)::SendMessage(m_hWnd, EM_AUTOURLDETECT, bAutoDetect, 0L);
	}

	// this method is deprecated, please use SetAutoURLDetect
	BOOL EnableAutoURLDetect(BOOL bEnable = TRUE) { return SetAutoURLDetect(bEnable); }

	UINT SetUndoLimit(UINT uUndoLimit)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, EM_SETUNDOLIMIT, uUndoLimit, 0L);
	}

	void SetPalette(HPALETTE hPalette)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETPALETTE, (WPARAM)hPalette, 0L);
	}

	int GetTextEx(GETTEXTEX* pGetTextEx, LPTSTR lpstrText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETTEXTEX, (WPARAM)pGetTextEx, (LPARAM)lpstrText);
	}

	int GetTextEx(LPTSTR lpstrText, int nTextLen, DWORD dwFlags = GT_DEFAULT, UINT uCodePage = CP_ACP, LPCSTR lpDefaultChar = NULL, LPBOOL lpUsedDefChar = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		GETTEXTEX gte = { 0 };
		gte.cb = nTextLen * sizeof(TCHAR);
		gte.codepage = uCodePage;
		gte.flags = dwFlags;
		gte.lpDefaultChar = lpDefaultChar;
		gte.lpUsedDefChar = lpUsedDefChar;
		return (int)::SendMessage(m_hWnd, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)lpstrText);
	}

	int GetTextLengthEx(GETTEXTLENGTHEX* pGetTextLengthEx) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM)pGetTextLengthEx, 0L);
	}

	int GetTextLengthEx(DWORD dwFlags = GTL_DEFAULT, UINT uCodePage = CP_ACP) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		GETTEXTLENGTHEX gtle = { 0 };
		gtle.codepage = uCodePage;
		gtle.flags = dwFlags;
		return (int)::SendMessage(m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtle, 0L);
	}
#endif // (_RICHEDIT_VER >= 0x0200)

#if (_RICHEDIT_VER >= 0x0300)
	int SetTextEx(SETTEXTEX* pSetTextEx, LPCTSTR lpstrText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_SETTEXTEX, (WPARAM)pSetTextEx, (LPARAM)lpstrText);
	}

	int SetTextEx(LPCTSTR lpstrText, DWORD dwFlags = ST_DEFAULT, UINT uCodePage = CP_ACP)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SETTEXTEX ste = { 0 };
		ste.flags = dwFlags;
		ste.codepage = uCodePage;
		return (int)::SendMessage(m_hWnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpstrText);
	}

	int GetEditStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_GETEDITSTYLE, 0, 0L);
	}

	int SetEditStyle(int nStyle, int nMask = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if(nMask == -1)
			nMask = nStyle;   // set everything specified
		return (int)::SendMessage(m_hWnd, EM_SETEDITSTYLE, nStyle, nMask);
	}

	BOOL SetFontSize(int nFontSizeDelta)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nFontSizeDelta >= -1637 && nFontSizeDelta <= 1638);
		return (BOOL)::SendMessage(m_hWnd, EM_SETFONTSIZE, nFontSizeDelta, 0L);
	}

	void GetScrollPos(LPPOINT lpPoint) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpPoint != NULL);
		::SendMessage(m_hWnd, EM_GETSCROLLPOS, 0, (LPARAM)lpPoint);
	}

	void SetScrollPos(LPPOINT lpPoint)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpPoint != NULL);
		::SendMessage(m_hWnd, EM_SETSCROLLPOS, 0, (LPARAM)lpPoint);
	}

	BOOL GetZoom(int& nNum, int& nDen) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_GETZOOM, (WPARAM)&nNum, (LPARAM)&nDen);
	}

	BOOL SetZoom(int nNum, int nDen)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nNum >= 0 && nNum <= 64);
		ATLASSERT(nDen >= 0 && nDen <= 64);
		return (BOOL)::SendMessage(m_hWnd, EM_SETZOOM, nNum, nDen);
	}

	BOOL SetZoomOff()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETZOOM, 0, 0L);
	}
#endif // (_RICHEDIT_VER >= 0x0300)

// Operations
	void LimitText(LONG nChars = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_EXLIMITTEXT, 0, nChars);
	}

	int LineFromChar(LONG nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_EXLINEFROMCHAR, 0, nIndex);
	}

	POINT PosFromChar(LONG nChar) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		POINT point = { 0, 0 };
		::SendMessage(m_hWnd, EM_POSFROMCHAR, (WPARAM)&point, nChar);
		return point;
	}

	int CharFromPos(POINT pt) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		POINTL ptl = { pt.x, pt.y };
		return (int)::SendMessage(m_hWnd, EM_CHARFROMPOS, 0, (LPARAM)&ptl);
	}

	void EmptyUndoBuffer()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_EMPTYUNDOBUFFER, 0, 0L);
	}

	int LineIndex(int nLine = -1) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_LINEINDEX, nLine, 0L);
	}

	int LineLength(int nLine = -1) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, EM_LINELENGTH, nLine, 0L);
	}

	BOOL LineScroll(int nLines, int nChars = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_LINESCROLL, nChars, nLines);
	}

	void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpszNewText);
	}

	void SetRect(LPCRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SETRECT, 0, (LPARAM)lpRect);
	}

	BOOL DisplayBand(LPRECT pDisplayRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_DISPLAYBAND, 0, (LPARAM)pDisplayRect);
	}

	LONG FindText(DWORD dwFlags, FINDTEXT& ft) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
#if (_RICHEDIT_VER >= 0x0200) && defined(_UNICODE)
		return (LONG)::SendMessage(m_hWnd, EM_FINDTEXTW, dwFlags, (LPARAM)&ft);
#else
		return (LONG)::SendMessage(m_hWnd, EM_FINDTEXT, dwFlags, (LPARAM)&ft);
#endif
	}

	LONG FindText(DWORD dwFlags, FINDTEXTEX& ft) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
#if (_RICHEDIT_VER >= 0x0200) && defined(_UNICODE)
		return (LONG)::SendMessage(m_hWnd, EM_FINDTEXTEXW, dwFlags, (LPARAM)&ft);
#else
		return (LONG)::SendMessage(m_hWnd, EM_FINDTEXTEX, dwFlags, (LPARAM)&ft);
#endif
	}

	LONG FormatRange(FORMATRANGE& fr, BOOL bDisplay = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_FORMATRANGE, bDisplay, (LPARAM)&fr);
	}

	LONG FormatRange(FORMATRANGE* pFormatRange, BOOL bDisplay = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_FORMATRANGE, bDisplay, (LPARAM)pFormatRange);
	}

	void HideSelection(BOOL bHide = TRUE, BOOL bChangeStyle = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_HIDESELECTION, bHide, bChangeStyle);
	}

	void PasteSpecial(UINT uClipFormat, DWORD dwAspect = 0, HMETAFILE hMF = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		REPASTESPECIAL reps = { dwAspect, (DWORD_PTR)hMF };
		::SendMessage(m_hWnd, EM_PASTESPECIAL, uClipFormat, (LPARAM)&reps);
	}

	void RequestResize()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_REQUESTRESIZE, 0, 0L);
	}

	LONG StreamIn(UINT uFormat, EDITSTREAM& es)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_STREAMIN, uFormat, (LPARAM)&es);
	}

	LONG StreamOut(UINT uFormat, EDITSTREAM& es)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (LONG)::SendMessage(m_hWnd, EM_STREAMOUT, uFormat, (LPARAM)&es);
	}

	DWORD FindWordBreak(int nCode, LONG nStartChar)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, EM_FINDWORDBREAK, nCode, nStartChar);
	}

	// Additional operations
	void ScrollCaret()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0L);
	}

	int InsertText(long nInsertAfterChar, LPCTSTR lpstrText, BOOL bCanUndo = FALSE)
	{
		int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
		ReplaceSel(lpstrText, bCanUndo);
		return nRet;
	}

	int AppendText(LPCTSTR lpstrText, BOOL bCanUndo = FALSE)
	{
		return InsertText(GetWindowTextLength(), lpstrText, bCanUndo);
	}

	// Clipboard operations
	BOOL Undo()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_UNDO, 0, 0L);
	}

	void Clear()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_CLEAR, 0, 0L);
	}

	void Copy()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_COPY, 0, 0L);
	}

	void Cut()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_CUT, 0, 0L);
	}

	void Paste()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_PASTE, 0, 0L);
	}

	// OLE support
	IRichEditOle* GetOleInterface() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		IRichEditOle *pRichEditOle = NULL;
		::SendMessage(m_hWnd, EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle);
		return pRichEditOle;
	}

	BOOL SetOleCallback(IRichEditOleCallback* pCallback)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETOLECALLBACK, 0, (LPARAM)pCallback);
	}

#if (_RICHEDIT_VER >= 0x0200)
	BOOL Redo()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_REDO, 0, 0L);
	}

	void StopGroupTyping()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_STOPGROUPTYPING, 0, 0L);
	}

	void ShowScrollBar(int nBarType, BOOL bVisible = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, EM_SHOWSCROLLBAR, nBarType, bVisible);
	}
#endif // (_RICHEDIT_VER >= 0x0200)

#if (_RICHEDIT_VER >= 0x0300)
	BOOL SetTabStops(int nTabStops, LPINT rgTabStops)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTABSTOPS, nTabStops, (LPARAM)rgTabStops);
	}

	BOOL SetTabStops()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTABSTOPS, 0, 0L);
	}

	BOOL SetTabStops(const int& cxEachStop)    // takes an 'int'
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, EM_SETTABSTOPS, 1, (LPARAM)(LPINT)&cxEachStop);
	}
#endif // (_RICHEDIT_VER >= 0x0300)
};

typedef CRichEditCtrlT<ATL::CWindow>   CRichEditCtrl;

#endif // !_WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CRichEditCommands - message handlers for standard EDIT commands

#ifndef _WIN32_WCE

// Chain to CRichEditCommands message map. Your class must also derive from CRichEditCtrl.
// Example:
// class CMyRichEdit : public CWindowImpl<CMyRichEdit, CRichEditCtrl>,
//                     public CRichEditCommands<CMyRichEdit>
// {
// public:
//      BEGIN_MSG_MAP(CMyRichEdit)
//              // your handlers...
//              CHAIN_MSG_MAP_ALT(CRichEditCommands<CMyRichEdit>, 1)
//      END_MSG_MAP()
//      // other stuff...
// };

template <class T>
class CRichEditCommands : public CEditCommands< T >
{
public:
	BEGIN_MSG_MAP(CRichEditCommands< T >)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR, CEditCommands< T >::OnEditClear)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR_ALL, CEditCommands< T >::OnEditClearAll)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, CEditCommands< T >::OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, CEditCommands< T >::OnEditCut)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, CEditCommands< T >::OnEditPaste)
		COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, CEditCommands< T >::OnEditSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, CEditCommands< T >::OnEditUndo)
#if (_RICHEDIT_VER >= 0x0200)
		COMMAND_ID_HANDLER(ID_EDIT_REDO, OnEditRedo)
#endif // (_RICHEDIT_VER >= 0x0200)
	END_MSG_MAP()

#if (_RICHEDIT_VER >= 0x0200)
	LRESULT OnEditRedo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->Redo();
		return 0;
	}
#endif // (_RICHEDIT_VER >= 0x0200)

// State (update UI) helpers
	BOOL CanCut() const
	{ return HasSelection(); }

	BOOL CanCopy() const
	{ return HasSelection(); }

	BOOL CanClear() const
	{ return HasSelection(); }

// Implementation
	BOOL HasSelection() const
	{
		const T* pT = static_cast<const T*>(this);
		return (pT->GetSelectionType() != SEL_EMPTY);
	}
};

#endif // _WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CDragListBox

#ifndef _WIN32_WCE

template <class TBase>
class CDragListBoxT : public CListBoxT< TBase >
{
public:
// Constructors
	CDragListBoxT(HWND hWnd = NULL) : CListBoxT< TBase >(hWnd)
	{ }

	CDragListBoxT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		HWND hWnd = TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
		if(hWnd != NULL)
			MakeDragList();
		return hWnd;
	}

// Operations
	BOOL MakeDragList()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((GetStyle() & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) == 0);
		return ::MakeDragList(m_hWnd);
	}

	int LBItemFromPt(POINT pt, BOOL bAutoScroll = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::LBItemFromPt(m_hWnd, pt, bAutoScroll);
	}

	void DrawInsert(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::DrawInsert(GetParent(), m_hWnd, nItem);
	}

	static UINT GetDragListMessage()
	{
		static UINT uDragListMessage = 0;
		if(uDragListMessage == 0)
		{
			CStaticDataInitCriticalSectionLock lock;
			if(FAILED(lock.Lock()))
			{
				ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CDragListBox::GetDragListMessage.\n"));
				ATLASSERT(FALSE);
				return 0;
			}

			if(uDragListMessage == 0)
				uDragListMessage = ::RegisterWindowMessage(DRAGLISTMSGSTRING);

			lock.Unlock();
		}
		ATLASSERT(uDragListMessage != 0);
		return uDragListMessage;
	}
};

typedef CDragListBoxT<ATL::CWindow>   CDragListBox;

template <class T>
class CDragListNotifyImpl
{
public:
	BEGIN_MSG_MAP(CDragListNotifyImpl< T >)
		MESSAGE_HANDLER(CDragListBox::GetDragListMessage(), OnDragListNotify)
	END_MSG_MAP()

	LRESULT OnDragListNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		uMsg;   // avoid level 4 warning
		ATLASSERT(uMsg == CDragListBox::GetDragListMessage());
		T* pT = static_cast<T*>(this);
		LPDRAGLISTINFO lpDragListInfo = (LPDRAGLISTINFO)lParam;
		LRESULT lRet = 0;
		switch(lpDragListInfo->uNotification)
		{
		case DL_BEGINDRAG:
			lRet = (LPARAM)pT->OnBeginDrag((int)wParam, lpDragListInfo->hWnd, lpDragListInfo->ptCursor);
			break;
		case DL_CANCELDRAG:
			pT->OnCancelDrag((int)wParam, lpDragListInfo->hWnd, lpDragListInfo->ptCursor);
			break;
		case DL_DRAGGING:
			lRet = (LPARAM)pT->OnDragging((int)wParam, lpDragListInfo->hWnd, lpDragListInfo->ptCursor);
			break;
		case DL_DROPPED:
			pT->OnDropped((int)wParam, lpDragListInfo->hWnd, lpDragListInfo->ptCursor);
			break;
		default:
			ATLTRACE2(atlTraceUI, 0, _T("Unknown DragListBox notification\n"));
			bHandled = FALSE;   // don't handle it
			break;
		}
		return lRet;
	}

// Overrideables
	BOOL OnBeginDrag(int /*nCtlID*/, HWND /*hWndDragList*/, POINT /*ptCursor*/)
	{
		return TRUE;   // allow dragging
	}

	void OnCancelDrag(int /*nCtlID*/, HWND /*hWndDragList*/, POINT /*ptCursor*/)
	{
		// nothing to do
	}

	int OnDragging(int /*nCtlID*/, HWND /*hWndDragList*/, POINT /*ptCursor*/)
	{
		return 0;   // don't change cursor
	}

	void OnDropped(int /*nCtlID*/, HWND /*hWndDragList*/, POINT /*ptCursor*/)
	{
		// nothing to do
	}
};

#endif // _WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CReBarCtrl

template <class TBase>
class CReBarCtrlT : public TBase
{
public:
// Constructors
	CReBarCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CReBarCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return REBARCLASSNAME;
	}

	UINT GetBandCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, RB_GETBANDCOUNT, 0, 0L);
	}

	BOOL GetBandInfo(int nBand, LPREBARBANDINFO lprbbi) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_GETBANDINFO, nBand, (LPARAM)lprbbi);
	}

	BOOL SetBandInfo(int nBand, LPREBARBANDINFO lprbbi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_SETBANDINFO, nBand, (LPARAM)lprbbi);
	}

	BOOL GetBarInfo(LPREBARINFO lprbi) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_GETBARINFO, 0, (LPARAM)lprbi);
	}

	BOOL SetBarInfo(LPREBARINFO lprbi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_SETBARINFO, 0, (LPARAM)lprbi);
	}

	CImageList GetImageList() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		REBARINFO rbi = { 0 };
		rbi.cbSize = sizeof(REBARINFO);
		rbi.fMask = RBIM_IMAGELIST;
		if( (BOOL)::SendMessage(m_hWnd, RB_GETBARINFO, 0, (LPARAM)&rbi) == FALSE ) return CImageList();
		return CImageList(rbi.himl);
	}

	BOOL SetImageList(HIMAGELIST hImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		REBARINFO rbi = { 0 };
		rbi.cbSize = sizeof(REBARINFO);
		rbi.fMask = RBIM_IMAGELIST;
		rbi.himl = hImageList;
		return (BOOL)::SendMessage(m_hWnd, RB_SETBARINFO, 0, (LPARAM)&rbi);
	}

	UINT GetRowCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, RB_GETROWCOUNT, 0, 0L);
	}

	UINT GetRowHeight(int nBand) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, RB_GETROWHEIGHT, nBand, 0L);
	}

#if (_WIN32_IE >= 0x0400)
	COLORREF GetTextColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RB_GETTEXTCOLOR, 0, 0L);
	}

	COLORREF SetTextColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RB_SETTEXTCOLOR, 0, (LPARAM)clr);
	}

	COLORREF GetBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RB_GETBKCOLOR, 0, 0L);
	}

	COLORREF SetBkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, RB_SETBKCOLOR, 0, (LPARAM)clr);
	}

	UINT GetBarHeight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (UINT)::SendMessage(m_hWnd, RB_GETBARHEIGHT, 0, 0L);
	}

	BOOL GetRect(int nBand, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_GETRECT, nBand, (LPARAM)lpRect);
	}

#ifndef _WIN32_WCE
	CToolTipCtrl GetToolTips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, RB_GETTOOLTIPS, 0, 0L));
	}

	void SetToolTips(HWND hwndToolTip)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_SETTOOLTIPS, (WPARAM)hwndToolTip, 0L);
	}
#endif // !_WIN32_WCE

	void GetBandBorders(int nBand, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpRect != NULL);
		::SendMessage(m_hWnd, RB_GETBANDBORDERS, nBand, (LPARAM)lpRect);
	}

#ifndef _WIN32_WCE
	BOOL GetColorScheme(LPCOLORSCHEME lpColorScheme) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpColorScheme != NULL);
		return (BOOL)::SendMessage(m_hWnd, RB_GETCOLORSCHEME, 0, (LPARAM)lpColorScheme);
	}

	void SetColorScheme(LPCOLORSCHEME lpColorScheme)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpColorScheme != NULL);
		::SendMessage(m_hWnd, RB_SETCOLORSCHEME, 0, (LPARAM)lpColorScheme);
	}

	HPALETTE GetPalette() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HPALETTE)::SendMessage(m_hWnd, RB_GETPALETTE, 0, 0L);
	}

	HPALETTE SetPalette(HPALETTE hPalette)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HPALETTE)::SendMessage(m_hWnd, RB_SETPALETTE, 0, (LPARAM)hPalette);
	}

	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_WINNT >= 0x0501)
	// requires uxtheme.h to be included to use MARGINS struct
#ifndef _UXTHEME_H_
	typedef struct _MARGINS*   PMARGINS;
#endif // !_UXTHEME_H_
	void GetBandMargins(PMARGINS pMargins) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_GETBANDMARGINS, 0, (LPARAM)pMargins);
	}

	void SetWindowTheme(LPCWSTR lpstrTheme)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_SETWINDOWTHEME, 0, (LPARAM)lpstrTheme);
	}
#endif // (_WIN32_WINNT >= 0x0501)

#if (_WIN32_IE >= 0x0600)
	DWORD GetExtendedStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, RB_GETEXTENDEDSTYLE, 0, 0L);
	}

	DWORD SetExtendedStyle(DWORD dwStyle, DWORD dwMask)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, RB_SETEXTENDEDSTYLE, dwMask, dwStyle);
	}
#endif // (_WIN32_IE >= 0x0600)

// Operations
	BOOL InsertBand(int nBand, LPREBARBANDINFO lprbbi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_INSERTBAND, nBand, (LPARAM)lprbbi);
	}

	BOOL AddBand(LPREBARBANDINFO lprbbi)
	{
		return InsertBand(-1, lprbbi);
	}

	BOOL DeleteBand(int nBand)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_DELETEBAND, nBand, 0L);
	}

	ATL::CWindow SetNotifyWnd(HWND hWnd)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ATL::CWindow((HWND)::SendMessage(m_hWnd, RB_SETPARENT, (WPARAM)hWnd, 0L));
	}

#if (_WIN32_IE >= 0x0400)
	void BeginDrag(int nBand, DWORD dwPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_BEGINDRAG, nBand, dwPos);
	}

	void BeginDrag(int nBand, int xPos, int yPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_BEGINDRAG, nBand, MAKELPARAM(xPos, yPos));
	}

	void EndDrag()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_ENDDRAG, 0, 0L);
	}

	void DragMove(DWORD dwPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_DRAGMOVE, 0, dwPos);
	}

	void DragMove(int xPos, int yPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_DRAGMOVE, 0, MAKELPARAM(xPos, yPos));
	}

#ifndef _WIN32_WCE
	void GetDropTarget(IDropTarget** ppDropTarget) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_GETDROPTARGET, 0, (LPARAM)ppDropTarget);
	}
#endif // !_WIN32_WCE

	void MaximizeBand(int nBand, BOOL bIdeal = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_MAXIMIZEBAND, nBand, bIdeal);
	}

	void MinimizeBand(int nBand)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_MINIMIZEBAND, nBand, 0L);
	}

	BOOL SizeToRect(LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_SIZETORECT, 0, (LPARAM)lpRect);
	}

	int IdToIndex(UINT uBandID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, RB_IDTOINDEX, uBandID, 0L);
	}

	int HitTest(LPRBHITTESTINFO lprbht) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, RB_HITTEST, 0, (LPARAM)lprbht);
	}

	BOOL ShowBand(int nBand, BOOL bShow)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_SHOWBAND, nBand, bShow);
	}

#ifndef _WIN32_WCE
	BOOL MoveBand(int nBand, int nNewPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nNewPos >= 0 && nNewPos <= ((int)GetBandCount() - 1));
		return (BOOL)::SendMessage(m_hWnd, RB_MOVEBAND, nBand, nNewPos);
	}
#endif // !_WIN32_WCE
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)
	void PushChevron(int nBand, LPARAM lAppValue)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, RB_PUSHCHEVRON, nBand, lAppValue);
	}
#endif // (_WIN32_IE >= 0x0500) && !defined(_WIN32_WCE)

// Extra operations
#if (_WIN32_IE >= 0x0400)
	void LockBands(bool bLock)
	{
		int nBandCount = GetBandCount();
		for(int i =0; i < nBandCount; i++)
		{
			REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO() };
			rbbi.fMask = RBBIM_STYLE;
			BOOL bRet = GetBandInfo(i, &rbbi);
			ATLASSERT(bRet);

			if((rbbi.fStyle & RBBS_GRIPPERALWAYS) == 0)
			{
				rbbi.fStyle |= RBBS_GRIPPERALWAYS;
				bRet = SetBandInfo(i, &rbbi);
				ATLASSERT(bRet);
				rbbi.fStyle &= ~RBBS_GRIPPERALWAYS;
			}

			if(bLock)
				rbbi.fStyle |= RBBS_NOGRIPPER;
			else
				rbbi.fStyle &= ~RBBS_NOGRIPPER;

			bRet = SetBandInfo(i, &rbbi);
			ATLASSERT(bRet);
		}
	}
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_WINNT >= 0x0600)
	BOOL SetBandWidth(int nBand, int cxWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, RB_SETBANDWIDTH, nBand, cxWidth);
	}
#endif // (_WIN32_WINNT >= 0x0600)
};

typedef CReBarCtrlT<ATL::CWindow>   CReBarCtrl;


///////////////////////////////////////////////////////////////////////////////
// CComboBoxEx

#ifndef _WIN32_WCE

template <class TBase>
class CComboBoxExT : public CComboBoxT< TBase >
{
public:
// Constructors
	CComboBoxExT(HWND hWnd = NULL) : CComboBoxT< TBase >(hWnd)
	{ }

	CComboBoxExT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_COMBOBOXEX;
	}

	CImageList GetImageList() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, CBEM_GETIMAGELIST, 0, 0L));
	}

	CImageList SetImageList(HIMAGELIST hImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CImageList((HIMAGELIST)::SendMessage(m_hWnd, CBEM_SETIMAGELIST, 0, (LPARAM)hImageList));
	}

#if (_WIN32_IE >= 0x0400)
	DWORD GetExtendedStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, CBEM_GETEXTENDEDSTYLE, 0, 0L);
	}

	DWORD SetExtendedStyle(DWORD dwExMask, DWORD dwExStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, CBEM_SETEXTENDEDSTYLE, dwExMask, dwExStyle);
	}

	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CBEM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CBEM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_WINNT >= 0x0501)
	void SetWindowTheme(LPCWSTR lpstrTheme)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, CBEM_SETWINDOWTHEME, 0, (LPARAM)lpstrTheme);
	}
#endif // (_WIN32_WINNT >= 0x0501)

// Operations
	int InsertItem(const COMBOBOXEXITEM* lpcCBItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CBEM_INSERTITEM, 0, (LPARAM)lpcCBItem);
	}

	int InsertItem(UINT nMask, int nIndex, LPCTSTR lpszItem, int nImage, int nSelImage, 
	               int iIndent, int iOverlay, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		COMBOBOXEXITEM cbex = { 0 };
		cbex.mask = nMask;
		cbex.iItem = nIndex;
		cbex.pszText = (LPTSTR) lpszItem;
		cbex.iImage = nImage;
		cbex.iSelectedImage = nSelImage;
		cbex.iIndent = iIndent;
		cbex.iOverlay = iOverlay;
		cbex.lParam = lParam;
		return (int)::SendMessage(m_hWnd, CBEM_INSERTITEM, 0, (LPARAM)&cbex);
	}

	int InsertItem(int nIndex, LPCTSTR lpszItem, int nImage, int nSelImage, int iIndent, LPARAM lParam = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		COMBOBOXEXITEM cbex = { 0 };
		cbex.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT | CBEIF_LPARAM;
		cbex.iItem = nIndex;
		cbex.pszText = (LPTSTR) lpszItem;
		cbex.iImage = nImage;
		cbex.iSelectedImage = nSelImage;
		cbex.iIndent = iIndent;
		cbex.lParam = lParam;
		return (int)::SendMessage(m_hWnd, CBEM_INSERTITEM, 0, (LPARAM)&cbex);
	}

	int AddItem(UINT nMask, LPCTSTR lpszItem, int nImage, int nSelImage, int iIndent, int iOverlay, LPARAM lParam)
	{
		return InsertItem(nMask, -1, lpszItem, nImage, nSelImage, iIndent, iOverlay, lParam);
	}

	int AddItem(LPCTSTR lpszItem, int nImage, int nSelImage, int iIndent, LPARAM lParam = 0)
	{
		return InsertItem(-1, lpszItem, nImage, nSelImage, iIndent, lParam);
	}

	int DeleteItem(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, CBEM_DELETEITEM, nIndex, 0L);
	}

	BOOL GetItem(PCOMBOBOXEXITEM pCBItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CBEM_GETITEM, 0, (LPARAM)pCBItem);
	}

	BOOL SetItem(const COMBOBOXEXITEM* lpcCBItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CBEM_SETITEM, 0, (LPARAM)lpcCBItem);
	}

	int SetItem(int nIndex, UINT nMask, LPCTSTR lpszItem, int nImage, int nSelImage, 
	            int iIndent, int iOverlay, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		COMBOBOXEXITEM cbex = { 0 };
		cbex.mask = nMask;
		cbex.iItem = nIndex;
		cbex.pszText = (LPTSTR) lpszItem;
		cbex.iImage = nImage;
		cbex.iSelectedImage = nSelImage;
		cbex.iIndent = iIndent;
		cbex.iOverlay = iOverlay;
		cbex.lParam = lParam;
		return (int)::SendMessage(m_hWnd, CBEM_SETITEM, 0, (LPARAM)&cbex);
	}

	BOOL GetItemText(int nIndex, LPTSTR lpszItem, int nLen) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(lpszItem != NULL);

		COMBOBOXEXITEM cbex = { 0 };
		cbex.mask = CBEIF_TEXT;
		cbex.iItem = nIndex;
		cbex.pszText = lpszItem;
		cbex.cchTextMax = nLen;

		return (BOOL)::SendMessage(m_hWnd, CBEM_GETITEM, 0, (LPARAM)&cbex);
	}

#ifndef _ATL_NO_COM
	BOOL GetItemText(int nIndex, BSTR& bstrText) const
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(bstrText == NULL);

		COMBOBOXEXITEM cbex = { 0 };
		cbex.mask = CBEIF_TEXT;
		cbex.iItem = nIndex;

		LPTSTR lpstrText = NULL;
		BOOL bRet = FALSE;
		for(int nLen = 256; ; nLen *= 2)
		{
			ATLTRY(lpstrText = new TCHAR[nLen]);
			if(lpstrText == NULL)
				break;
			lpstrText[0] = NULL;
			cbex.pszText = lpstrText;
			cbex.cchTextMax = nLen;
			bRet = (BOOL)::SendMessage(m_hWnd, CBEM_GETITEM, 0, (LPARAM)&cbex);
			if(!bRet || (lstrlen(cbex.pszText) < nLen - 1))
				break;
			delete [] lpstrText;
			lpstrText = NULL;
		}

		if(lpstrText != NULL)
		{
			if(bRet)
				bstrText = ::SysAllocString(T2OLE(lpstrText));
			delete [] lpstrText;
		}

		return (bstrText != NULL) ? TRUE : FALSE;
	}
#endif // !_ATL_NO_COM

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	BOOL GetItemText(int nIndex, _CSTRING_NS::CAtlString& strText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));

		COMBOBOXEXITEM cbex = { 0 };
		cbex.mask = CBEIF_TEXT;
		cbex.iItem = nIndex;

		strText.Empty();
		BOOL bRet = FALSE;
		for(int nLen = 256; ; nLen *= 2)
		{
			cbex.pszText = strText.GetBufferSetLength(nLen);
			if(cbex.pszText == NULL)
			{
				bRet = FALSE;
				break;
			}
			cbex.cchTextMax = nLen;
			bRet = (BOOL)::SendMessage(m_hWnd, CBEM_GETITEM, 0, (LPARAM)&cbex);
			if(!bRet || (lstrlen(cbex.pszText) < nLen - 1))
				break;
		}
		strText.ReleaseBuffer();
		return bRet;
	}
#endif // defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

	BOOL SetItemText(int nIndex, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return SetItem(nIndex, CBEIF_TEXT, lpszItem, 0, 0, 0, 0, 0);
	}

	CComboBox GetComboCtrl() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CComboBox((HWND)::SendMessage(m_hWnd, CBEM_GETCOMBOCONTROL, 0, 0L));
	}

	CEdit GetEditCtrl() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CEdit((HWND)::SendMessage(m_hWnd, CBEM_GETEDITCONTROL, 0, 0L));
	}

	BOOL HasEditChanged() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, CBEM_HASEDITCHANGED, 0, 0L);
	}

// Non-functional
	int AddString(LPCTSTR /*lpszItem*/)
	{
		ATLASSERT(FALSE);  // Not available in CComboBoxEx; use InsertItem
		return 0;
	}

	int InsertString(int /*nIndex*/, LPCTSTR /*lpszString*/)
	{
		ATLASSERT(FALSE);  // Not available in CComboBoxEx; use InsertItem
		return 0;
	}

	int Dir(UINT /*attr*/, LPCTSTR /*lpszWildCard*/)
	{
		ATLASSERT(FALSE);  // Not available in CComboBoxEx
		return 0;
	}

	int FindString(int /*nStartAfter*/, LPCTSTR /*lpszString*/) const
	{
		ATLASSERT(FALSE);  // Not available in CComboBoxEx; try FindStringExact
		return 0;
	}
};

typedef CComboBoxExT<ATL::CWindow>   CComboBoxEx;

#endif // !_WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// CMonthCalendarCtrl

template <class TBase>
class CMonthCalendarCtrlT : public TBase
{
public:
// Constructors
	CMonthCalendarCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CMonthCalendarCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return MONTHCAL_CLASS;
	}

	COLORREF GetColor(int nColorType) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, MCM_GETCOLOR, nColorType, 0L);
	}

	COLORREF SetColor(int nColorType, COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, MCM_SETCOLOR, nColorType, clr);
	}

	BOOL GetCurSel(LPSYSTEMTIME lpSysTime) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_GETCURSEL, 0, (LPARAM)lpSysTime);
	}

	BOOL SetCurSel(LPSYSTEMTIME lpSysTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETCURSEL, 0, (LPARAM)lpSysTime);
	}

	int GetFirstDayOfWeek(BOOL* pbLocaleVal = NULL) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, MCM_GETFIRSTDAYOFWEEK, 0, 0L);
		if(pbLocaleVal != NULL)
			*pbLocaleVal = (BOOL)HIWORD(dwRet);
		return (int)(short)LOWORD(dwRet);
	}

	int SetFirstDayOfWeek(int nDay, BOOL* pbLocaleVal = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwRet = (DWORD)::SendMessage(m_hWnd, MCM_SETFIRSTDAYOFWEEK, 0, nDay);
		if(pbLocaleVal != NULL)
			*pbLocaleVal = (BOOL)HIWORD(dwRet);
		return (int)(short)LOWORD(dwRet);
	}

	int GetMaxSelCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, MCM_GETMAXSELCOUNT, 0, 0L);
	}

	BOOL SetMaxSelCount(int nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETMAXSELCOUNT, nMax, 0L);
	}

	int GetMonthDelta() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, MCM_GETMONTHDELTA, 0, 0L);
	}

	int SetMonthDelta(int nDelta)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, MCM_SETMONTHDELTA, nDelta, 0L);
	}

	DWORD GetRange(LPSYSTEMTIME lprgSysTimeArray) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, MCM_GETRANGE, 0, (LPARAM)lprgSysTimeArray);
	}

	BOOL SetRange(DWORD dwFlags, LPSYSTEMTIME lprgSysTimeArray)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETRANGE, dwFlags, (LPARAM)lprgSysTimeArray);
	}

	BOOL GetSelRange(LPSYSTEMTIME lprgSysTimeArray) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_GETSELRANGE, 0, (LPARAM)lprgSysTimeArray);
	}

	BOOL SetSelRange(LPSYSTEMTIME lprgSysTimeArray)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETSELRANGE, 0, (LPARAM)lprgSysTimeArray);
	}

	BOOL GetToday(LPSYSTEMTIME lpSysTime) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_GETTODAY, 0, (LPARAM)lpSysTime);
	}

	void SetToday(LPSYSTEMTIME lpSysTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, MCM_SETTODAY, 0, (LPARAM)lpSysTime);
	}

	BOOL GetMinReqRect(LPRECT lpRectInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_GETMINREQRECT, 0, (LPARAM)lpRectInfo);
	}

	int GetMaxTodayWidth() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, MCM_GETMAXTODAYWIDTH, 0, 0L);
	}

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
	BOOL GetUnicodeFormat() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_GETUNICODEFORMAT, 0, 0L);
	}

	BOOL SetUnicodeFormat(BOOL bUnicode = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETUNICODEFORMAT, bUnicode, 0L);
	}
#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

#if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
	DWORD GetCurrentView() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, MCM_GETCURRENTVIEW, 0, 0L);
	}

	BOOL SetCurrentView(DWORD dwView)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETCURRENTVIEW, 0, dwView);
	}

	DWORD GetCalendarCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, MCM_GETCALENDARCOUNT, 0, 0L);
	}

	BOOL GetCalendarGridInfo(PMCGRIDINFO pGridInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_GETCALENDARGRIDINFO, 0, (LPARAM)pGridInfo);
	}

	CALID GetCALID() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (CALID)::SendMessage(m_hWnd, MCM_GETCALID, 0, 0L);
	}

	void SetCALID(CALID calid)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, MCM_SETCALID, (LPARAM)calid, 0L);
	}

	int GetCalendarBorder() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, MCM_GETCALENDARBORDER, 0, 0L);
	}

	void SetCalendarBorder(int cxyBorder, BOOL bSet = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, MCM_SETCALENDARBORDER, (WPARAM)bSet, (LPARAM)cxyBorder);
	}
#endif // defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)

// Operations
	int GetMonthRange(DWORD dwFlags, LPSYSTEMTIME lprgSysTimeArray) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, MCM_GETMONTHRANGE, dwFlags, (LPARAM)lprgSysTimeArray);
	}

	BOOL SetDayState(int nMonths, LPMONTHDAYSTATE lpDayStateArray)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, MCM_SETDAYSTATE, nMonths, (LPARAM)lpDayStateArray);
	}

	DWORD HitTest(PMCHITTESTINFO pMCHitTest) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, MCM_HITTEST, 0, (LPARAM)pMCHitTest);
	}

#if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
	void SizeRectToMin(LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, MCM_SIZERECTTOMIN, 0, (LPARAM)lpRect);
	}
#endif // defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
};

typedef CMonthCalendarCtrlT<ATL::CWindow>   CMonthCalendarCtrl;


///////////////////////////////////////////////////////////////////////////////
// CDateTimePickerCtrl

template <class TBase>
class CDateTimePickerCtrlT : public TBase
{
public:
// Constructors
	CDateTimePickerCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CDateTimePickerCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Operations
	static LPCTSTR GetWndClassName()
	{
		return DATETIMEPICK_CLASS;
	}

	BOOL SetFormat(LPCTSTR lpszFormat)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DTM_SETFORMAT, 0, (LPARAM)lpszFormat);
	}

	COLORREF GetMonthCalColor(int nColorType) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, DTM_GETMCCOLOR, nColorType, 0L);
	}

	COLORREF SetMonthCalColor(int nColorType, COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, DTM_SETMCCOLOR, nColorType, clr);
	}

	DWORD GetRange(LPSYSTEMTIME lpSysTimeArray) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DTM_GETRANGE, 0, (LPARAM)lpSysTimeArray);
	}

	BOOL SetRange(DWORD dwFlags, LPSYSTEMTIME lpSysTimeArray)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DTM_SETRANGE, dwFlags, (LPARAM)lpSysTimeArray);
	}

	DWORD GetSystemTime(LPSYSTEMTIME lpSysTime) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)lpSysTime);
	}

	BOOL SetSystemTime(DWORD dwFlags, LPSYSTEMTIME lpSysTime)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DTM_SETSYSTEMTIME, dwFlags, (LPARAM)lpSysTime);
	}

	CMonthCalendarCtrl GetMonthCal() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CMonthCalendarCtrl((HWND)::SendMessage(m_hWnd, DTM_GETMONTHCAL, 0, 0L));
	}

#if (_WIN32_IE >= 0x0400)
	CFontHandle GetMonthCalFont() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CFontHandle((HFONT)::SendMessage(m_hWnd, DTM_GETMCFONT, 0, 0L));
	}

	void SetMonthCalFont(HFONT hFont, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DTM_SETMCFONT, (WPARAM)hFont, MAKELPARAM(bRedraw, 0));
	}
#endif // (_WIN32_IE >= 0x0400)

#if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
	DWORD GetMonthCalStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DTM_GETMCSTYLE, 0, 0L);
	}

	DWORD SetMonthCalStyle(DWORD dwStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DTM_SETMCSTYLE, 0, (LPARAM)dwStyle);
	}

	void GetDateTimePickerInfo(LPDATETIMEPICKERINFO lpPickerInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DTM_GETDATETIMEPICKERINFO, 0, (LPARAM)lpPickerInfo);
	}

	BOOL GetIdealSize(LPSIZE lpSize) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DTM_GETIDEALSIZE, 0, (LPARAM)lpSize);
	}

	void CloseMonthCal()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DTM_CLOSEMONTHCAL, 0, 0L);
	}
#endif // defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
};

typedef CDateTimePickerCtrlT<ATL::CWindow>   CDateTimePickerCtrl;


///////////////////////////////////////////////////////////////////////////////
// CFlatScrollBarImpl - support for flat scroll bars

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

template <class T>
class CFlatScrollBarImpl
{
public:
// Initialization
	BOOL FlatSB_Initialize()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::InitializeFlatSB(pT->m_hWnd);
	}

	HRESULT FlatSB_Uninitialize()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::UninitializeFlatSB(pT->m_hWnd);
	}

// Flat scroll bar properties
	BOOL FlatSB_GetScrollProp(UINT uIndex, LPINT lpnValue) const
	{
		const T* pT = static_cast<const T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_GetScrollProp(pT->m_hWnd, uIndex, lpnValue);
	}

	BOOL FlatSB_SetScrollProp(UINT uIndex, int nValue, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_SetScrollProp(pT->m_hWnd, uIndex, nValue, bRedraw);
	}

// Attributes
	int FlatSB_GetScrollPos(int nBar) const
	{
		const T* pT = static_cast<const T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_GetScrollPos(pT->m_hWnd, nBar);
	}

	int FlatSB_SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_SetScrollPos(pT->m_hWnd, nBar, nPos, bRedraw);
	}

	BOOL FlatSB_GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const
	{
		const T* pT = static_cast<const T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_GetScrollRange(pT->m_hWnd, nBar, lpMinPos, lpMaxPos);
	}

	BOOL FlatSB_SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_SetScrollRange(pT->m_hWnd, nBar, nMinPos, nMaxPos, bRedraw);
	}

	BOOL FlatSB_GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo) const
	{
		const T* pT = static_cast<const T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_GetScrollInfo(pT->m_hWnd, nBar, lpScrollInfo);
	}

	int FlatSB_SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_SetScrollInfo(pT->m_hWnd, nBar, lpScrollInfo, bRedraw);
	}

// Operations
	BOOL FlatSB_ShowScrollBar(UINT nBar, BOOL bShow = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_ShowScrollBar(pT->m_hWnd, nBar, bShow);
	}

	BOOL FlatSB_EnableScrollBar(UINT uSBFlags, UINT uArrowFlags = ESB_ENABLE_BOTH)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		return ::FlatSB_EnableScrollBar(pT->m_hWnd, uSBFlags, uArrowFlags);
	}
};

template <class TBase>
class CFlatScrollBarT : public TBase, public CFlatScrollBarImpl<CFlatScrollBarT< TBase > >
{
public:
	CFlatScrollBarT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CFlatScrollBarT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}
};

typedef CFlatScrollBarT<ATL::CWindow>   CFlatScrollBar;

#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)


///////////////////////////////////////////////////////////////////////////////
// CIPAddressCtrl

#if (_WIN32_IE >= 0x0400)

template <class TBase>
class CIPAddressCtrlT : public TBase
{
public:
// Constructors
	CIPAddressCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CIPAddressCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Atteributes
	static LPCTSTR GetWndClassName()
	{
		return WC_IPADDRESS;
	}

	BOOL IsBlank() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, IPM_ISBLANK, 0, 0L);
	}

	int GetAddress(LPDWORD lpdwAddress) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, IPM_GETADDRESS, 0, (LPARAM)lpdwAddress);
	}

	void SetAddress(DWORD dwAddress)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, IPM_SETADDRESS, 0, dwAddress);
	}

	void ClearAddress()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, IPM_CLEARADDRESS, 0, 0L);
	}

	void SetRange(int nField, WORD wRange)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, IPM_SETRANGE, nField, wRange);
	}

	void SetRange(int nField, BYTE nMin, BYTE nMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, IPM_SETRANGE, nField, MAKEIPRANGE(nMin, nMax));
	}

	void SetFocus(int nField)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, IPM_SETFOCUS, nField, 0L);
	}
};

typedef CIPAddressCtrlT<ATL::CWindow>   CIPAddressCtrl;

#endif // (_WIN32_IE >= 0x0400)


///////////////////////////////////////////////////////////////////////////////
// CPagerCtrl

#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)

template <class TBase>
class CPagerCtrlT : public TBase
{
public:
// Constructors
	CPagerCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CPagerCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_PAGESCROLLER;
	}

	int GetButtonSize() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PGM_GETBUTTONSIZE, 0, 0L);
	}

	int SetButtonSize(int nButtonSize)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PGM_SETBUTTONSIZE, 0, nButtonSize);
	}

	DWORD GetButtonState(int nButton) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(nButton == PGB_TOPORLEFT || nButton == PGB_BOTTOMORRIGHT);
		return (DWORD)::SendMessage(m_hWnd, PGM_GETBUTTONSTATE, 0, nButton);
	}

	COLORREF GetBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PGM_GETBKCOLOR, 0, 0L);
	}

	COLORREF SetBkColor(COLORREF clrBk)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, PGM_SETBKCOLOR, 0, (LPARAM)clrBk);
	}

	int GetBorder() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PGM_GETBORDER, 0, 0L);
	}

	int SetBorder(int nBorderSize)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PGM_SETBORDER, 0, nBorderSize);
	}

	int GetPos() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PGM_GETPOS, 0, 0L);
	}

	int SetPos(int nPos)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PGM_SETPOS, 0, nPos);
	}

// Operations
	void SetChild(HWND hWndChild)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PGM_SETCHILD, 0, (LPARAM)hWndChild);
	}

	void ForwardMouse(BOOL bForward = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PGM_FORWARDMOUSE, bForward, 0L);
	}

	void RecalcSize()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PGM_RECALCSIZE, 0, 0L);
	}

	void GetDropTarget(IDropTarget** ppDropTarget)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(ppDropTarget != NULL);
		::SendMessage(m_hWnd, PGM_GETDROPTARGET, 0, (LPARAM)ppDropTarget);
	}
};

typedef CPagerCtrlT<ATL::CWindow>   CPagerCtrl;

#endif // (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)


///////////////////////////////////////////////////////////////////////////////
// CLinkCtrl - Windows SYSLINK control

#if (_WIN32_WINNT >= 0x0501) && !defined(_WIN32_WCE)

template <class TBase>
class CLinkCtrlT : public TBase
{
public:
// Constructors
	CLinkCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CLinkCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
#ifdef _UNICODE
		return WC_LINK;
#else // !_UNICODE
		return "SysLink";
#endif // !_UNICODE
	}

	int GetIdealHeight(int cxMaxWidth = 0) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LM_GETIDEALHEIGHT, cxMaxWidth, 0L);
	}

	BOOL GetItem(PLITEM pLItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LM_GETITEM, 0, (LPARAM)pLItem);
	}

	BOOL SetItem(PLITEM pLItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LM_SETITEM, 0, (LPARAM)pLItem);
	}

	// Vista only
	int GetIdealSize(SIZE& size, int cxMaxWidth = 0) const
	{
#ifndef LM_GETIDEALSIZE
		const UINT LM_GETIDEALSIZE = LM_GETIDEALHEIGHT;
#endif
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, LM_GETIDEALSIZE, cxMaxWidth, (LPARAM)&size);
	}

// Operations
	BOOL HitTest(PLHITTESTINFO pLHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, LM_HITTEST, 0, (LPARAM)pLHitTestInfo);
	}
};

typedef CLinkCtrlT<ATL::CWindow>   CLinkCtrl;

#endif // (_WIN32_WINNT >= 0x0501) && !defined(_WIN32_WCE)


///////////////////////////////////////////////////////////////////////////////
// CCustomDraw - MI class for custom-draw support

template <class T>
class CCustomDraw
{
public:
#if (_ATL_VER < 0x0700)
	BOOL m_bHandledCD;

	BOOL IsMsgHandled() const
	{
		return m_bHandledCD;
	}

	void SetMsgHandled(BOOL bHandled)
	{
		m_bHandledCD = bHandled;
	}
#endif // !(_ATL_VER < 0x0700)

// Message map and handlers
	BEGIN_MSG_MAP(CCustomDraw< T >)
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
	ALT_MSG_MAP(1)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
	END_MSG_MAP()

// message handler
	LRESULT OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->SetMsgHandled(TRUE);
		LPNMCUSTOMDRAW lpNMCustomDraw = (LPNMCUSTOMDRAW)pnmh;
		DWORD dwRet = 0;
		switch(lpNMCustomDraw->dwDrawStage)
		{
		case CDDS_PREPAINT:
			dwRet = pT->OnPrePaint(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_POSTPAINT:
			dwRet = pT->OnPostPaint(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_PREERASE:
			dwRet = pT->OnPreErase(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_POSTERASE:
			dwRet = pT->OnPostErase(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_ITEMPREPAINT:
			dwRet = pT->OnItemPrePaint(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_ITEMPOSTPAINT:
			dwRet = pT->OnItemPostPaint(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_ITEMPREERASE:
			dwRet = pT->OnItemPreErase(idCtrl, lpNMCustomDraw);
			break;
		case CDDS_ITEMPOSTERASE:
			dwRet = pT->OnItemPostErase(idCtrl, lpNMCustomDraw);
			break;
#if (_WIN32_IE >= 0x0400)
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
			dwRet = pT->OnSubItemPrePaint(idCtrl, lpNMCustomDraw);
			break;
#endif // (_WIN32_IE >= 0x0400)
		default:
			pT->SetMsgHandled(FALSE);
			break;
		}
		bHandled = pT->IsMsgHandled();
		return dwRet;
	}

// Overrideables
	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnPostPaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnPreErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnPostErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnItemPostPaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnItemPreErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

	DWORD OnItemPostErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}

#if (_WIN32_IE >= 0x0400)
	DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
	{
		return CDRF_DODEFAULT;
	}
#endif // (_WIN32_IE >= 0x0400)
};


// --- Windows CE common controls ---

#ifdef _WIN32_WCE

///////////////////////////////////////////////////////////////////////////////
// CCECommandBarCtrl

template <class TBase>
class CCECommandBarCtrlT : public TBase
{
public:
// Constructors
	CCECommandBarCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CCECommandBarCtrlT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

// Attributes
	BOOL IsVisible() const
	{
		return IsWindowVisible();
	}

	int GetHeight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_Height(m_hWnd);
	}

	HMENU GetMenu(WORD wButton) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_GetMenu(m_hWnd, wButton);
	}

// Operations
	HWND Create(HWND hWndParent, int nCmdBarID)
	{
		m_hWnd = ::CommandBar_Create(ModuleHelper::GetModuleInstance(), hWndParent, nCmdBarID);
		ATLASSERT(::IsWindow(m_hWnd));
		return m_hWnd;
	}

	void Destroy()
	{
		DestroyWindow();
	}

	BOOL Show(BOOL bShow = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_Show(m_hWnd, bShow);
	}

	BOOL DrawMenuBar(WORD wButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_DrawMenuBar(m_hWnd, wButton);
	}

	BOOL AddAdornments(DWORD dwFlags = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_AddAdornments(m_hWnd, dwFlags, 0);
	}

	int AddBitmap(int nBitmapID, int nNumImages)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_AddBitmap(m_hWnd, ModuleHelper::GetResourceInstance(), nBitmapID, nNumImages, 16, 16);
	}

	BOOL AddButtons(UINT uNumButtons, LPTBBUTTON lpButtons)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CommandBar_AddButtons(m_hWnd, uNumButtons, lpButtons);
	}

	BOOL AddToolTips(UINT uNumToolTips, LPTSTR lpToolTips)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CommandBar_AddToolTips(m_hWnd, uNumToolTips, lpToolTips);
	}

	BOOL InsertButton(int nButton, LPTBBUTTON lpButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CommandBar_InsertButton(m_hWnd, nButton, lpButton);
	}

	HWND InsertComboBox(int nWidth, UINT dwStyle, WORD wComboBoxID, WORD wButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_InsertComboBox(m_hWnd, ModuleHelper::GetModuleInstance(), nWidth, dwStyle, wComboBoxID, wButton);
	}

	BOOL InsertMenubar(WORD wMenuID, WORD wButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_InsertMenubar(m_hWnd, ModuleHelper::GetResourceInstance(), wMenuID, wButton);
	}

	BOOL InsertMenubarEx(ATL::_U_STRINGorID menu, WORD wButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBar_InsertMenubarEx(m_hWnd, ModuleHelper::GetResourceInstance(), (LPTSTR)menu.m_lpstr, wButton);
	}

	BOOL IsCommandBarMessage(LPMSG lpMsg)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsCommandBarMessage(m_hWnd, lpMsg);
	}
};

#if defined(_AYGSHELL_H_) || defined(__AYGSHELL_H__) // PPC MenuBar
	typedef CCECommandBarCtrlT<CToolBarCtrl>	CMenuBarCtrl;
#else
	typedef CCECommandBarCtrlT<CToolBarCtrl>	CCECommandBarCtrl;
#endif // defined(_AYGSHELL_H_) || defined(__AYGSHELL_H__)

///////////////////////////////////////////////////////////////////////////////
// CCECommandBandsCtrl

template <class TBase>
class CCECommandBandsCtrlT : public TBase
{
public:
// Constructors
	CCECommandBandsCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CCECommandBandsCtrlT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

// Attributes
	BOOL IsVisible() const
	{
		return IsWindowVisible();
	}

#if (_WIN32_IE >= 0x0400)
	UINT GetHeight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CommandBands_Height(m_hWnd);
	}
#endif // (_WIN32_IE >= 0x0400)

	HWND GetCommandBar(UINT uBand) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBands_GetCommandBar(m_hWnd, uBand);
	}

	BOOL GetRestoreInformation(UINT uBand, LPCOMMANDBANDSRESTOREINFO pcbr) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBands_GetRestoreInformation(m_hWnd, uBand, pcbr);
	}

// Operations
	HWND Create(HWND hWndParent, UINT wID, DWORD dwStyles, HIMAGELIST hImageList = NULL)
	{
		m_hWnd = ::CommandBands_Create(ModuleHelper::GetModuleInstance(), hWndParent, wID, dwStyles, hImageList);
		ATLASSERT(::IsWindow(m_hWnd));
		return m_hWnd;
	}

	BOOL AddAdornments(DWORD dwFlags = 0, LPREBARBANDINFO prbbi = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBands_AddAdornments(m_hWnd, ModuleHelper::GetModuleInstance(), dwFlags, prbbi);
	}

	BOOL AddBands(UINT uBandCount, LPREBARBANDINFO prbbi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBands_AddBands(m_hWnd, ModuleHelper::GetModuleInstance(), uBandCount, prbbi);
	}

	BOOL Show(BOOL bShow = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CommandBands_Show(m_hWnd, bShow);
	}
};

typedef CCECommandBandsCtrlT<ATL::CWindow>	CCECommandBandsCtrl;

#endif // _WIN32_WCE

}; // namespace WTL

#endif // __ATLCTRLS_H__
