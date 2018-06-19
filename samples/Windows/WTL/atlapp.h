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

#ifndef __ATLAPP_H__
#define __ATLAPP_H__

#pragma once

#ifndef __cplusplus
	#error WTL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlapp.h requires atlbase.h to be included first
#endif

#ifndef _WIN32_WCE
  #if (WINVER < 0x0400)
	#error WTL requires Windows version 4.0 or higher
  #endif

  #if (_WIN32_IE < 0x0300)
	#error WTL requires IE version 3.0 or higher
  #endif
#endif

#ifdef _ATL_NO_COMMODULE
	#error WTL requires that _ATL_NO_COMMODULE is not defined
#endif

#if (_ATL_VER >= 0x0900) && defined(_ATL_MIN_CRT)
	#error _ATL_MIN_CRT is not supported with ATL 9.0 and higher
#endif

#if defined(_WIN32_WCE) && defined(_ATL_MIN_CRT)
	#pragma message("Warning: WTL for Windows CE doesn't use _ATL_MIN_CRT")
#endif

#include <limits.h>
#if !defined(_ATL_MIN_CRT) && defined(_MT) && !defined(_WIN32_WCE)
  #include <process.h>	// for _beginthreadex
#endif

#if (_ATL_VER < 0x0800) && !defined(_DEBUG)
  #include <stdio.h>
#endif

#include <commctrl.h>
#ifndef _WIN32_WCE
  #pragma comment(lib, "comctl32.lib")
#endif

#ifndef _WIN32_WCE
  #include "atlres.h"
#else // CE specific
  #include "atlresce.h"
#endif // _WIN32_WCE

// We need to disable this warning because of template class arguments
#pragma warning(disable: 4127)

#if (_ATL_VER >= 0x0900) && !defined(_SECURE_ATL)
  #define _SECURE_ATL	1
#endif


///////////////////////////////////////////////////////////////////////////////
// WTL version number

#define _WTL_VER	0x0810


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CMessageFilter
// CIdleHandler
// CMessageLoop
//
// CAppModule
// CServerAppModule
//
// CRegKeyEx
//
// Global functions:
//   AtlGetDefaultGuiFont()
//   AtlCreateControlFont()
//   AtlCreateBoldFont()
//   AtlInitCommonControls()


///////////////////////////////////////////////////////////////////////////////
// Global support for Windows CE

#ifdef _WIN32_WCE

#ifndef SW_SHOWDEFAULT
  #define SW_SHOWDEFAULT	SW_SHOWNORMAL
#endif // !SW_SHOWDEFAULT

// These get's OR-ed in a constant and will have no effect.
// Defining them reduces the number of #ifdefs required for CE.
#define LR_DEFAULTSIZE      0
#define LR_LOADFROMFILE     0

#ifndef SM_CXCURSOR
  #define SM_CXCURSOR             13
#endif
#ifndef SM_CYCURSOR
  #define SM_CYCURSOR             14
#endif

inline BOOL IsMenu(HMENU hMenu)
{
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	::SetLastError(0);
	BOOL bRet = ::GetMenuItemInfo(hMenu, 0, TRUE, &mii);
	if(!bRet)
		bRet = (::GetLastError() != ERROR_INVALID_MENU_HANDLE) ? TRUE : FALSE;
	return bRet;
}

#if (_WIN32_WCE >= 410)
extern "C" void WINAPI ListView_SetItemSpacing(HWND hwndLV, int iHeight);
#endif // (_WIN32_WCE >= 410)

inline int MulDiv(IN int nNumber, IN int nNumerator, IN int nDenominator)
{
	__int64 multiple = nNumber * nNumerator;
	return static_cast<int>(multiple / nDenominator);
}

#if (_ATL_VER >= 0x0800)

#ifndef _WTL_KEEP_WS_OVERLAPPEDWINDOW
  #ifdef WS_OVERLAPPEDWINDOW
    #undef WS_OVERLAPPEDWINDOW
    #define WS_OVERLAPPEDWINDOW	0
  #endif // WS_OVERLAPPEDWINDOW
#endif // !_WTL_KEEP_WS_OVERLAPPEDWINDOW

#ifndef RDW_FRAME
  #define RDW_FRAME	0
#endif // !RDW_FRAME

#ifndef WM_WINDOWPOSCHANGING
  #define WM_WINDOWPOSCHANGING	0
#endif // !WM_WINDOWPOSCHANGING

#define FreeResource(x)
#define UnlockResource(x)

namespace ATL
{
  inline HRESULT CComModule::RegisterClassObjects(DWORD /*dwClsContext*/, DWORD /*dwFlags*/) throw()
  { return E_NOTIMPL; }
  inline HRESULT CComModule::RevokeClassObjects() throw()
  { return E_NOTIMPL; }
}; // namespace ATL

#ifndef lstrlenW
  #define lstrlenW	(int)ATL::lstrlenW
#endif // lstrlenW

inline int WINAPI lstrlenA(LPCSTR lpszString)
{ return ATL::lstrlenA(lpszString); }

#ifdef lstrcpyn
  #undef lstrcpyn
  #define lstrcpyn	ATL::lstrcpynW
#endif // lstrcpyn

#ifndef SetWindowLongPtrW
  inline LONG_PTR tmp_SetWindowLongPtrW( HWND hWnd, int nIndex, LONG_PTR dwNewLong )
  {
	return( ::SetWindowLongW( hWnd, nIndex, LONG( dwNewLong ) ) );
  }
  #define SetWindowLongPtrW tmp_SetWindowLongPtrW
#endif

#ifndef GetWindowLongPtrW
  inline LONG_PTR tmp_GetWindowLongPtrW( HWND hWnd, int nIndex )
  {
	return( ::GetWindowLongW( hWnd, nIndex ) );
  }
  #define GetWindowLongPtrW tmp_GetWindowLongPtrW
#endif

#ifndef LongToPtr
  #define LongToPtr(x) ((void*)x)
#endif

#ifndef PtrToInt
  #define PtrToInt( p ) ((INT)(INT_PTR) (p) )
#endif

#else // !(_ATL_VER >= 0x0800)

#ifdef lstrlenW
  #undef lstrlenW
  #define lstrlenW (int)::wcslen
#endif // lstrlenW

#define lstrlenA (int)strlen

#ifndef lstrcpyn
  inline LPTSTR lstrcpyn(LPTSTR lpstrDest, LPCTSTR lpstrSrc, int nLength)
  {
	if(lpstrDest == NULL || lpstrSrc == NULL || nLength <= 0)
		return NULL;
	int nLen = min(lstrlen(lpstrSrc), nLength - 1);
	LPTSTR lpstrRet = (LPTSTR)memcpy(lpstrDest, lpstrSrc, nLen * sizeof(TCHAR));
	lpstrDest[nLen] = 0;
	return lpstrRet;
  }
#endif // !lstrcpyn

#ifndef lstrcpynW
  inline LPWSTR lstrcpynW(LPWSTR lpstrDest, LPCWSTR lpstrSrc, int nLength)
  {
	return lstrcpyn(lpstrDest, lpstrSrc, nLength);   // WinCE is Unicode only
  }
#endif // !lstrcpynW

#ifndef lstrcpynA
  inline LPSTR lstrcpynA(LPSTR lpstrDest, LPCSTR lpstrSrc, int nLength)
  {
	if(lpstrDest == NULL || lpstrSrc == NULL || nLength <= 0)
		return NULL;
	int nLen = min(lstrlenA(lpstrSrc), nLength - 1);
	LPSTR lpstrRet = (LPSTR)memcpy(lpstrDest, lpstrSrc, nLen * sizeof(char));
	lpstrDest[nLen] = 0;
	return lpstrRet;
  }
#endif // !lstrcpyn

#ifdef TrackPopupMenu
  #undef TrackPopupMenu
#endif // TrackPopupMenu

#define DECLARE_WND_CLASS_EX(WndClassName, style, bkgnd) \
static CWndClassInfo& GetWndClassInfo() \
{ \
	static CWndClassInfo wc = \
	{ \
		{ style, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), NULL, WndClassName }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	return wc; \
}

#ifndef _MAX_FNAME
  #define _MAX_FNAME	_MAX_PATH
#endif // _MAX_FNAME

#if (_WIN32_WCE < 400)
  #define MAKEINTATOM(i)  (LPTSTR)((ULONG_PTR)((WORD)(i)))
#endif // (_WIN32_WCE < 400)

#if (_WIN32_WCE < 410)
  #define WHEEL_PAGESCROLL                (UINT_MAX)
  #define WHEEL_DELTA                     120
#endif // (_WIN32_WCE < 410)

#ifdef DrawIcon
  #undef DrawIcon
#endif

#ifndef VARCMP_LT
  #define VARCMP_LT   0
#endif
#ifndef VARCMP_EQ
  #define VARCMP_EQ   1
#endif
#ifndef VARCMP_GT
  #define VARCMP_GT   2
#endif
#ifndef VARCMP_NULL
  #define VARCMP_NULL 3
#endif

#ifndef RDW_ALLCHILDREN
  #define RDW_ALLCHILDREN   0
#endif

#endif // !(_ATL_VER >= 0x0800)

#endif // _WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// Global support for using original VC++ 6.0 headers with WTL

#ifndef _ATL_NO_OLD_HEADERS_WIN64
#if !defined(_WIN64) && (_ATL_VER < 0x0700)

  #ifndef PSM_INSERTPAGE
    #define PSM_INSERTPAGE          (WM_USER + 119)
  #endif // !PSM_INSERTPAGE

  #ifndef GetClassLongPtr
    #define GetClassLongPtrA   GetClassLongA
    #define GetClassLongPtrW   GetClassLongW
    #ifdef UNICODE
      #define GetClassLongPtr  GetClassLongPtrW
    #else
      #define GetClassLongPtr  GetClassLongPtrA
    #endif // !UNICODE
  #endif // !GetClassLongPtr

  #ifndef GCLP_HICONSM
    #define GCLP_HICONSM        (-34)
  #endif // !GCLP_HICONSM

  #ifndef GetWindowLongPtr
    #define GetWindowLongPtrA   GetWindowLongA
    #define GetWindowLongPtrW   GetWindowLongW
    #ifdef UNICODE
      #define GetWindowLongPtr  GetWindowLongPtrW
    #else
      #define GetWindowLongPtr  GetWindowLongPtrA
    #endif // !UNICODE
  #endif // !GetWindowLongPtr

  #ifndef SetWindowLongPtr
    #define SetWindowLongPtrA   SetWindowLongA
    #define SetWindowLongPtrW   SetWindowLongW
    #ifdef UNICODE
      #define SetWindowLongPtr  SetWindowLongPtrW
    #else
      #define SetWindowLongPtr  SetWindowLongPtrA
    #endif // !UNICODE
  #endif // !SetWindowLongPtr

  #ifndef GWLP_WNDPROC
    #define GWLP_WNDPROC        (-4)
  #endif
  #ifndef GWLP_HINSTANCE
    #define GWLP_HINSTANCE      (-6)
  #endif
  #ifndef GWLP_HWNDPARENT
    #define GWLP_HWNDPARENT     (-8)
  #endif
  #ifndef GWLP_USERDATA
    #define GWLP_USERDATA       (-21)
  #endif
  #ifndef GWLP_ID
    #define GWLP_ID             (-12)
  #endif

  #ifndef DWLP_MSGRESULT
    #define DWLP_MSGRESULT  0
  #endif

  typedef long LONG_PTR;
  typedef unsigned long ULONG_PTR;
  typedef ULONG_PTR DWORD_PTR;

  #ifndef HandleToUlong
    #define HandleToUlong( h ) ((ULONG)(ULONG_PTR)(h) )
  #endif
  #ifndef HandleToLong
    #define HandleToLong( h ) ((LONG)(LONG_PTR) (h) )
  #endif
  #ifndef LongToHandle
    #define LongToHandle( h) ((HANDLE)(LONG_PTR) (h))
  #endif
  #ifndef PtrToUlong
    #define PtrToUlong( p ) ((ULONG)(ULONG_PTR) (p) )
  #endif
  #ifndef PtrToLong
    #define PtrToLong( p ) ((LONG)(LONG_PTR) (p) )
  #endif
  #ifndef PtrToUint
    #define PtrToUint( p ) ((UINT)(UINT_PTR) (p) )
  #endif
  #ifndef PtrToInt
    #define PtrToInt( p ) ((INT)(INT_PTR) (p) )
  #endif
  #ifndef PtrToUshort
    #define PtrToUshort( p ) ((unsigned short)(ULONG_PTR)(p) )
  #endif
  #ifndef PtrToShort
    #define PtrToShort( p ) ((short)(LONG_PTR)(p) )
  #endif
  #ifndef IntToPtr
    #define IntToPtr( i )    ((VOID *)(INT_PTR)((int)i))
  #endif
  #ifndef UIntToPtr
    #define UIntToPtr( ui )  ((VOID *)(UINT_PTR)((unsigned int)ui))
  #endif
  #ifndef LongToPtr
    #define LongToPtr( l )   ((VOID *)(LONG_PTR)((long)l))
  #endif
  #ifndef ULongToPtr
    #define ULongToPtr( ul )  ((VOID *)(ULONG_PTR)((unsigned long)ul))
  #endif

#endif // !defined(_WIN64) && (_ATL_VER < 0x0700)
#endif // !_ATL_NO_OLD_HEADERS_WIN64


///////////////////////////////////////////////////////////////////////////////
// Global support for SecureHelper functions

#ifndef _TRUNCATE
  #define _TRUNCATE ((size_t)-1)
#endif

#ifndef _ERRCODE_DEFINED
  #define _ERRCODE_DEFINED
  typedef int errno_t;
#endif

#ifndef _SECURECRT_ERRCODE_VALUES_DEFINED
  #define _SECURECRT_ERRCODE_VALUES_DEFINED
  #define EINVAL          22
  #define STRUNCATE       80
#endif

#ifndef _countof
  #define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous global support

// define useful macros from winuser.h
#ifndef IS_INTRESOURCE
  #define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#endif // IS_INTRESOURCE

// protect template members from windowsx.h macros
#ifdef _INC_WINDOWSX
  #undef SubclassWindow
#endif // _INC_WINDOWSX

// define useful macros from windowsx.h
#ifndef GET_X_LPARAM
  #define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
  #define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif

// Dummy structs for compiling with /CLR
#if (_MSC_VER >= 1300) && defined(_MANAGED)
  __if_not_exists(_IMAGELIST::_IMAGELIST) { struct _IMAGELIST { }; }
  __if_not_exists(_TREEITEM::_TREEITEM) { struct _TREEITEM { }; }
  __if_not_exists(_PSP::_PSP) { struct _PSP { }; }
#endif

// Define ATLVERIFY macro for ATL3
#if (_ATL_VER < 0x0700)
  #ifndef ATLVERIFY
    #ifdef _DEBUG
      #define ATLVERIFY(expr) ATLASSERT(expr)
    #else
      #define ATLVERIFY(expr) (expr)
    #endif // DEBUG
  #endif // ATLVERIFY
#endif // (_ATL_VER < 0x0700)

// Forward declaration for ATL3 and ATL11 fix
#if (((_ATL_VER < 0x0700) && defined(_ATL_DLL)) || (_ATL_VER >= 0x0B00)) && !defined(_WIN32_WCE)
  namespace ATL { HRESULT AtlGetCommCtrlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor); };
#endif

#ifndef WM_MOUSEHWHEEL
  #define WM_MOUSEHWHEEL                  0x020E
#endif


namespace WTL
{

#if (_ATL_VER >= 0x0700)
  DECLARE_TRACE_CATEGORY(atlTraceUI);
  #ifdef _DEBUG
    __declspec(selectany) ATL::CTraceCategory atlTraceUI(_T("atlTraceUI"));
  #endif // _DEBUG
#else // !(_ATL_VER >= 0x0700)
  enum wtlTraceFlags
  {
	atlTraceUI = 0x10000000
  };
#endif // !(_ATL_VER >= 0x0700)

// Windows version helper
inline bool AtlIsOldWindows()
{
	OSVERSIONINFO ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	BOOL bRet = ::GetVersionEx(&ovi);
	return (!bRet || !((ovi.dwMajorVersion >= 5) || (ovi.dwMajorVersion == 4 && ovi.dwMinorVersion >= 90)));
}

// Default GUI font helper - "MS Shell Dlg" stock font
inline HFONT AtlGetDefaultGuiFont()
{
#ifndef _WIN32_WCE
	return (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
#else // CE specific
	return (HFONT)::GetStockObject(SYSTEM_FONT);
#endif // _WIN32_WCE
}

// Control font helper - default font for controls not in a dialog
// (NOTE: Caller owns the font, and should destroy it when it's no longer needed)
inline HFONT AtlCreateControlFont()
{
#ifndef _WIN32_WCE
	LOGFONT lf = { 0 };
	ATLVERIFY(::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0) != FALSE);
	HFONT hFont = ::CreateFontIndirect(&lf);
	ATLASSERT(hFont != NULL);
	return hFont;
#else // CE specific
	return (HFONT)::GetStockObject(SYSTEM_FONT);
#endif // _WIN32_WCE
}

// Bold font helper
// (NOTE: Caller owns the font, and should destroy it when it's no longer needed)
inline HFONT AtlCreateBoldFont(HFONT hFont = NULL)
{
	LOGFONT lf = { 0 };
#ifndef _WIN32_WCE
	if(hFont == NULL)
		ATLVERIFY(::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0) != FALSE);
	else
		ATLVERIFY(::GetObject(hFont, sizeof(LOGFONT), &lf) == sizeof(LOGFONT));
#else // CE specific
	if(hFont == NULL)
		hFont = (HFONT)::GetStockObject(SYSTEM_FONT);
	ATLVERIFY(::GetObject(hFont, sizeof(LOGFONT), &lf) == sizeof(LOGFONT));
#endif // _WIN32_WCE
	lf.lfWeight = FW_BOLD;
	HFONT hFontBold =  ::CreateFontIndirect(&lf);
	ATLASSERT(hFontBold != NULL);
	return hFontBold;
}

// Common Controls initialization helper
inline BOOL AtlInitCommonControls(DWORD dwFlags)
{
	INITCOMMONCONTROLSEX iccx = { sizeof(INITCOMMONCONTROLSEX), dwFlags };
	BOOL bRet = ::InitCommonControlsEx(&iccx);
	ATLASSERT(bRet);
	return bRet;
}


///////////////////////////////////////////////////////////////////////////////
// RunTimeHelper - helper functions for Windows version and structure sizes

// Not for Windows CE
#if defined(_WIN32_WCE) && !defined(_WTL_NO_RUNTIME_STRUCT_SIZE)
  #define _WTL_NO_RUNTIME_STRUCT_SIZE
#endif

#ifndef _WTL_NO_RUNTIME_STRUCT_SIZE

#ifndef _SIZEOF_STRUCT
  #define _SIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

#if (_WIN32_WINNT >= 0x0600) && !defined(REBARBANDINFO_V6_SIZE)
  #define REBARBANDINFO_V6_SIZE   _SIZEOF_STRUCT(REBARBANDINFO, cxHeader)
#endif // (_WIN32_WINNT >= 0x0600) && !defined(REBARBANDINFO_V6_SIZE)

#if (_WIN32_WINNT >= 0x0600) && !defined(LVGROUP_V5_SIZE)
  #define LVGROUP_V5_SIZE   _SIZEOF_STRUCT(LVGROUP, uAlign)
#endif // (_WIN32_WINNT >= 0x0600) && !defined(LVGROUP_V5_SIZE)

#if (_WIN32_WINNT >= 0x0600) && !defined(LVTILEINFO_V5_SIZE)
  #define LVTILEINFO_V5_SIZE   _SIZEOF_STRUCT(LVTILEINFO, puColumns)
#endif // (_WIN32_WINNT >= 0x0600) && !defined(LVTILEINFO_V5_SIZE)

#if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN) && !defined(MCHITTESTINFO_V1_SIZE)
  #define MCHITTESTINFO_V1_SIZE   _SIZEOF_STRUCT(MCHITTESTINFO, st)
#endif // defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN) && !defined(MCHITTESTINFO_V1_SIZE)

#if !defined(_WIN32_WCE) && (WINVER >= 0x0600) && !defined(NONCLIENTMETRICS_V1_SIZE)
  #define NONCLIENTMETRICS_V1_SIZE   _SIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont)
#endif // !defined(_WIN32_WCE) && (WINVER >= 0x0600) && !defined(NONCLIENTMETRICS_V1_SIZE)

#endif // !_WTL_NO_RUNTIME_STRUCT_SIZE

namespace RunTimeHelper
{
#ifndef _WIN32_WCE
	inline bool IsCommCtrl6()
	{
		DWORD dwMajor = 0, dwMinor = 0;
		HRESULT hRet = ATL::AtlGetCommCtrlVersion(&dwMajor, &dwMinor);
		return (SUCCEEDED(hRet) && (dwMajor >= 6));
	}

	inline bool IsVista()
	{
		OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
		BOOL bRet = ::GetVersionEx(&ovi);
		return ((bRet != FALSE) && (ovi.dwMajorVersion >= 6));
	}

	inline bool IsThemeAvailable()
	{
		bool bRet = false;

		if(IsCommCtrl6())
		{
			HMODULE hThemeDLL = ::LoadLibrary(_T("uxtheme.dll"));
			if(hThemeDLL != NULL)
			{
				typedef BOOL (STDAPICALLTYPE *PFN_IsThemeActive)();
				PFN_IsThemeActive pfnIsThemeActive = (PFN_IsThemeActive)::GetProcAddress(hThemeDLL, "IsThemeActive");
				ATLASSERT(pfnIsThemeActive != NULL);
				bRet = (pfnIsThemeActive != NULL) && (pfnIsThemeActive() != FALSE);
				if(bRet)
				{
					typedef BOOL (STDAPICALLTYPE *PFN_IsAppThemed)();
					PFN_IsAppThemed pfnIsAppThemed = (PFN_IsAppThemed)::GetProcAddress(hThemeDLL, "IsAppThemed");
					ATLASSERT(pfnIsAppThemed != NULL);
					bRet = (pfnIsAppThemed != NULL) && (pfnIsAppThemed() != FALSE);
				}

				::FreeLibrary(hThemeDLL);
			}
		}

		return bRet;
	}

	inline bool IsWin7()
	{
		OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
		BOOL bRet = ::GetVersionEx(&ovi);
		return ((bRet != FALSE) && (ovi.dwMajorVersion == 6) && (ovi.dwMinorVersion >= 1));
	}

	inline bool IsRibbonUIAvailable()
	{
		static INT iRibbonUI = -1;

#if defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7)
		if (iRibbonUI == -1)
		{
			HMODULE hRibbonDLL = ::LoadLibrary(_T("propsys.dll"));
			if (hRibbonDLL != NULL)
			{
				const GUID CLSID_UIRibbonFramework = { 0x926749fa, 0x2615, 0x4987, { 0x88, 0x45, 0xc3, 0x3e, 0x65, 0xf2, 0xb9, 0x57 } };
				// block - create instance
				{
					ATL::CComPtr<IUnknown> pIUIFramework;
					iRibbonUI = SUCCEEDED(pIUIFramework.CoCreateInstance(CLSID_UIRibbonFramework)) ? 1 : 0;
				}
				::FreeLibrary(hRibbonDLL);
			}
			else
			{
				iRibbonUI = 0;
			}
		}
#endif // defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7)

		return (iRibbonUI == 1);
	}

#endif // !_WIN32_WCE

	inline int SizeOf_REBARBANDINFO()
	{
		int nSize = sizeof(REBARBANDINFO);
#if !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (_WIN32_WINNT >= 0x0600)
		if(!(IsVista() && IsCommCtrl6()))
			nSize = REBARBANDINFO_V6_SIZE;
#endif // !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (_WIN32_WINNT >= 0x0600)
		return nSize;
	}

#if (_WIN32_WINNT >= 0x501)
  	inline int SizeOf_LVGROUP()
	{
		int nSize = sizeof(LVGROUP);
#if !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (_WIN32_WINNT >= 0x0600)
		if(!IsVista())
			nSize = LVGROUP_V5_SIZE;
#endif // !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (_WIN32_WINNT >= 0x0600)
		return nSize;
	}

	inline int SizeOf_LVTILEINFO()
	{
		int nSize = sizeof(LVTILEINFO);
#if !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (_WIN32_WINNT >= 0x0600)
		if(!IsVista())
			nSize = LVTILEINFO_V5_SIZE;
#endif // !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (_WIN32_WINNT >= 0x0600)
		return nSize;
	}
#endif // (_WIN32_WINNT >= 0x501)

	inline int SizeOf_MCHITTESTINFO()
	{
		int nSize = sizeof(MCHITTESTINFO);
#if !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
		if(!(IsVista() && IsCommCtrl6()))
			nSize = MCHITTESTINFO_V1_SIZE;
#endif // !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_LONGHORN)
		return nSize;
	}

#ifndef _WIN32_WCE
	inline int SizeOf_NONCLIENTMETRICS()
	{
		int nSize = sizeof(NONCLIENTMETRICS);
#if !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (WINVER >= 0x0600)
		if(!IsVista())
			nSize = NONCLIENTMETRICS_V1_SIZE;
#endif // !defined(_WTL_NO_RUNTIME_STRUCT_SIZE) && (WINVER >= 0x0600)
		return nSize;
	}
#endif // !_WIN32_WCE
};


///////////////////////////////////////////////////////////////////////////////
// ModuleHelper - helper functions for ATL3 and ATL7 module classes

namespace ModuleHelper
{
	inline HINSTANCE GetModuleInstance()
	{
#if (_ATL_VER >= 0x0700)
		return ATL::_AtlBaseModule.GetModuleInstance();
#else // !(_ATL_VER >= 0x0700)
		return ATL::_pModule->GetModuleInstance();
#endif // !(_ATL_VER >= 0x0700)
	}

	inline HINSTANCE GetResourceInstance()
	{
#if (_ATL_VER >= 0x0700)
		return ATL::_AtlBaseModule.GetResourceInstance();
#else // !(_ATL_VER >= 0x0700)
		return ATL::_pModule->GetResourceInstance();
#endif // !(_ATL_VER >= 0x0700)
	}

	inline void AddCreateWndData(ATL::_AtlCreateWndData* pData, void* pObject)
	{
#if (_ATL_VER >= 0x0700)
		ATL::_AtlWinModule.AddCreateWndData(pData, pObject);
#else // !(_ATL_VER >= 0x0700)
		ATL::_pModule->AddCreateWndData(pData, pObject);
#endif // !(_ATL_VER >= 0x0700)
	}

	inline void* ExtractCreateWndData()
	{
#if (_ATL_VER >= 0x0700)
		return ATL::_AtlWinModule.ExtractCreateWndData();
#else // !(_ATL_VER >= 0x0700)
		return ATL::_pModule->ExtractCreateWndData();
#endif // !(_ATL_VER >= 0x0700)
	}
};


///////////////////////////////////////////////////////////////////////////////
// SecureHelper - helper functions for VS2005 secure CRT

namespace SecureHelper
{
	inline void strcpyA_x(char* lpstrDest, size_t cchDest, const char* lpstrSrc)
	{
#if _SECURE_ATL
		ATL::Checked::strcpy_s(lpstrDest, cchDest, lpstrSrc);
#else
		if(cchDest > (size_t)lstrlenA(lpstrSrc))
			ATLVERIFY(lstrcpyA(lpstrDest, lpstrSrc) != NULL);
		else
			ATLASSERT(FALSE);
#endif
	}

	inline void strcpyW_x(wchar_t* lpstrDest, size_t cchDest, const wchar_t* lpstrSrc)
	{
#if _SECURE_ATL
		ATL::Checked::wcscpy_s(lpstrDest, cchDest, lpstrSrc);
#else
		if(cchDest > (size_t)lstrlenW(lpstrSrc))
			ATLVERIFY(lstrcpyW(lpstrDest, lpstrSrc) != NULL);
		else
			ATLASSERT(FALSE);
#endif
	}

	inline void strcpy_x(LPTSTR lpstrDest, size_t cchDest, LPCTSTR lpstrSrc)
	{
#ifdef _UNICODE
		strcpyW_x(lpstrDest, cchDest, lpstrSrc);
#else
		strcpyA_x(lpstrDest, cchDest, lpstrSrc);
#endif
	}

	inline errno_t strncpyA_x(char* lpstrDest, size_t cchDest, const char* lpstrSrc, size_t cchCount)
	{
#if _SECURE_ATL
		return ATL::Checked::strncpy_s(lpstrDest, cchDest, lpstrSrc, cchCount);
#else
		errno_t nRet = 0;
		if(lpstrDest == NULL || cchDest == 0 || lpstrSrc == NULL)
		{
			nRet = EINVAL;
		}
		else if(cchCount == _TRUNCATE)
		{
			cchCount = min(cchDest - 1, size_t(lstrlenA(lpstrSrc)));
			nRet = STRUNCATE;
		}
		else if(cchDest <= cchCount)
		{
			lpstrDest[0] = 0;
			nRet = EINVAL;
		}
		if(nRet == 0 || nRet == STRUNCATE)
			nRet = (lstrcpynA(lpstrDest, lpstrSrc, (int)cchCount + 1) != NULL) ? nRet : EINVAL;
		ATLASSERT(nRet == 0 || nRet == STRUNCATE);
		return nRet;
#endif
	}

	inline errno_t strncpyW_x(wchar_t* lpstrDest, size_t cchDest, const wchar_t* lpstrSrc, size_t cchCount)
	{
#if _SECURE_ATL
		return ATL::Checked::wcsncpy_s(lpstrDest, cchDest, lpstrSrc, cchCount);
#else
		errno_t nRet = 0;
		if(lpstrDest == NULL || cchDest == 0 || lpstrSrc == NULL)
		{
			nRet = EINVAL;
		}
		else if(cchCount == _TRUNCATE)
		{
			cchCount = min(cchDest - 1, size_t(lstrlenW(lpstrSrc)));
			nRet = STRUNCATE;
		}
		else if(cchDest <= cchCount)
		{
			lpstrDest[0] = 0;
			nRet = EINVAL;
		}
		if(nRet == 0 || nRet == STRUNCATE)
			nRet = (lstrcpynW(lpstrDest, lpstrSrc, (int)cchCount + 1) != NULL) ? nRet : EINVAL;
		ATLASSERT(nRet == 0 || nRet == STRUNCATE);
		return nRet;
#endif
	}

	inline errno_t strncpy_x(LPTSTR lpstrDest, size_t cchDest, LPCTSTR lpstrSrc, size_t cchCount)
	{
#ifdef _UNICODE
		return strncpyW_x(lpstrDest, cchDest, lpstrSrc, cchCount);
#else
		return strncpyA_x(lpstrDest, cchDest, lpstrSrc, cchCount);
#endif
	}

	inline void strcatA_x(char* lpstrDest, size_t cchDest, const char* lpstrSrc)
	{
#if _SECURE_ATL
		ATL::Checked::strcat_s(lpstrDest, cchDest, lpstrSrc);
#else
		if(cchDest > (size_t)lstrlenA(lpstrSrc))
			ATLVERIFY(lstrcatA(lpstrDest, lpstrSrc) != NULL);
		else
			ATLASSERT(FALSE);
#endif
	}

	inline void strcatW_x(wchar_t* lpstrDest, size_t cchDest, const wchar_t* lpstrSrc)
	{
#if _SECURE_ATL
		ATL::Checked::wcscat_s(lpstrDest, cchDest, lpstrSrc);
#else
		if(cchDest > (size_t)lstrlenW(lpstrSrc))
			ATLVERIFY(lstrcatW(lpstrDest, lpstrSrc) != NULL);
		else
			ATLASSERT(FALSE);
#endif
	}

	inline void strcat_x(LPTSTR lpstrDest, size_t cchDest, LPCTSTR lpstrSrc)
	{
#ifdef _UNICODE
		strcatW_x(lpstrDest, cchDest, lpstrSrc);
#else
		strcatA_x(lpstrDest, cchDest, lpstrSrc);
#endif
	}

	inline void memcpy_x(void* pDest, size_t cbDest, const void* pSrc, size_t cbSrc)
	{
#if _SECURE_ATL
		ATL::Checked::memcpy_s(pDest, cbDest, pSrc, cbSrc);
#else
		if(cbDest >= cbSrc)
			memcpy(pDest, pSrc, cbSrc);
		else
			ATLASSERT(FALSE);
#endif
	}

	inline void memmove_x(void* pDest, size_t cbDest, const void* pSrc, size_t cbSrc)
	{
#if _SECURE_ATL
		ATL::Checked::memmove_s(pDest, cbDest, pSrc, cbSrc);
#else
		if(cbDest >= cbSrc)
			memmove(pDest, pSrc, cbSrc);
		else
			ATLASSERT(FALSE);
#endif
	}

	inline int vsprintf_x(LPTSTR lpstrBuff, size_t cchBuff, LPCTSTR lpstrFormat, va_list args)
	{
#if _SECURE_ATL && !defined(_ATL_MIN_CRT) && !defined(_WIN32_WCE)
		return _vstprintf_s(lpstrBuff, cchBuff, lpstrFormat, args);
#else
		cchBuff;   // Avoid unused argument warning
#pragma warning(disable: 4996)
		return _vstprintf(lpstrBuff, lpstrFormat, args);
#pragma warning(default: 4996)
#endif
	}

	inline int wvsprintf_x(LPTSTR lpstrBuff, size_t cchBuff, LPCTSTR lpstrFormat, va_list args)
	{
#if _SECURE_ATL && !defined(_ATL_MIN_CRT) && !defined(_WIN32_WCE)
		return _vstprintf_s(lpstrBuff, cchBuff, lpstrFormat, args);
#else
		cchBuff;   // Avoid unused argument warning
		return ::wvsprintf(lpstrBuff, lpstrFormat, args);
#endif
	}

	inline int sprintf_x(LPTSTR lpstrBuff, size_t cchBuff, LPCTSTR lpstrFormat, ...)
	{
		va_list args;
		va_start(args, lpstrFormat);
		int nRes = vsprintf_x(lpstrBuff, cchBuff, lpstrFormat, args);
		va_end(args);
		return nRes;
	}

	inline int wsprintf_x(LPTSTR lpstrBuff, size_t cchBuff, LPCTSTR lpstrFormat, ...)
	{
		va_list args;
		va_start(args, lpstrFormat);
		int nRes = wvsprintf_x(lpstrBuff, cchBuff, lpstrFormat, args);
		va_end(args);
		return nRes;
	}
}; // namespace SecureHelper


///////////////////////////////////////////////////////////////////////////////
// MinCrtHelper - helper functions for using _ATL_MIN_CRT

namespace MinCrtHelper
{
	inline int _isspace(TCHAR ch)
	{
#ifndef _ATL_MIN_CRT
		return _istspace(ch);
#else // _ATL_MIN_CRT
		WORD type = 0;
		::GetStringTypeEx(::GetThreadLocale(), CT_CTYPE1, &ch, 1, &type);
		return (type & C1_SPACE) == C1_SPACE;
#endif // _ATL_MIN_CRT
	}

	inline int _isdigit(TCHAR ch)
	{
#ifndef _ATL_MIN_CRT
		return _istdigit(ch);
#else // _ATL_MIN_CRT
		WORD type = 0;
		::GetStringTypeEx(::GetThreadLocale(), CT_CTYPE1, &ch, 1, &type);
		return (type & C1_DIGIT) == C1_DIGIT;
#endif // _ATL_MIN_CRT
	}

	inline int _atoi(LPCTSTR str)
	{
#ifndef _ATL_MIN_CRT
		return _ttoi(str);
#else // _ATL_MIN_CRT
		while(_isspace(*str) != 0)
			++str;

		TCHAR ch = *str++;
		TCHAR sign = ch;   // save sign indication
		if(ch == _T('-') || ch == _T('+'))
			ch = *str++;   // skip sign

		int total = 0;
		while(_isdigit(ch) != 0)
		{
			total = 10 * total + (ch - '0');   // accumulate digit
			ch = *str++;        // get next char
		}

		return (sign == '-') ? -total : total;   // return result, negated if necessary
#endif // _ATL_MIN_CRT
	}

	inline LPCTSTR _strrchr(LPCTSTR str, TCHAR ch)
	{
#ifndef _ATL_MIN_CRT
		return _tcsrchr(str, ch);
#else // _ATL_MIN_CRT
		LPCTSTR lpsz = NULL;
		while(*str != 0)
		{
			if(*str == ch)
				lpsz = str;
			str = ::CharNext(str);
		}
		return lpsz;
#endif // _ATL_MIN_CRT
	}

	inline LPTSTR _strrchr(LPTSTR str, TCHAR ch)
	{
#ifndef _ATL_MIN_CRT
		return _tcsrchr(str, ch);
#else // _ATL_MIN_CRT
		LPTSTR lpsz = NULL;
		while(*str != 0)
		{
			if(*str == ch)
				lpsz = str;
			str = ::CharNext(str);
		}
		return lpsz;
#endif // _ATL_MIN_CRT
	}
}; // namespace MinCrtHelper


///////////////////////////////////////////////////////////////////////////////
// CMessageFilter - Interface for message filter support

class CMessageFilter
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;
};


///////////////////////////////////////////////////////////////////////////////
// CIdleHandler - Interface for idle processing

class CIdleHandler
{
public:
	virtual BOOL OnIdle() = 0;
};

#ifndef _ATL_NO_OLD_NAMES
  // for compatilibility with old names only
  typedef CIdleHandler CUpdateUIObject;
  #define DoUpdate OnIdle
#endif // !_ATL_NO_OLD_NAMES


///////////////////////////////////////////////////////////////////////////////
// CMessageLoop - message loop implementation

class CMessageLoop
{
public:
	ATL::CSimpleArray<CMessageFilter*> m_aMsgFilter;
	ATL::CSimpleArray<CIdleHandler*> m_aIdleHandler;
	MSG m_msg;

// Message filter operations
	BOOL AddMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Add(pMessageFilter);
	}

	BOOL RemoveMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Remove(pMessageFilter);
	}

// Idle handler operations
	BOOL AddIdleHandler(CIdleHandler* pIdleHandler)
	{
		return m_aIdleHandler.Add(pIdleHandler);
	}

	BOOL RemoveIdleHandler(CIdleHandler* pIdleHandler)
	{
		return m_aIdleHandler.Remove(pIdleHandler);
	}

#ifndef _ATL_NO_OLD_NAMES
	// for compatilibility with old names only
	BOOL AddUpdateUI(CIdleHandler* pIdleHandler)
	{
		ATLTRACE2(atlTraceUI, 0, _T("CUpdateUIObject and AddUpdateUI are deprecated. Please change your code to use CIdleHandler and OnIdle\n"));
		return AddIdleHandler(pIdleHandler);
	}

	BOOL RemoveUpdateUI(CIdleHandler* pIdleHandler)
	{
		ATLTRACE2(atlTraceUI, 0, _T("CUpdateUIObject and RemoveUpdateUI are deprecated. Please change your code to use CIdleHandler and OnIdle\n"));
		return RemoveIdleHandler(pIdleHandler);
	}
#endif // !_ATL_NO_OLD_NAMES

// message loop
	int Run()
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet;

		for(;;)
		{
			while(bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if(!OnIdle(nIdleCount++))
					bDoIdle = FALSE;
			}

			bRet = ::GetMessage(&m_msg, NULL, 0, 0);

			if(bRet == -1)
			{
				ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
				continue;   // error, don't process
			}
			else if(!bRet)
			{
				ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
				break;   // WM_QUIT, exit message loop
			}

			if(!PreTranslateMessage(&m_msg))
			{
				::TranslateMessage(&m_msg);
				::DispatchMessage(&m_msg);
			}

			if(IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}
		}

		return (int)m_msg.wParam;
	}

	static BOOL IsIdleMessage(MSG* pMsg)
	{
		// These messages should NOT cause idle processing
		switch(pMsg->message)
		{
		case WM_MOUSEMOVE:
#ifndef _WIN32_WCE
		case WM_NCMOUSEMOVE:
#endif // !_WIN32_WCE
		case WM_PAINT:
		case 0x0118:	// WM_SYSTIMER (caret blink)
			return FALSE;
		}

		return TRUE;
	}

// Overrideables
	// Override to change message filtering
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		// loop backwards
		for(int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--)
		{
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if(pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}
		return FALSE;   // not translated
	}

	// override to change idle processing
	virtual BOOL OnIdle(int /*nIdleCount*/)
	{
		for(int i = 0; i < m_aIdleHandler.GetSize(); i++)
		{
			CIdleHandler* pIdleHandler = m_aIdleHandler[i];
			if(pIdleHandler != NULL)
				pIdleHandler->OnIdle();
		}
		return FALSE;   // don't continue
	}
};


///////////////////////////////////////////////////////////////////////////////
// CStaticDataInitCriticalSectionLock and CWindowCreateCriticalSectionLock
// internal classes to manage critical sections for both ATL3 and ATL7

class CStaticDataInitCriticalSectionLock
{
public:
#if (_ATL_VER >= 0x0700)
	ATL::CComCritSecLock<ATL::CComCriticalSection> m_cslock;

	CStaticDataInitCriticalSectionLock() : m_cslock(ATL::_pAtlModule->m_csStaticDataInitAndTypeInfo, false)
	{ }
#endif // (_ATL_VER >= 0x0700)

	HRESULT Lock()
	{
#if (_ATL_VER >= 0x0700)
		return m_cslock.Lock();
#else // !(_ATL_VER >= 0x0700)
		::EnterCriticalSection(&ATL::_pModule->m_csStaticDataInit);
		return S_OK;
#endif // !(_ATL_VER >= 0x0700)
	}

	void Unlock()
	{
#if (_ATL_VER >= 0x0700)
		m_cslock.Unlock();
#else // !(_ATL_VER >= 0x0700)
		::LeaveCriticalSection(&ATL::_pModule->m_csStaticDataInit);
#endif // !(_ATL_VER >= 0x0700)
	}
};


class CWindowCreateCriticalSectionLock
{
public:
#if (_ATL_VER >= 0x0700)
	ATL::CComCritSecLock<ATL::CComCriticalSection> m_cslock;

	CWindowCreateCriticalSectionLock() : m_cslock(ATL::_AtlWinModule.m_csWindowCreate, false)
	{ }
#endif // (_ATL_VER >= 0x0700)

	HRESULT Lock()
	{
#if (_ATL_VER >= 0x0700)
		return m_cslock.Lock();
#else // !(_ATL_VER >= 0x0700)
		::EnterCriticalSection(&ATL::_pModule->m_csWindowCreate);
		return S_OK;
#endif // !(_ATL_VER >= 0x0700)
	}

	void Unlock()
	{
#if (_ATL_VER >= 0x0700)
		m_cslock.Unlock();
#else // !(_ATL_VER >= 0x0700)
		::LeaveCriticalSection(&ATL::_pModule->m_csWindowCreate);
#endif // !(_ATL_VER >= 0x0700)
	}
};


///////////////////////////////////////////////////////////////////////////////
// CTempBuffer - helper class for stack allocations for ATL3

#ifndef _WTL_STACK_ALLOC_THRESHOLD
  #define _WTL_STACK_ALLOC_THRESHOLD   512
#endif

#if (_ATL_VER >= 0x0700)

using ATL::CTempBuffer;

#else // !(_ATL_VER >= 0x0700)

#ifndef SIZE_MAX
  #ifdef _WIN64 
    #define SIZE_MAX _UI64_MAX
  #else
    #define SIZE_MAX UINT_MAX
  #endif
#endif

#pragma warning(disable: 4284)   // warning for operator ->

template<typename T, int t_nFixedBytes = 128>
class CTempBuffer
{
public:
	CTempBuffer() : m_p(NULL)
	{
	}

	CTempBuffer(size_t nElements) : m_p(NULL)
	{
		Allocate(nElements);
	}

	~CTempBuffer()
	{
		if(m_p != reinterpret_cast<T*>(m_abFixedBuffer))
			free(m_p);
	}

	operator T*() const
	{
		return m_p;
	}

	T* operator ->() const
	{
		ATLASSERT(m_p != NULL);
		return m_p;
	}

	T* Allocate(size_t nElements)
	{
		ATLASSERT(nElements <= (SIZE_MAX / sizeof(T)));
		return AllocateBytes(nElements * sizeof(T));
	}

	T* AllocateBytes(size_t nBytes)
	{
		ATLASSERT(m_p == NULL);
		if(nBytes > t_nFixedBytes)
			m_p = static_cast<T*>(malloc(nBytes));
		else
			m_p = reinterpret_cast<T*>(m_abFixedBuffer);

		return m_p;
	}

private:
	T* m_p;
	BYTE m_abFixedBuffer[t_nFixedBytes];
};

#pragma warning(default: 4284)

#endif // !(_ATL_VER >= 0x0700)


///////////////////////////////////////////////////////////////////////////////
// CAppModule - module class for an application

class CAppModule : public ATL::CComModule
{
public:
	DWORD m_dwMainThreadID;
	ATL::CSimpleMap<DWORD, CMessageLoop*>* m_pMsgLoopMap;
	ATL::CSimpleArray<HWND>* m_pSettingChangeNotify;

// Overrides of CComModule::Init and Term
	HRESULT Init(ATL::_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = NULL)
	{
		HRESULT hRet = CComModule::Init(pObjMap, hInstance, pLibID);
		if(FAILED(hRet))
			return hRet;

		m_dwMainThreadID = ::GetCurrentThreadId();
		typedef ATL::CSimpleMap<DWORD, CMessageLoop*>   _mapClass;
		m_pMsgLoopMap = NULL;
		ATLTRY(m_pMsgLoopMap = new _mapClass);
		if(m_pMsgLoopMap == NULL)
			return E_OUTOFMEMORY;
		m_pSettingChangeNotify = NULL;

		return hRet;
	}

	void Term()
	{
		TermSettingChangeNotify();
		delete m_pMsgLoopMap;
		CComModule::Term();
	}

// Message loop map methods
	BOOL AddMessageLoop(CMessageLoop* pMsgLoop)
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::AddMessageLoop.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		ATLASSERT(pMsgLoop != NULL);
		ATLASSERT(m_pMsgLoopMap->Lookup(::GetCurrentThreadId()) == NULL);   // not in map yet

		BOOL bRet = m_pMsgLoopMap->Add(::GetCurrentThreadId(), pMsgLoop);

		lock.Unlock();

		return bRet;
	}

	BOOL RemoveMessageLoop()
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::RemoveMessageLoop.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		BOOL bRet = m_pMsgLoopMap->Remove(::GetCurrentThreadId());

		lock.Unlock();

		return bRet;
	}

	CMessageLoop* GetMessageLoop(DWORD dwThreadID = ::GetCurrentThreadId()) const
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::GetMessageLoop.\n"));
			ATLASSERT(FALSE);
			return NULL;
		}

		CMessageLoop* pLoop =  m_pMsgLoopMap->Lookup(dwThreadID);

		lock.Unlock();

		return pLoop;
	}

// Setting change notify methods
	// Note: Call this from the main thread for MSDI apps
	BOOL InitSettingChangeNotify(DLGPROC pfnDlgProc = _SettingChangeDlgProc)
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::InitSettingChangeNotify.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		if(m_pSettingChangeNotify == NULL)
		{
			typedef ATL::CSimpleArray<HWND>   _notifyClass;
			ATLTRY(m_pSettingChangeNotify = new _notifyClass);
			ATLASSERT(m_pSettingChangeNotify != NULL);
		}

		BOOL bRet = (m_pSettingChangeNotify != NULL);
		if(bRet && m_pSettingChangeNotify->GetSize() == 0)
		{
			// init everything
			_ATL_EMPTY_DLGTEMPLATE templ;
			HWND hNtfWnd = ::CreateDialogIndirect(GetModuleInstance(), &templ, NULL, pfnDlgProc);
			ATLASSERT(::IsWindow(hNtfWnd));
			if(::IsWindow(hNtfWnd))
			{
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
				::SetWindowLongPtr(hNtfWnd, GWLP_USERDATA, (LONG_PTR)this);
#else
				::SetWindowLongPtr(hNtfWnd, GWLP_USERDATA, PtrToLong(this));
#endif
				bRet = m_pSettingChangeNotify->Add(hNtfWnd);
			}
			else
			{
				bRet = FALSE;
			}
		}

		lock.Unlock();

		return bRet;
	}

	void TermSettingChangeNotify()
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::TermSettingChangeNotify.\n"));
			ATLASSERT(FALSE);
			return;
		}

		if(m_pSettingChangeNotify != NULL && m_pSettingChangeNotify->GetSize() > 0)
			::DestroyWindow((*m_pSettingChangeNotify)[0]);
		delete m_pSettingChangeNotify;
		m_pSettingChangeNotify = NULL;

		lock.Unlock();
	}

	BOOL AddSettingChangeNotify(HWND hWnd)
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::AddSettingChangeNotify.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = FALSE;
		if(InitSettingChangeNotify() != FALSE)
			bRet = m_pSettingChangeNotify->Add(hWnd);

		lock.Unlock();

		return bRet;
	}

	BOOL RemoveSettingChangeNotify(HWND hWnd)
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::RemoveSettingChangeNotify.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		BOOL bRet = FALSE;
		if(m_pSettingChangeNotify != NULL)
			bRet = m_pSettingChangeNotify->Remove(hWnd);

		lock.Unlock();

		return bRet;
	}

// Implementation - setting change notify dialog template and dialog procedure
	struct _ATL_EMPTY_DLGTEMPLATE : DLGTEMPLATE
	{
		_ATL_EMPTY_DLGTEMPLATE()
		{
			memset(this, 0, sizeof(_ATL_EMPTY_DLGTEMPLATE));
			style = WS_POPUP;
		}
		WORD wMenu, wClass, wTitle;
	};

#ifdef _WIN64
	static INT_PTR CALLBACK _SettingChangeDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
#else
	static BOOL CALLBACK _SettingChangeDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
#endif
	{
		if(uMsg == WM_SETTINGCHANGE)
		{
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
			CAppModule* pModule = (CAppModule*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
			CAppModule* pModule = (CAppModule*)LongToPtr(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#endif
			ATLASSERT(pModule != NULL);
			ATLASSERT(pModule->m_pSettingChangeNotify != NULL);
			const UINT uTimeout = 1500;   // ms
			for(int i = 1; i < pModule->m_pSettingChangeNotify->GetSize(); i++)
			{
#if !defined(_WIN32_WCE)
				::SendMessageTimeout((*pModule->m_pSettingChangeNotify)[i], uMsg, wParam, lParam, SMTO_ABORTIFHUNG, uTimeout, NULL);
#elif(_WIN32_WCE >= 400) // CE specific
				::SendMessageTimeout((*pModule->m_pSettingChangeNotify)[i], uMsg, wParam, lParam, SMTO_NORMAL, uTimeout, NULL);
#else // _WIN32_WCE < 400 specific
				uTimeout;
				::SendMessage((*pModule->m_pSettingChangeNotify)[i], uMsg, wParam, lParam);
#endif
			}
			return TRUE;
		}
		return FALSE;
	}
};


///////////////////////////////////////////////////////////////////////////////
// CServerAppModule - module class for a COM server application

class CServerAppModule : public CAppModule
{
public:
	HANDLE m_hEventShutdown;
	bool m_bActivity;
	DWORD m_dwTimeOut;
	DWORD m_dwPause;

// Override of CAppModule::Init
	HRESULT Init(ATL::_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = NULL)
	{
		m_dwTimeOut = 5000;
		m_dwPause = 1000;
		return CAppModule::Init(pObjMap, hInstance, pLibID);
	}

	void Term()
	{
		if(m_hEventShutdown != NULL && ::CloseHandle(m_hEventShutdown))
			m_hEventShutdown = NULL;
		CAppModule::Term();
	}

// COM Server methods
	LONG Unlock()
	{
		LONG lRet = CComModule::Unlock();
		if(lRet == 0)
		{
			m_bActivity = true;
			::SetEvent(m_hEventShutdown); // tell monitor that we transitioned to zero
		}
		return lRet;
	}

	void MonitorShutdown()
	{
		for(;;)
		{
			::WaitForSingleObject(m_hEventShutdown, INFINITE);
			DWORD dwWait = 0;
			do
			{
				m_bActivity = false;
				dwWait = ::WaitForSingleObject(m_hEventShutdown, m_dwTimeOut);
			}
			while(dwWait == WAIT_OBJECT_0);
			// timed out
			if(!m_bActivity && m_nLockCnt == 0) // if no activity let's really bail
			{
#if ((_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)) && defined(_ATL_FREE_THREADED) && !defined(_WIN32_WCE)
				::CoSuspendClassObjects();
				if(!m_bActivity && m_nLockCnt == 0)
#endif
					break;
			}
		}
		// This handle should be valid now. If it isn't, 
		// check if _Module.Term was called first (it shouldn't)
		if(::CloseHandle(m_hEventShutdown))
			m_hEventShutdown = NULL;
		::PostThreadMessage(m_dwMainThreadID, WM_QUIT, 0, 0);
	}

	bool StartMonitor()
	{
		m_hEventShutdown = ::CreateEvent(NULL, false, false, NULL);
		if(m_hEventShutdown == NULL)
			return false;
		DWORD dwThreadID = 0;
#if !defined(_ATL_MIN_CRT) && defined(_MT) && !defined(_WIN32_WCE)
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, (UINT (WINAPI*)(void*))MonitorProc, this, 0, (UINT*)&dwThreadID);
#else
		HANDLE hThread = ::CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
#endif
		bool bRet = (hThread != NULL);
		if(bRet)
			::CloseHandle(hThread);
		return bRet;
	}

	static DWORD WINAPI MonitorProc(void* pv)
	{
		CServerAppModule* p = (CServerAppModule*)pv;
		p->MonitorShutdown();
		return 0;
	}

#if (_ATL_VER < 0x0700)
	// search for an occurence of string p2 in string p1
	static LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
	{
		while(p1 != NULL && *p1 != NULL)
		{
			LPCTSTR p = p2;
			while(p != NULL && *p != NULL)
			{
				if(*p1 == *p)
					return ::CharNext(p1);
				p = ::CharNext(p);
			}
			p1 = ::CharNext(p1);
		}
		return NULL;
	}
#endif // (_ATL_VER < 0x0700)
};


///////////////////////////////////////////////////////////////////////////////
// CRegKeyEx - adds type-specific methods to ATL3 CRegKey

#if (_ATL_VER < 0x0700)

class CRegKeyEx : public ATL::CRegKey
{
public:
// Constructors and operators
	CRegKeyEx(HKEY hKey = NULL)
	{
		m_hKey = hKey;
	}

	CRegKeyEx(CRegKeyEx& key)
	{
		Attach(key.Detach());
	}

	CRegKeyEx& operator =(CRegKeyEx& key)
	{
		Close();
		Attach(key.Detach());
		return *this;
	}

// Methods
	LONG SetValue(LPCTSTR pszValueName, DWORD dwType, const void* pValue, ULONG nBytes)
	{
		ATLASSERT(m_hKey != NULL);
		return ::RegSetValueEx(m_hKey, pszValueName, NULL, dwType, static_cast<const BYTE*>(pValue), nBytes);
	}

	LONG SetGUIDValue(LPCTSTR pszValueName, REFGUID guidValue)
	{
		ATLASSERT(m_hKey != NULL);

		OLECHAR szGUID[64] = { 0 };
		::StringFromGUID2(guidValue, szGUID, 64);

		USES_CONVERSION;
		LPCTSTR lpstr = OLE2CT(szGUID);
#ifndef _UNICODE
		if(lpstr == NULL) 
			return E_OUTOFMEMORY;
#endif	
		return SetStringValue(pszValueName, lpstr);
	}

	LONG SetBinaryValue(LPCTSTR pszValueName, const void* pValue, ULONG nBytes)
	{
		ATLASSERT(m_hKey != NULL);
		return ::RegSetValueEx(m_hKey, pszValueName, NULL, REG_BINARY, reinterpret_cast<const BYTE*>(pValue), nBytes);
	}

	LONG SetDWORDValue(LPCTSTR pszValueName, DWORD dwValue)
	{
		ATLASSERT(m_hKey != NULL);
		return ::RegSetValueEx(m_hKey, pszValueName, NULL, REG_DWORD, reinterpret_cast<const BYTE*>(&dwValue), sizeof(DWORD));
	}

#ifndef _WIN32_WCE
	LONG SetQWORDValue(LPCTSTR pszValueName, ULONGLONG qwValue)
	{
		ATLASSERT(m_hKey != NULL);
		return ::RegSetValueEx(m_hKey, pszValueName, NULL, REG_QWORD, reinterpret_cast<const BYTE*>(&qwValue), sizeof(ULONGLONG));
	}
#endif

	LONG SetStringValue(LPCTSTR pszValueName, LPCTSTR pszValue, DWORD dwType = REG_SZ)
	{
		ATLASSERT(m_hKey != NULL);
		if(pszValue == NULL)
		{
			ATLASSERT(FALSE);
			return ERROR_INVALID_DATA;
		}
		ATLASSERT((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ));

		return ::RegSetValueEx(m_hKey, pszValueName, NULL, dwType, reinterpret_cast<const BYTE*>(pszValue), (lstrlen(pszValue) + 1) * sizeof(TCHAR));
	}

	LONG SetMultiStringValue(LPCTSTR pszValueName, LPCTSTR pszValue)
	{
		ATLASSERT(m_hKey != NULL);
		if(pszValue == NULL)
		{
			ATLASSERT(FALSE);
			return ERROR_INVALID_DATA;
		}

		ULONG nBytes = 0;
		ULONG nLength = 0;
		LPCTSTR pszTemp = pszValue;
		do
		{
			nLength = lstrlen(pszTemp) + 1;
			pszTemp += nLength;
			nBytes += nLength * sizeof(TCHAR);
		} while (nLength != 1);

		return ::RegSetValueEx(m_hKey, pszValueName, NULL, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(pszValue), nBytes);
	}

	LONG QueryValue(LPCTSTR pszValueName, DWORD* pdwType, void* pData, ULONG* pnBytes)
	{
		ATLASSERT(m_hKey != NULL);
		return ::RegQueryValueEx(m_hKey, pszValueName, NULL, pdwType, static_cast<LPBYTE>(pData), pnBytes);
	}

	LONG QueryGUIDValue(LPCTSTR pszValueName, GUID& guidValue)
	{
		ATLASSERT(m_hKey != NULL);

		guidValue = GUID_NULL;

		TCHAR szGUID[64] = { 0 };
		ULONG nCount = 64;
		LONG lRes = QueryStringValue(pszValueName, szGUID, &nCount);

		if (lRes != ERROR_SUCCESS)
			return lRes;

		if(szGUID[0] != _T('{'))
			return ERROR_INVALID_DATA;

		USES_CONVERSION;
		LPOLESTR lpstr = T2OLE(szGUID);
#ifndef _UNICODE
		if(lpstr == NULL) 
			return E_OUTOFMEMORY;
#endif	
		
		HRESULT hr = ::CLSIDFromString(lpstr, &guidValue);
		if (FAILED(hr))
			return ERROR_INVALID_DATA;

		return ERROR_SUCCESS;
	}

	LONG QueryBinaryValue(LPCTSTR pszValueName, void* pValue, ULONG* pnBytes)
	{
		ATLASSERT(pnBytes != NULL);
		ATLASSERT(m_hKey != NULL);

		DWORD dwType = 0;
		LONG lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(pValue), pnBytes);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		if (dwType != REG_BINARY)
			return ERROR_INVALID_DATA;

		return ERROR_SUCCESS;
	}

	LONG QueryDWORDValue(LPCTSTR pszValueName, DWORD& dwValue)
	{
		ATLASSERT(m_hKey != NULL);

		ULONG nBytes = sizeof(DWORD);
		DWORD dwType = 0;
		LONG lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(&dwValue), &nBytes);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		if (dwType != REG_DWORD)
			return ERROR_INVALID_DATA;

		return ERROR_SUCCESS;
	}

	LONG QueryQWORDValue(LPCTSTR pszValueName, ULONGLONG& qwValue)
	{
		ATLASSERT(m_hKey != NULL);

		ULONG nBytes = sizeof(ULONGLONG);
		DWORD dwType = 0;
		LONG lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(&qwValue), &nBytes);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		if (dwType != REG_QWORD)
			return ERROR_INVALID_DATA;

		return ERROR_SUCCESS;
	}

	LONG QueryStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars)
	{
		ATLASSERT(m_hKey != NULL);
		ATLASSERT(pnChars != NULL);

		ULONG nBytes = (*pnChars) * sizeof(TCHAR);
		DWORD dwType = 0;
		*pnChars = 0;
		LONG lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(pszValue), &nBytes);
	
		if (lRes != ERROR_SUCCESS)
		{
			return lRes;
		}

		if(dwType != REG_SZ && dwType != REG_EXPAND_SZ)
		{
			return ERROR_INVALID_DATA;
		}

		if (pszValue != NULL)
		{
			if(nBytes != 0)
			{
				if ((nBytes % sizeof(TCHAR) != 0) || (pszValue[nBytes / sizeof(TCHAR) -1] != 0))
					return ERROR_INVALID_DATA;
			}
			else
			{
				pszValue[0] = _T('\0');
			}
		}

		*pnChars = nBytes / sizeof(TCHAR);

		return ERROR_SUCCESS;
	}

	LONG QueryMultiStringValue(LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars)
	{
		ATLASSERT(m_hKey != NULL);
		ATLASSERT(pnChars != NULL);

		if (pszValue != NULL && *pnChars < 2)
			return ERROR_INSUFFICIENT_BUFFER;
		
		ULONG nBytes = (*pnChars) * sizeof(TCHAR);
		DWORD dwType = 0;
		*pnChars = 0;
		LONG lRes = ::RegQueryValueEx(m_hKey, pszValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(pszValue), &nBytes);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		if (dwType != REG_MULTI_SZ)
			return ERROR_INVALID_DATA;
		if (pszValue != NULL && (nBytes % sizeof(TCHAR) != 0 || nBytes / sizeof(TCHAR) < 1 || pszValue[nBytes / sizeof(TCHAR) - 1] != 0 || ((nBytes / sizeof(TCHAR)) > 1 && pszValue[nBytes / sizeof(TCHAR) - 2] != 0)))
			return ERROR_INVALID_DATA;

		*pnChars = nBytes / sizeof(TCHAR);

		return ERROR_SUCCESS;
	}
};

#else // !(_ATL_VER < 0x0700)

typedef ATL::CRegKey CRegKeyEx;

#endif // !(_ATL_VER < 0x0700)


///////////////////////////////////////////////////////////////////////////////
// CString forward reference (enables CString use in atluser.h and atlgdi.h)

#if defined(_WTL_FORWARD_DECLARE_CSTRING) && !defined(_WTL_USE_CSTRING)
  #define _WTL_USE_CSTRING
#endif // defined(_WTL_FORWARD_DECLARE_CSTRING) && !defined(_WTL_USE_CSTRING)

#ifdef _WTL_USE_CSTRING
  class CString;   // forward declaration (include atlmisc.h for the whole class)
#endif // _WTL_USE_CSTRING

// CString namespace
#ifndef _CSTRING_NS
  #ifdef __ATLSTR_H__
    #define _CSTRING_NS	ATL
  #else
    #define _CSTRING_NS	WTL
  #endif
#endif // _CSTRING_NS

// Type classes namespace
#ifndef _WTYPES_NS
  #ifdef __ATLTYPES_H__
    #define _WTYPES_NS
  #else
    #define _WTYPES_NS	WTL
  #endif
#endif // _WTYPES_NS

}; // namespace WTL


///////////////////////////////////////////////////////////////////////////////
// General DLL version helpers
// (ATL3: excluded from atlbase.h if _ATL_DLL is defined; ATL11: removed)

#if (((_ATL_VER < 0x0700) && defined(_ATL_DLL)) || (_ATL_VER >= 0x0B00)) && !defined(_WIN32_WCE)

namespace ATL
{

inline HRESULT AtlGetDllVersion(HINSTANCE hInstDLL, DLLVERSIONINFO* pDllVersionInfo)
{
	ATLASSERT(pDllVersionInfo != NULL);
	if(pDllVersionInfo == NULL)
		return E_INVALIDARG;

	// We must get this function explicitly because some DLLs don't implement it.
	DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hInstDLL, "DllGetVersion");
	if(pfnDllGetVersion == NULL)
		return E_NOTIMPL;

	return (*pfnDllGetVersion)(pDllVersionInfo);
}

inline HRESULT AtlGetDllVersion(LPCTSTR lpstrDllName, DLLVERSIONINFO* pDllVersionInfo)
{
	HINSTANCE hInstDLL = ::LoadLibrary(lpstrDllName);
	if(hInstDLL == NULL)
		return E_FAIL;
	HRESULT hRet = AtlGetDllVersion(hInstDLL, pDllVersionInfo);
	::FreeLibrary(hInstDLL);
	return hRet;
}

// Common Control Versions:
//   Win95/WinNT 4.0    maj=4 min=00
//   IE 3.x     maj=4 min=70
//   IE 4.0     maj=4 min=71
inline HRESULT AtlGetCommCtrlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	ATLASSERT(pdwMajor != NULL && pdwMinor != NULL);
	if(pdwMajor == NULL || pdwMinor == NULL)
		return E_INVALIDARG;

	DLLVERSIONINFO dvi;
	::ZeroMemory(&dvi, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hRet = AtlGetDllVersion(_T("comctl32.dll"), &dvi);

	if(SUCCEEDED(hRet))
	{
		*pdwMajor = dvi.dwMajorVersion;
		*pdwMinor = dvi.dwMinorVersion;
	}
	else if(hRet == E_NOTIMPL)
	{
		// If DllGetVersion is not there, then the DLL is a version
		// previous to the one shipped with IE 3.x
		*pdwMajor = 4;
		*pdwMinor = 0;
		hRet = S_OK;
	}

	return hRet;
}

// Shell Versions:
//   Win95/WinNT 4.0                    maj=4 min=00
//   IE 3.x, IE 4.0 without Web Integrated Desktop  maj=4 min=00
//   IE 4.0 with Web Integrated Desktop         maj=4 min=71
//   IE 4.01 with Web Integrated Desktop        maj=4 min=72
inline HRESULT AtlGetShellVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	ATLASSERT(pdwMajor != NULL && pdwMinor != NULL);
	if(pdwMajor == NULL || pdwMinor == NULL)
		return E_INVALIDARG;

	DLLVERSIONINFO dvi;
	::ZeroMemory(&dvi, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hRet = AtlGetDllVersion(_T("shell32.dll"), &dvi);

	if(SUCCEEDED(hRet))
	{
		*pdwMajor = dvi.dwMajorVersion;
		*pdwMinor = dvi.dwMinorVersion;
	}
	else if(hRet == E_NOTIMPL)
	{
		// If DllGetVersion is not there, then the DLL is a version
		// previous to the one shipped with IE 4.x
		*pdwMajor = 4;
		*pdwMinor = 0;
		hRet = S_OK;
	}

	return hRet;
}

}; // namespace ATL

#endif // (_ATL_VER < 0x0700) && defined(_ATL_DLL) && !defined(_WIN32_WCE)


// These are always included
#include "atlwinx.h"
#include "atluser.h"
#include "atlgdi.h"

#ifndef _WTL_NO_AUTOMATIC_NAMESPACE
using namespace WTL;
#endif // !_WTL_NO_AUTOMATIC_NAMESPACE

#endif // __ATLAPP_H__
