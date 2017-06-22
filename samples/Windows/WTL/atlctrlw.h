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

#ifndef __ATLCTRLW_H__
#define __ATLCTRLW_H__

#pragma once

#ifdef _WIN32_WCE
	#error atlctrlw.h is not supported on Windows CE
#endif

#ifndef __ATLAPP_H__
	#error atlctrlw.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error atlctrlw.h requires atlctrls.h to be included first
#endif

#if (_WIN32_IE < 0x0400)
	#error atlctrlw.h requires _WIN32_IE >= 0x0400
#endif

// Define _WTL_CMDBAR_VISTA_MENUS as 0 to exclude Vista menus support
#if !defined(_WTL_CMDBAR_VISTA_MENUS) && (WINVER >= 0x0500) && (_WIN32_WINNT >= 0x0501) && (_WIN32_IE >= 0x0501)
  #define _WTL_CMDBAR_VISTA_MENUS 1
#endif

#if _WTL_CMDBAR_VISTA_MENUS
  #if !((_WIN32_WINNT >= 0x0501) && (_WIN32_IE >= 0x0501))
	#error _WTL_CMDBAR_VISTA_MENUS requires (_WIN32_WINNT >= 0x0501) && (_WIN32_IE >= 0x0501)
  #endif
#endif


///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CCommandBarCtrlImpl<T, TBase, TWinTraits>
// CCommandBarCtrl
// CMDICommandBarCtrlImpl<T, TBase, TWinTraits>
// CMDICommandBarCtrl


namespace WTL
{

///////////////////////////////////////////////////////////////////////////////
// Command Bars

// Window Styles:
#define CBRWS_TOP		CCS_TOP
#define CBRWS_BOTTOM		CCS_BOTTOM
#define CBRWS_NORESIZE		CCS_NORESIZE
#define CBRWS_NOPARENTALIGN	CCS_NOPARENTALIGN
#define CBRWS_NODIVIDER		CCS_NODIVIDER

// Extended styles
#define CBR_EX_TRANSPARENT	0x00000001L
#define CBR_EX_SHAREMENU	0x00000002L
#define CBR_EX_ALTFOCUSMODE	0x00000004L
#define CBR_EX_TRACKALWAYS	0x00000008L
#define CBR_EX_NOVISTAMENUS	0x00000010L

// standard command bar styles
#define ATL_SIMPLE_CMDBAR_PANE_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CBRWS_NODIVIDER | CBRWS_NORESIZE | CBRWS_NOPARENTALIGN)

// Messages - support chevrons for frame windows
#define CBRM_GETCMDBAR			(WM_USER + 301) // returns command bar HWND
#define CBRM_GETMENU			(WM_USER + 302) // returns loaded or attached menu
#define CBRM_TRACKPOPUPMENU		(WM_USER + 303) // displays a popup menu

typedef struct tagCBRPOPUPMENU
{
	int cbSize;
	HMENU hMenu;         // popup menu do display
	UINT uFlags;         // TPM_* flags for ::TrackPopupMenuEx
	int x;
	int y;
	LPTPMPARAMS lptpm;   // ptr to TPMPARAMS for ::TrackPopupMenuEx
} CBRPOPUPMENU, *LPCBRPOPUPMENU;

// helper class
template <class T>
class CSimpleStack : public ATL::CSimpleArray< T >
{
public:
	BOOL Push(T t)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - STACK-PUSH (%8.8X) size = %i\n"), t, GetSize());
#endif
		return Add(t);
	}

	T Pop()
	{
		int nLast = GetSize() - 1;
		if(nLast < 0)
			return NULL;   // must be able to convert to NULL
		T t = m_aT[nLast];
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - STACK-POP (%8.8X) size = %i\n"), t, GetSize());
#endif
		if(!RemoveAt(nLast))
			return NULL;
		return t;
	}

	T GetCurrent()
	{
		int nLast = GetSize() - 1;
		if(nLast < 0)
			return NULL;   // must be able to convert to NULL
		return m_aT[nLast];
	}
};


///////////////////////////////////////////////////////////////////////////////
// CCommandBarCtrlBase - base class for the Command Bar implementation

class CCommandBarCtrlBase : public CToolBarCtrl
{
public:
	struct _MsgHookData
	{
		HHOOK hMsgHook;
		DWORD dwUsage;

		_MsgHookData() : hMsgHook(NULL), dwUsage(0)
		{ }
	};

	typedef ATL::CSimpleMap<DWORD, _MsgHookData*>   CMsgHookMap;
	static CMsgHookMap* s_pmapMsgHook;

	static HHOOK s_hCreateHook;
	static bool s_bW2K;  // For animation flag
	static CCommandBarCtrlBase* s_pCurrentBar;
	static bool s_bStaticInit;

	CSimpleStack<HWND> m_stackMenuWnd;
	CSimpleStack<HMENU> m_stackMenuHandle;

	HWND m_hWndHook;
	DWORD m_dwMagic;


	CCommandBarCtrlBase() : m_hWndHook(NULL), m_dwMagic(1314)
	{
		// init static variables
		if(!s_bStaticInit)
		{
			CStaticDataInitCriticalSectionLock lock;
			if(FAILED(lock.Lock()))
			{
				ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CCommandBarCtrlBase::CCommandBarCtrlBase.\n"));
				ATLASSERT(FALSE);
				return;
			}

			if(!s_bStaticInit)
			{
				// Just in case...
				AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
				// Animation on Win2000 only
				s_bW2K = !AtlIsOldWindows();
				// done
				s_bStaticInit = true;
			}

			lock.Unlock();
		}
	}

	bool IsCommandBarBase() const { return m_dwMagic == 1314; }
};

__declspec(selectany) CCommandBarCtrlBase::CMsgHookMap* CCommandBarCtrlBase::s_pmapMsgHook = NULL;
__declspec(selectany) HHOOK CCommandBarCtrlBase::s_hCreateHook = NULL;
__declspec(selectany) CCommandBarCtrlBase* CCommandBarCtrlBase::s_pCurrentBar = NULL;
__declspec(selectany) bool CCommandBarCtrlBase::s_bW2K = false;
__declspec(selectany) bool CCommandBarCtrlBase::s_bStaticInit = false;


///////////////////////////////////////////////////////////////////////////////
// CCommandBarCtrl - ATL implementation of Command Bars

template <class T, class TBase = CCommandBarCtrlBase, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CCommandBarCtrlImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

// Declarations
	struct _MenuItemData	// menu item data
	{
		DWORD dwMagic;
		LPTSTR lpstrText;
		UINT fType;
		UINT fState;
		int iButton;

		_MenuItemData() { dwMagic = 0x1313; }
		bool IsCmdBarMenuItem() { return (dwMagic == 0x1313); }
	};

	struct _ToolBarData	// toolbar resource data
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;
		//WORD aItems[wItemCount]

		WORD* items()
			{ return (WORD*)(this+1); }
	};

// Constants
	enum _CmdBarDrawConstants
	{
		s_kcxGap = 1,
		s_kcxTextMargin = 2,
		s_kcxButtonMargin = 3,
		s_kcyButtonMargin = 3
	};

	enum
	{
		_nMaxMenuItemTextLength = 100,
		_chChevronShortcut = _T('/')
	};

#ifndef DT_HIDEPREFIX
	enum { DT_HIDEPREFIX = 0x00100000 };
#endif // !DT_HIDEPREFIX

// Data members
	HMENU m_hMenu;
	HIMAGELIST m_hImageList;
	ATL::CSimpleValArray<WORD> m_arrCommand;

	DWORD m_dwExtendedStyle;   // Command Bar specific extended styles

	ATL::CContainedWindow m_wndParent;

	bool m_bMenuActive:1;
	bool m_bAttachedMenu:1;
	bool m_bImagesVisible:1;
	bool m_bPopupItem:1;
	bool m_bContextMenu:1;
	bool m_bEscapePressed:1;
	bool m_bSkipMsg:1;
	bool m_bParentActive:1;
	bool m_bFlatMenus:1;
	bool m_bUseKeyboardCues:1;
	bool m_bShowKeyboardCues:1;
	bool m_bAllowKeyboardCues:1;
	bool m_bKeyboardInput:1;
	bool m_bAlphaImages:1;
	bool m_bLayoutRTL:1;
	bool m_bSkipPostDown:1;
	bool m_bVistaMenus:1;

	int m_nPopBtn;
	int m_nNextPopBtn;

	SIZE m_szBitmap;
	SIZE m_szButton;

	COLORREF m_clrMask;
	CFont m_fontMenu;   // used internally, only to measure text

	UINT m_uSysKey;

	HWND m_hWndFocus;   // Alternate focus mode

	int m_cxExtraSpacing;

#if _WTL_CMDBAR_VISTA_MENUS
	ATL::CSimpleValArray<HBITMAP> m_arrVistaBitmap;   // Bitmaps for Vista menus
#endif // _WTL_CMDBAR_VISTA_MENUS

// Constructor/destructor
	CCommandBarCtrlImpl() : 
			m_hMenu(NULL), 
			m_hImageList(NULL), 
			m_wndParent(this, 1), 
			m_bMenuActive(false), 
			m_bAttachedMenu(false), 
			m_nPopBtn(-1), 
			m_nNextPopBtn(-1), 
			m_bPopupItem(false),
			m_bImagesVisible(true),
			m_bSkipMsg(false),
			m_uSysKey(0),
			m_hWndFocus(NULL),
			m_bContextMenu(false),
			m_bEscapePressed(false),
			m_clrMask(RGB(192, 192, 192)),
			m_dwExtendedStyle(CBR_EX_TRANSPARENT | CBR_EX_SHAREMENU | CBR_EX_TRACKALWAYS),
			m_bParentActive(true),
			m_bFlatMenus(false),
			m_bUseKeyboardCues(false),
			m_bShowKeyboardCues(false),
			m_bAllowKeyboardCues(true),
			m_bKeyboardInput(false),
			m_cxExtraSpacing(0),
			m_bAlphaImages(false),
			m_bLayoutRTL(false),
			m_bSkipPostDown(false),
			m_bVistaMenus(false)
	{
		SetImageSize(16, 15);   // default
 	}

	~CCommandBarCtrlImpl()
	{
		if(m_wndParent.IsWindow())
/*scary!*/			m_wndParent.UnsubclassWindow();

		if(m_hMenu != NULL && (m_dwExtendedStyle & CBR_EX_SHAREMENU) == 0)
			::DestroyMenu(m_hMenu);

		if(m_hImageList != NULL)
			::ImageList_Destroy(m_hImageList);
	}

// Attributes
	DWORD GetCommandBarExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetCommandBarExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		return dwPrevStyle;
	}

	CMenuHandle GetMenu() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return m_hMenu;
	}

	COLORREF GetImageMaskColor() const
	{
		return m_clrMask;
	}

	COLORREF SetImageMaskColor(COLORREF clrMask)
	{
		COLORREF clrOld = m_clrMask;
		m_clrMask = clrMask;
		return clrOld;
	}

	bool GetImagesVisible() const
	{
		return m_bImagesVisible;
	}

	bool SetImagesVisible(bool bVisible)
	{
		bool bOld = m_bImagesVisible;
		m_bImagesVisible = bVisible;
		return bOld;
	}

	void GetImageSize(SIZE& size) const
	{
		size = m_szBitmap;
	}

	bool SetImageSize(SIZE& size)
	{
		return SetImageSize(size.cx, size.cy);
	}

	bool SetImageSize(int cx, int cy)
	{
		if(m_hImageList != NULL)
		{
			if(::ImageList_GetImageCount(m_hImageList) == 0)   // empty
			{
				::ImageList_Destroy(m_hImageList);
				m_hImageList = NULL;
			}
			else
			{
				return false;   // can't set, image list exists
			}
		}

		if(cx == 0 || cy == 0)
			return false;

		m_szBitmap.cx = cx;
		m_szBitmap.cy = cy;
		m_szButton.cx = m_szBitmap.cx + 2 * s_kcxButtonMargin;
		m_szButton.cy = m_szBitmap.cy + 2 * s_kcyButtonMargin;

		return true;
	}

	bool GetAlphaImages() const
	{
		return m_bAlphaImages;
	}

	bool SetAlphaImages(bool bAlphaImages)
	{
		if(m_hImageList != NULL)
		{
			if(::ImageList_GetImageCount(m_hImageList) == 0)   // empty
			{
				::ImageList_Destroy(m_hImageList);
				m_hImageList = NULL;
			}
			else
			{
				return false;   // can't set, image list exists
			}
		}

		m_bAlphaImages = bAlphaImages;
		return true;
	}

	HWND GetCmdBar() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, CBRM_GETCMDBAR, 0, 0L);
	}

// Methods
	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		// These styles are required for command bars
		dwStyle |= TBSTYLE_LIST | TBSTYLE_FLAT;
#if (_MSC_VER >= 1300)
		return ATL::CWindowImpl< T, TBase, TWinTraits >::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
#else // !(_MSC_VER >= 1300)
		typedef ATL::CWindowImpl< T, TBase, TWinTraits >   _baseClass;
		return _baseClass::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
#endif // !(_MSC_VER >= 1300)
	}

	BOOL AttachToWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = SubclassWindow(hWnd);
		if(bRet)
		{
			m_bAttachedMenu = true;
			T* pT = static_cast<T*>(this);
			pT->GetSystemSettings();
		}
		return bRet;
	}

	BOOL LoadMenu(ATL::_U_STRINGorID menu)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		if(m_bAttachedMenu)   // doesn't work in this mode
			return FALSE;
		if(menu.m_lpstr == NULL)
			return FALSE;

		HMENU hMenu = ::LoadMenu(ModuleHelper::GetResourceInstance(), menu.m_lpstr);
		if(hMenu == NULL)
			return FALSE;

		return AttachMenu(hMenu);
	}

	BOOL AttachMenu(HMENU hMenu)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hMenu == NULL || ::IsMenu(hMenu));
		if(hMenu != NULL && !::IsMenu(hMenu))
			return FALSE;

#if _WTL_CMDBAR_VISTA_MENUS
		// remove Vista bitmaps if used
		if(m_bVistaMenus && (m_hMenu != NULL))
		{
			T* pT = static_cast<T*>(this);
			pT->_RemoveVistaBitmapsFromMenu();
		}
#endif // _WTL_CMDBAR_VISTA_MENUS

		// destroy old menu, if needed, and set new one
		if(m_hMenu != NULL && (m_dwExtendedStyle & CBR_EX_SHAREMENU) == 0)
			::DestroyMenu(m_hMenu);

		m_hMenu = hMenu;

		if(m_bAttachedMenu)   // Nothing else in this mode
			return TRUE;

		// Build buttons according to menu
		SetRedraw(FALSE);

		// Clear all buttons
		int nCount = GetButtonCount();
		for(int i = 0; i < nCount; i++)
			ATLVERIFY(DeleteButton(0) != FALSE);

		// Add buttons for each menu item
		if(m_hMenu != NULL)
		{
			int nItems = ::GetMenuItemCount(m_hMenu);

			T* pT = static_cast<T*>(this);
			pT;   // avoid level 4 warning
			TCHAR szString[pT->_nMaxMenuItemTextLength];
			for(int i = 0; i < nItems; i++)
			{
				CMenuItemInfo mii;
				mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_SUBMENU;
				mii.fType = MFT_STRING;
				mii.dwTypeData = szString;
				mii.cch = pT->_nMaxMenuItemTextLength;
				BOOL bRet = ::GetMenuItemInfo(m_hMenu, i, TRUE, &mii);
				ATLASSERT(bRet);
				// If we have more than the buffer, we assume we have bitmaps bits
				if(lstrlen(szString) > pT->_nMaxMenuItemTextLength - 1)
				{
					mii.fType = MFT_BITMAP;
					::SetMenuItemInfo(m_hMenu, i, TRUE, &mii);
					szString[0] = 0;
				}

				// NOTE: Command Bar currently supports only drop-down menu items
				ATLASSERT(mii.hSubMenu != NULL);

				TBBUTTON btn = { 0 };
				btn.iBitmap = 0;
				btn.idCommand = i;
				btn.fsState = (BYTE)(((mii.fState & MFS_DISABLED) == 0) ? TBSTATE_ENABLED : 0);
				btn.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN;
				btn.dwData = 0;
				btn.iString = 0;

				bRet = InsertButton(-1, &btn);
				ATLASSERT(bRet);

				TBBUTTONINFO bi = { 0 };
				bi.cbSize = sizeof(TBBUTTONINFO);
				bi.dwMask = TBIF_TEXT;
				bi.pszText = szString;

				bRet = SetButtonInfo(i, &bi);
				ATLASSERT(bRet);
			}
		}

		SetRedraw(TRUE);
		Invalidate();
		UpdateWindow();

		return TRUE;
	}

	BOOL LoadImages(ATL::_U_STRINGorID image)
	{
		return _LoadImagesHelper(image, false);
	}

	BOOL LoadMappedImages(UINT nIDImage, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
	{
		return _LoadImagesHelper(nIDImage, true, nFlags , lpColorMap, nMapSize);
	}

	BOOL _LoadImagesHelper(ATL::_U_STRINGorID image, bool bMapped, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HINSTANCE hInstance = ModuleHelper::GetResourceInstance();

		HRSRC hRsrc = ::FindResource(hInstance, image.m_lpstr, (LPTSTR)RT_TOOLBAR);
		if(hRsrc == NULL)
			return FALSE;

		HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
		if(hGlobal == NULL)
			return FALSE;

		_ToolBarData* pData = (_ToolBarData*)::LockResource(hGlobal);
		if(pData == NULL)
			return FALSE;
		ATLASSERT(pData->wVersion == 1);

		WORD* pItems = pData->items();
		int nItems = pData->wItemCount;

		// Set internal data
		SetImageSize(pData->wWidth, pData->wHeight);

		// Create image list if needed
		if(m_hImageList == NULL)
		{
			// Check if the bitmap is 32-bit (alpha channel) bitmap (valid for Windows XP only)
			T* pT = static_cast<T*>(this);
			m_bAlphaImages = AtlIsAlphaBitmapResource(image);

			if(!pT->CreateInternalImageList(pData->wItemCount))
				return FALSE;
		}

#if _WTL_CMDBAR_VISTA_MENUS
		int nOldImageCount = ::ImageList_GetImageCount(m_hImageList);
#endif // _WTL_CMDBAR_VISTA_MENUS

		// Add bitmap to our image list
		CBitmap bmp;
		if(bMapped)
		{
			ATLASSERT(HIWORD(PtrToUlong(image.m_lpstr)) == 0);   // if mapped, must be a numeric ID
			int nIDImage = (int)(short)LOWORD(PtrToUlong(image.m_lpstr));
			bmp.LoadMappedBitmap(nIDImage, (WORD)nFlags, lpColorMap, nMapSize);
		}
		else
		{
			if(m_bAlphaImages)
				bmp = (HBITMAP)::LoadImage(ModuleHelper::GetResourceInstance(), image.m_lpstr, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
			else
				bmp.LoadBitmap(image.m_lpstr);
		}
		ATLASSERT(bmp.m_hBitmap != NULL);
		if(bmp.m_hBitmap == NULL)
			return FALSE;
		if(::ImageList_AddMasked(m_hImageList, bmp, m_clrMask) == -1)
			return FALSE;

		// Fill the array with command IDs
		for(int i = 0; i < nItems; i++)
		{
			if(pItems[i] != 0)
				m_arrCommand.Add(pItems[i]);
		}

		int nImageCount = ::ImageList_GetImageCount(m_hImageList);
		ATLASSERT(nImageCount == m_arrCommand.GetSize());
		if(nImageCount != m_arrCommand.GetSize())
			return FALSE;

#if _WTL_CMDBAR_VISTA_MENUS
		if(RunTimeHelper::IsVista())
		{
			T* pT = static_cast<T*>(this);
			pT->_AddVistaBitmapsFromImageList(nOldImageCount, nImageCount - nOldImageCount);
			ATLASSERT(nImageCount == m_arrVistaBitmap.GetSize());
		}
#endif // _WTL_CMDBAR_VISTA_MENUS

		return TRUE;
	}

	BOOL AddBitmap(ATL::_U_STRINGorID bitmap, int nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CBitmap bmp;
		bmp.LoadBitmap(bitmap.m_lpstr);
		if(bmp.m_hBitmap == NULL)
			return FALSE;
		return AddBitmap(bmp, nCommandID);
	}

	BOOL AddBitmap(HBITMAP hBitmap, UINT nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		// Create image list if it doesn't exist
		if(m_hImageList == NULL)
		{
			if(!pT->CreateInternalImageList(1))
				return FALSE;
		}
		// check bitmap size
		CBitmapHandle bmp = hBitmap;
		SIZE size = { 0, 0 };
		bmp.GetSize(size);
		if(size.cx != m_szBitmap.cx || size.cy != m_szBitmap.cy)
		{
			ATLASSERT(FALSE);   // must match size!
			return FALSE;
		}
		// add bitmap
		int nRet = ::ImageList_AddMasked(m_hImageList, hBitmap, m_clrMask);
		if(nRet == -1)
			return FALSE;
		BOOL bRet = m_arrCommand.Add((WORD)nCommandID);
		ATLASSERT(::ImageList_GetImageCount(m_hImageList) == m_arrCommand.GetSize());
#if _WTL_CMDBAR_VISTA_MENUS
		if(RunTimeHelper::IsVista())
		{
			pT->_AddVistaBitmapFromImageList(m_arrCommand.GetSize() - 1);
			ATLASSERT(m_arrVistaBitmap.GetSize() == m_arrCommand.GetSize());
		}
#endif // _WTL_CMDBAR_VISTA_MENUS
		return bRet;
	}

	BOOL AddIcon(ATL::_U_STRINGorID icon, UINT nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HICON hIcon = ::LoadIcon(ModuleHelper::GetResourceInstance(), icon.m_lpstr);
		if(hIcon == NULL)
			return FALSE;
		return AddIcon(hIcon, nCommandID);
	}

	BOOL AddIcon(HICON hIcon, UINT nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		// create image list if it doesn't exist
		if(m_hImageList == NULL)
		{
			if(!pT->CreateInternalImageList(1))
				return FALSE;
		}

		int nRet = ::ImageList_AddIcon(m_hImageList, hIcon);
		if(nRet == -1)
			return FALSE;
		BOOL bRet = m_arrCommand.Add((WORD)nCommandID);
		ATLASSERT(::ImageList_GetImageCount(m_hImageList) == m_arrCommand.GetSize());
#if _WTL_CMDBAR_VISTA_MENUS
		if(RunTimeHelper::IsVista())
		{
			pT->_AddVistaBitmapFromImageList(m_arrCommand.GetSize() - 1);
			ATLASSERT(m_arrVistaBitmap.GetSize() == m_arrCommand.GetSize());
		}
#endif // _WTL_CMDBAR_VISTA_MENUS
		return bRet;
	}

	BOOL ReplaceBitmap(ATL::_U_STRINGorID bitmap, int nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CBitmap bmp;
		bmp.LoadBitmap(bitmap.m_lpstr);
		if(bmp.m_hBitmap == NULL)
			return FALSE;
		return ReplaceBitmap(bmp, nCommandID);
	}

	BOOL ReplaceBitmap(HBITMAP hBitmap, UINT nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		BOOL bRet = FALSE;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			if(m_arrCommand[i] == nCommandID)
			{
				bRet = ::ImageList_Remove(m_hImageList, i);
				if(bRet)
				{
					m_arrCommand.RemoveAt(i);
#if _WTL_CMDBAR_VISTA_MENUS
					if(RunTimeHelper::IsVista())
					{
						if(m_arrVistaBitmap[i] != NULL)
							::DeleteObject(m_arrVistaBitmap[i]);
						m_arrVistaBitmap.RemoveAt(i);
					}
#endif // _WTL_CMDBAR_VISTA_MENUS
				}
				break;
			}
		}
		if(bRet)
			bRet = AddBitmap(hBitmap, nCommandID);
		return bRet;
	}

	BOOL ReplaceIcon(ATL::_U_STRINGorID icon, UINT nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HICON hIcon = ::LoadIcon(ModuleHelper::GetResourceInstance(), icon.m_lpstr);
		if(hIcon == NULL)
			return FALSE;
		return ReplaceIcon(hIcon, nCommandID);
	}

	BOOL ReplaceIcon(HICON hIcon, UINT nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		BOOL bRet = FALSE;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			if(m_arrCommand[i] == nCommandID)
			{
				bRet = (::ImageList_ReplaceIcon(m_hImageList, i, hIcon) != -1);
#if _WTL_CMDBAR_VISTA_MENUS
				if(RunTimeHelper::IsVista() && bRet != FALSE)
				{
					T* pT = static_cast<T*>(this);
					pT->_ReplaceVistaBitmapFromImageList(i);
				}
#endif // _WTL_CMDBAR_VISTA_MENUS
				break;
			}
		}
		return bRet;
	}

	BOOL RemoveImage(int nCommandID)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		BOOL bRet = FALSE;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			if(m_arrCommand[i] == nCommandID)
			{
				bRet = ::ImageList_Remove(m_hImageList, i);
				if(bRet)
				{
					m_arrCommand.RemoveAt(i);
#if _WTL_CMDBAR_VISTA_MENUS
					if(RunTimeHelper::IsVista())
					{
						if(m_arrVistaBitmap[i] != NULL)
							::DeleteObject(m_arrVistaBitmap[i]);
						m_arrVistaBitmap.RemoveAt(i);
					}
#endif // _WTL_CMDBAR_VISTA_MENUS
				}
				break;
			}
		}
		return bRet;
	}

	BOOL RemoveAllImages()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Removing all images\n"));
		BOOL bRet = ::ImageList_RemoveAll(m_hImageList);
		if(bRet)
		{
			m_arrCommand.RemoveAll();
#if _WTL_CMDBAR_VISTA_MENUS
			for(int i = 0; i < m_arrVistaBitmap.GetSize(); i++)
			{
				if(m_arrVistaBitmap[i] != NULL)
					::DeleteObject(m_arrVistaBitmap[i]);
			}
			m_arrVistaBitmap.RemoveAll();
#endif // _WTL_CMDBAR_VISTA_MENUS
		}
		return bRet;
	}

	BOOL TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, LPTPMPARAMS lpParams = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsMenu(hMenu));
		if(!::IsMenu(hMenu))
			return FALSE;
		m_bContextMenu = true;
		if(m_bUseKeyboardCues)
			m_bShowKeyboardCues = m_bKeyboardInput;
		T* pT = static_cast<T*>(this);
		return pT->DoTrackPopupMenu(hMenu, uFlags, x, y, lpParams);
	}

	BOOL SetMDIClient(HWND /*hWndMDIClient*/)
	{
		// Use CMDICommandBarCtrl for MDI support
		ATLASSERT(FALSE);
		return FALSE;
	}

// Message map and handlers
	BEGIN_MSG_MAP(CCommandBarCtrlImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		MESSAGE_HANDLER(GetAutoPopupMessage(), OnInternalAutoPopup)
		MESSAGE_HANDLER(GetGetBarMessage(), OnInternalGetBar)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)

		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYUP, OnSysKeyUp)
		MESSAGE_HANDLER(WM_SYSCHAR, OnSysChar)
// public API handlers - these stay to support chevrons in atlframe.h
		MESSAGE_HANDLER(CBRM_GETMENU, OnAPIGetMenu)
		MESSAGE_HANDLER(CBRM_TRACKPOPUPMENU, OnAPITrackPopupMenu)
		MESSAGE_HANDLER(CBRM_GETCMDBAR, OnAPIGetCmdBar)

		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)

		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
	ALT_MSG_MAP(1)   // Parent window messages
		NOTIFY_CODE_HANDLER(TBN_HOTITEMCHANGE, OnParentHotItemChange)
		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnParentDropDown)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnParentInitMenuPopup)
		MESSAGE_HANDLER(GetGetBarMessage(), OnParentInternalGetBar)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnParentSysCommand)
		MESSAGE_HANDLER(CBRM_GETMENU, OnParentAPIGetMenu)
		MESSAGE_HANDLER(WM_MENUCHAR, OnParentMenuChar)
		MESSAGE_HANDLER(CBRM_TRACKPOPUPMENU, OnParentAPITrackPopupMenu)
		MESSAGE_HANDLER(CBRM_GETCMDBAR, OnParentAPIGetCmdBar)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnParentSettingChange)

		MESSAGE_HANDLER(WM_DRAWITEM, OnParentDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnParentMeasureItem)

		MESSAGE_HANDLER(WM_ACTIVATE, OnParentActivate)
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnParentCustomDraw)
	ALT_MSG_MAP(2)   // MDI client window messages
		// Use CMDICommandBarCtrl for MDI support
	ALT_MSG_MAP(3)   // Message hook messages
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnHookMouseMove)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnHookSysKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYUP, OnHookSysKeyUp)
		MESSAGE_HANDLER(WM_SYSCHAR, OnHookSysChar)
		MESSAGE_HANDLER(WM_KEYDOWN, OnHookKeyDown)
		MESSAGE_HANDLER(WM_NEXTMENU, OnHookNextMenu)
		MESSAGE_HANDLER(WM_CHAR, OnHookChar)
	END_MSG_MAP()

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		if(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
			m_bKeyboardInput = false;
		else if(pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
			m_bKeyboardInput = true;
		LRESULT lRet = 0;
		ProcessWindowMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam, lRet, 3);
		return lRet;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Let the toolbar initialize itself
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		// get and use system settings
		T* pT = static_cast<T*>(this);
		pT->GetSystemSettings();
		// Parent init
		ATL::CWindow wndParent = GetParent();
		ATL::CWindow wndTopLevelParent = wndParent.GetTopLevelParent();
		m_wndParent.SubclassWindow(wndTopLevelParent);
		// Toolbar Init
		SetButtonStructSize();
		SetImageList(NULL);

		// Create message hook if needed
		CWindowCreateCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CCommandBarCtrlImpl::OnCreate.\n"));
			ATLASSERT(FALSE);
			return -1;
		}

		if(s_pmapMsgHook == NULL)
		{
			ATLTRY(s_pmapMsgHook = new CMsgHookMap);
			ATLASSERT(s_pmapMsgHook != NULL);
		}

		if(s_pmapMsgHook != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			_MsgHookData* pData = s_pmapMsgHook->Lookup(dwThreadID);
			if(pData == NULL)
			{
				ATLTRY(pData = new _MsgHookData);
				ATLASSERT(pData != NULL);
				HHOOK hMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, MessageHookProc, ModuleHelper::GetModuleInstance(), dwThreadID);
				ATLASSERT(hMsgHook != NULL);
				if(pData != NULL && hMsgHook != NULL)
				{
					pData->hMsgHook = hMsgHook;
					pData->dwUsage = 1;
					BOOL bRet = s_pmapMsgHook->Add(dwThreadID, pData);
					bRet;
					ATLASSERT(bRet);
				}
			}
			else
			{
				(pData->dwUsage)++;
			}
		}
		lock.Unlock();

		// Get layout
#if (WINVER >= 0x0500)
		m_bLayoutRTL = ((GetExStyle() & WS_EX_LAYOUTRTL) != 0);
#endif // (WINVER >= 0x0500)

		return lRet;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

#if _WTL_CMDBAR_VISTA_MENUS
		if(m_bVistaMenus && (m_hMenu != NULL))
		{
			T* pT = static_cast<T*>(this);
			pT->_RemoveVistaBitmapsFromMenu();
		}

		for(int i = 0; i < m_arrVistaBitmap.GetSize(); i++)
		{
			if(m_arrVistaBitmap[i] != NULL)
				::DeleteObject(m_arrVistaBitmap[i]);
		}
#endif // _WTL_CMDBAR_VISTA_MENUS

		if(m_bAttachedMenu)   // nothing to do in this mode
			return lRet;

		CWindowCreateCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CCommandBarCtrlImpl::OnDestroy.\n"));
			ATLASSERT(FALSE);
			return lRet;
		}

		if(s_pmapMsgHook != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			_MsgHookData* pData = s_pmapMsgHook->Lookup(dwThreadID);
			if(pData != NULL)
			{
				(pData->dwUsage)--;
				if(pData->dwUsage == 0)
				{
					BOOL bRet = ::UnhookWindowsHookEx(pData->hMsgHook);
					ATLASSERT(bRet);
					bRet = s_pmapMsgHook->Remove(dwThreadID);
					ATLASSERT(bRet);
					if(bRet)
						delete pData;
				}

				if(s_pmapMsgHook->GetSize() == 0)
				{
					delete s_pmapMsgHook;
					s_pmapMsgHook = NULL;
				}
			}
		}

		lock.Unlock();

		return lRet;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnKeyDown\n"));
#endif
		bHandled = FALSE;
		// Simulate Alt+Space for the parent
		if(wParam == VK_SPACE)
		{
			m_wndParent.PostMessage(WM_SYSKEYDOWN, wParam, lParam | (1 << 29));
			bHandled = TRUE;
		}
#if (_WIN32_IE >= 0x0500)
		else if(wParam == VK_LEFT || wParam == VK_RIGHT)
		{
			WPARAM wpNext = m_bLayoutRTL ? VK_LEFT : VK_RIGHT;

			if(!m_bMenuActive)
			{
				T* pT = static_cast<T*>(this);
				int nBtn = GetHotItem();
				int nNextBtn = (wParam == wpNext) ? pT->GetNextMenuItem(nBtn) : pT->GetPreviousMenuItem(nBtn);
				if(nNextBtn == -2)
				{
					SetHotItem(-1);
					if(pT->DisplayChevronMenu())
						bHandled = TRUE;
				}
			}
		}
#endif // (_WIN32_IE >= 0x0500)
		return 0;
	}

	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnKeyUp\n"));
#endif
		if(wParam != VK_SPACE)
			bHandled = FALSE;
		return 0;
	}

	LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnChar\n"));
#endif
		if(wParam != VK_SPACE)
			bHandled = FALSE;
		else
			return 0;
		// Security
		if(!m_wndParent.IsWindowEnabled() || ::GetFocus() != m_hWnd)
			return 0;

		// Handle mnemonic press when we have focus
		int nBtn = 0;
		if(wParam != VK_RETURN && !MapAccelerator((TCHAR)LOWORD(wParam), nBtn))
		{
#if (_WIN32_IE >= 0x0500)
			if((TCHAR)LOWORD(wParam) != _chChevronShortcut)
#endif // (_WIN32_IE >= 0x0500)
				::MessageBeep(0);
		}
		else
		{
#if (_WIN32_IE >= 0x0500)
			RECT rcClient = { 0 };
			GetClientRect(&rcClient);
			RECT rcBtn = { 0 };
			GetItemRect(nBtn, &rcBtn);
			TBBUTTON tbb = { 0 };
			GetButton(nBtn, &tbb);
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0 && rcBtn.right <= rcClient.right)
			{
#endif // (_WIN32_IE >= 0x0500)
				PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
				if(wParam != VK_RETURN)
					SetHotItem(nBtn);
#if (_WIN32_IE >= 0x0500)
			}
			else
			{
				::MessageBeep(0);
				bHandled = TRUE;
			}
#endif // (_WIN32_IE >= 0x0500)
		}
		return 0;
	}

	LRESULT OnSysKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnSysKeyDown\n"));
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnSysKeyUp\n"));
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnSysChar\n"));
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_bAttachedMenu || (m_dwExtendedStyle & CBR_EX_TRANSPARENT))
		{
			bHandled = FALSE;
			return 0;
		}

		CDCHandle dc = (HDC)wParam;
		RECT rect = { 0 };
		GetClientRect(&rect);
		dc.FillRect(&rect, COLOR_MENU);

		return 1;   // don't do the default erase
	}

	LRESULT OnInitMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		int nIndex = GetHotItem();
		SendMessage(WM_MENUSELECT, MAKEWPARAM(nIndex, MF_POPUP|MF_HILITE), (LPARAM)m_hMenu);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if((BOOL)HIWORD(lParam))   // System menu, do nothing
		{
			bHandled = FALSE;
			return 1;
		}

		if(!(m_bAttachedMenu || m_bMenuActive))   // Not attached or ours, do nothing
		{
			bHandled = FALSE;
			return 1;
		}

#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnInitMenuPopup\n"));
#endif
		// forward to the parent or subclassed window, so it can handle update UI
		LRESULT lRet = 0;
		if(m_bAttachedMenu)
			lRet = DefWindowProc(uMsg, wParam, (lParam || m_bContextMenu) ? lParam : GetHotItem());
		else
			lRet = m_wndParent.DefWindowProc(uMsg, wParam, (lParam || m_bContextMenu) ? lParam : GetHotItem());

#if _WTL_CMDBAR_VISTA_MENUS
		// If Vista menus are active, just set bitmaps and return
		if(m_bVistaMenus)
		{
			CMenuHandle menu = (HMENU)wParam;
			ATLASSERT(menu.m_hMenu != NULL);

			for(int i = 0; i < menu.GetMenuItemCount(); i++)
			{
				WORD nID = (WORD)menu.GetMenuItemID(i);
				int nIndex = m_arrCommand.Find(nID);

				CMenuItemInfo mii;
				mii.fMask = MIIM_BITMAP;
				mii.hbmpItem = (m_bImagesVisible && (nIndex != -1)) ? m_arrVistaBitmap[nIndex] : NULL;
				menu.SetMenuItemInfo(i, TRUE, &mii);
			}

			return lRet;
		}
#endif // _WTL_CMDBAR_VISTA_MENUS

		// Convert menu items to ownerdraw, add our data
		if(m_bImagesVisible)
		{
			CMenuHandle menuPopup = (HMENU)wParam;
			ATLASSERT(menuPopup.m_hMenu != NULL);

			T* pT = static_cast<T*>(this);
			pT;   // avoid level 4 warning
			TCHAR szString[pT->_nMaxMenuItemTextLength];
			BOOL bRet = FALSE;
			for(int i = 0; i < menuPopup.GetMenuItemCount(); i++)
			{
				CMenuItemInfo mii;
				mii.cch = pT->_nMaxMenuItemTextLength;
				mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
				mii.dwTypeData = szString;
				bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
				ATLASSERT(bRet);

				if(!(mii.fType & MFT_OWNERDRAW))   // Not already an ownerdraw item
				{
					mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
					_MenuItemData* pMI = NULL;
					ATLTRY(pMI = new _MenuItemData);
					ATLASSERT(pMI != NULL);
					if(pMI != NULL)
					{
						pMI->fType = mii.fType;
						pMI->fState = mii.fState;
						mii.fType |= MFT_OWNERDRAW;
						pMI->iButton = -1;
						for(int j = 0; j < m_arrCommand.GetSize(); j++)
						{
							if(m_arrCommand[j] == mii.wID)
							{
								pMI->iButton = j;
								break;
							}
						}
						int cchLen = lstrlen(szString) + 1;
						pMI->lpstrText = NULL;
						ATLTRY(pMI->lpstrText = new TCHAR[cchLen]);
						ATLASSERT(pMI->lpstrText != NULL);
						if(pMI->lpstrText != NULL)
							SecureHelper::strcpy_x(pMI->lpstrText, cchLen, szString);
						mii.dwItemData = (ULONG_PTR)pMI;
						bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
						ATLASSERT(bRet);
					}
				}
			}

			// Add it to the list
			m_stackMenuHandle.Push(menuPopup.m_hMenu);
		}

		return lRet;
	}

	LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(!m_bAttachedMenu)   // Not attached, do nothing, forward to parent
		{
			m_bPopupItem = (lParam != NULL) && ((HMENU)lParam != m_hMenu) && (HIWORD(wParam) & MF_POPUP);
			if(m_wndParent.IsWindow())
				m_wndParent.SendMessage(uMsg, wParam, lParam);
			bHandled = FALSE;
			return 1;
		}

		// Check if a menu is closing, do a cleanup
		if(HIWORD(wParam) == 0xFFFF && lParam == NULL)   // Menu closing
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnMenuSelect - CLOSING!!!!\n"));
#endif
			ATLASSERT(m_stackMenuWnd.GetSize() == 0);
			// Restore the menu items to the previous state for all menus that were converted
			if(m_bImagesVisible)
			{
				HMENU hMenu = NULL;
				while((hMenu = m_stackMenuHandle.Pop()) != NULL)
				{
					CMenuHandle menuPopup = hMenu;
					ATLASSERT(menuPopup.m_hMenu != NULL);
					// Restore state and delete menu item data
					BOOL bRet = FALSE;
					for(int i = 0; i < menuPopup.GetMenuItemCount(); i++)
					{
						CMenuItemInfo mii;
						mii.fMask = MIIM_DATA | MIIM_TYPE;
						bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
						ATLASSERT(bRet);

						_MenuItemData* pMI = (_MenuItemData*)mii.dwItemData;
						if(pMI != NULL && pMI->IsCmdBarMenuItem())
						{
							mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
							mii.fType = pMI->fType;
							mii.dwTypeData = pMI->lpstrText;
							mii.cch = lstrlen(pMI->lpstrText);
							mii.dwItemData = NULL;

							bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
							ATLASSERT(bRet);

							delete [] pMI->lpstrText;
							pMI->dwMagic = 0x6666;
							delete pMI;
						}
					}
				}
			}
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnInternalAutoPopup(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		int nIndex = (int)wParam;
		T* pT = static_cast<T*>(this);
		pT->DoPopupMenu(nIndex, false);
		return 0;
	}

	LRESULT OnInternalGetBar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// Let's make sure we're not embedded in another process
		if((LPVOID)wParam != NULL)
			*((DWORD*)wParam) = GetCurrentProcessId();
		if(IsWindowVisible())
			return (LRESULT)static_cast<CCommandBarCtrlBase*>(this);
		else
			return NULL;
	}

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
#ifndef SPI_GETKEYBOARDCUES
		const UINT SPI_SETKEYBOARDCUES = 0x100B;
#endif // !SPI_GETKEYBOARDCUES
#ifndef SPI_GETFLATMENU
		const UINT SPI_SETFLATMENU = 0x1023;
#endif // !SPI_GETFLATMENU

		if(wParam == SPI_SETNONCLIENTMETRICS || wParam == SPI_SETKEYBOARDCUES || wParam == SPI_SETFLATMENU)
		{
			T* pT = static_cast<T*>(this);
			pT->GetSystemSettings();
		}

		return 0;
	}

	LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		LPWINDOWPOS lpWP = (LPWINDOWPOS)lParam;
		int cyMin = ::GetSystemMetrics(SM_CYMENU);
		if(lpWP->cy < cyMin)
		lpWP->cy = cyMin;

		return lRet;
	}

	LRESULT OnMenuChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - OnMenuChar\n"));
#endif
		bHandled = TRUE;
		T* pT = static_cast<T*>(this);

		LRESULT lRet;
		if(m_bMenuActive && LOWORD(wParam) != 0x0D)
			lRet = 0;
		else
			lRet = MAKELRESULT(1, 1);

		if(m_bMenuActive && HIWORD(wParam) == MF_POPUP)
		{
			// Convert character to lower/uppercase and possibly Unicode, using current keyboard layout
			TCHAR ch = (TCHAR)LOWORD(wParam);
			CMenuHandle menu = (HMENU)lParam;
			int nCount = ::GetMenuItemCount(menu);
			int nRetCode = MNC_EXECUTE;
			BOOL bRet = FALSE;
			TCHAR szString[pT->_nMaxMenuItemTextLength];
			WORD wMnem = 0;
			bool bFound = false;
			for(int i = 0; i < nCount; i++)
			{
				CMenuItemInfo mii;
				mii.cch = pT->_nMaxMenuItemTextLength;
				mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
				mii.dwTypeData = szString;
				bRet = menu.GetMenuItemInfo(i, TRUE, &mii);
				if(!bRet || (mii.fType & MFT_SEPARATOR))
					continue;
				_MenuItemData* pmd = (_MenuItemData*)mii.dwItemData;
				if(pmd != NULL && pmd->IsCmdBarMenuItem())
				{
					LPTSTR p = pmd->lpstrText;

					if(p != NULL)
					{
						while(*p && *p != _T('&'))
							p = ::CharNext(p);
						if(p != NULL && *p)
						{
							DWORD dwP = MAKELONG(*(++p), 0);
							DWORD dwC = MAKELONG(ch, 0);
							if(::CharLower((LPTSTR)ULongToPtr(dwP)) == ::CharLower((LPTSTR)ULongToPtr(dwC)))
							{
								if(!bFound)
								{
									wMnem = (WORD)i;
									bFound = true;
								}
								else
								{
									nRetCode = MNC_SELECT;
									break;
								}
							}
						}
					}
				}
			}
			if(bFound)
			{
				if(nRetCode == MNC_EXECUTE)
				{
					PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
					pT->GiveFocusBack();
				}
				bHandled = TRUE;
				lRet = MAKELRESULT(wMnem, nRetCode);
			}
		} 
		else if(!m_bMenuActive)
		{
			int nBtn = 0;
			if(!MapAccelerator((TCHAR)LOWORD(wParam), nBtn))
			{
				bHandled = FALSE;
				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
				pT->GiveFocusBack();

#if (_WIN32_IE >= 0x0500)
				// check if we should display chevron menu
				if((TCHAR)LOWORD(wParam) == pT->_chChevronShortcut)
				{
					if(pT->DisplayChevronMenu())
						bHandled = TRUE;
				}
#endif // (_WIN32_IE >= 0x0500)
			}
			else if(m_wndParent.IsWindowEnabled())
			{
#if (_WIN32_IE >= 0x0500)
				RECT rcClient = { 0 };
				GetClientRect(&rcClient);
				RECT rcBtn = { 0 };
				GetItemRect(nBtn, &rcBtn);
				TBBUTTON tbb = { 0 };
				GetButton(nBtn, &tbb);
				if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0 && rcBtn.right <= rcClient.right)
				{
#endif // (_WIN32_IE >= 0x0500)
					if(m_bUseKeyboardCues && !m_bShowKeyboardCues)
					{
						m_bAllowKeyboardCues = true;
						ShowKeyboardCues(true);
					}
					pT->TakeFocus();
					PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
					SetHotItem(nBtn);
#if (_WIN32_IE >= 0x0500)
				}
				else
				{
					::MessageBeep(0);
				}
#endif // (_WIN32_IE >= 0x0500)
			}
		}

		return lRet;
	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		_MenuItemData* pmd = (_MenuItemData*)lpDrawItemStruct->itemData;
		if(lpDrawItemStruct->CtlType == ODT_MENU && pmd != NULL && pmd->IsCmdBarMenuItem())
		{
			T* pT = static_cast<T*>(this);
			pT->DrawItem(lpDrawItemStruct);
		}
		else
		{
			bHandled = FALSE;
		}
		return (LRESULT)TRUE;
	}

	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPMEASUREITEMSTRUCT lpMeasureItemStruct = (LPMEASUREITEMSTRUCT)lParam;
		_MenuItemData* pmd = (_MenuItemData*)lpMeasureItemStruct->itemData;
		if(lpMeasureItemStruct->CtlType == ODT_MENU && pmd != NULL && pmd->IsCmdBarMenuItem())
		{
			T* pT = static_cast<T*>(this);
			pT->MeasureItem(lpMeasureItemStruct);
		}
		else
		{
			bHandled = FALSE;
		}
		return (LRESULT)TRUE;
	}

// API message handlers
	LRESULT OnAPIGetMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return (LRESULT)m_hMenu;
	}

	LRESULT OnAPITrackPopupMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if(lParam == NULL)
			return FALSE;
		LPCBRPOPUPMENU lpCBRPopupMenu = (LPCBRPOPUPMENU)lParam;
		if(lpCBRPopupMenu->cbSize != sizeof(CBRPOPUPMENU))
			return FALSE;

		T* pT = static_cast<T*>(this);
		return pT->TrackPopupMenu(lpCBRPopupMenu->hMenu, lpCBRPopupMenu->uFlags, lpCBRPopupMenu->x, lpCBRPopupMenu->y, lpCBRPopupMenu->lptpm);
	}

	LRESULT OnAPIGetCmdBar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return (LRESULT)m_hWnd;
	}

// Parent window message handlers
	LRESULT OnParentHotItemChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		LPNMTBHOTITEM lpNMHT = (LPNMTBHOTITEM)pnmh;

		// Check if this comes from us
		if(pnmh->hwndFrom != m_hWnd)
		{
			bHandled = FALSE;
			return 0;
		}

		bool bBlockTracking = false;
		if((m_dwExtendedStyle & CBR_EX_TRACKALWAYS) == 0)
		{
			DWORD dwProcessID;
			::GetWindowThreadProcessId(::GetActiveWindow(), &dwProcessID);
			bBlockTracking = (::GetCurrentProcessId() != dwProcessID);
		}

		if((!m_wndParent.IsWindowEnabled() || bBlockTracking) && (lpNMHT->dwFlags & HICF_MOUSE))
		{
			return 1;
		}
		else
		{
#ifndef HICF_LMOUSE
			const DWORD HICF_LMOUSE = 0x00000080;   // left mouse button selected
#endif
			bHandled = FALSE;

			// Send WM_MENUSELECT to the app if it needs to display a status text
			if(!(lpNMHT->dwFlags & HICF_MOUSE)
				&& !(lpNMHT->dwFlags & HICF_ACCELERATOR)
				&& !(lpNMHT->dwFlags & HICF_LMOUSE))
			{
				if(lpNMHT->dwFlags & HICF_ENTERING)
					m_wndParent.SendMessage(WM_MENUSELECT, 0, (LPARAM)m_hMenu);
				if(lpNMHT->dwFlags & HICF_LEAVING)
					m_wndParent.SendMessage(WM_MENUSELECT, MAKEWPARAM(0, 0xFFFF), NULL);
			}

			return 0;
		}
	}

	LRESULT OnParentDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		// Check if this comes from us
		if(pnmh->hwndFrom != m_hWnd)
		{
			bHandled = FALSE;
			return 1;
		}

		T* pT = static_cast<T*>(this);
		if(::GetFocus() != m_hWnd)
			pT->TakeFocus();
		LPNMTOOLBAR pNMToolBar = (LPNMTOOLBAR)pnmh;
		int nIndex = CommandToIndex(pNMToolBar->iItem);
		m_bContextMenu = false;
		m_bEscapePressed = false;
		pT->DoPopupMenu(nIndex, true);

		return TBDDRET_DEFAULT;
	}

	LRESULT OnParentInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnInitMenuPopup(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentInternalGetBar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnInternalGetBar(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		if((m_uSysKey == VK_MENU 
			|| (m_uSysKey == VK_F10 && !(::GetKeyState(VK_SHIFT) & 0x80))
			|| m_uSysKey == VK_SPACE) 
			&& wParam == SC_KEYMENU)
		{
			T* pT = static_cast<T*>(this);
			if(::GetFocus() == m_hWnd)
			{
				pT->GiveFocusBack();   // exit menu "loop"
				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
			}
			else if(m_uSysKey != VK_SPACE && !m_bSkipMsg)
			{
				if(m_bUseKeyboardCues && !m_bShowKeyboardCues && m_bAllowKeyboardCues)
					ShowKeyboardCues(true);

				pT->TakeFocus();      // enter menu "loop"
				bHandled = TRUE;
			}
			else if(m_uSysKey != VK_SPACE)
			{
				bHandled = TRUE;
			}
		}
		m_bSkipMsg = false;
		return 0;
	}

	LRESULT OnParentAPIGetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnAPIGetMenu(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentMenuChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnMenuChar(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentAPITrackPopupMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnAPITrackPopupMenu(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentAPIGetCmdBar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnAPIGetCmdBar(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		OnSettingChange(uMsg, wParam, lParam, bHandled);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnParentDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnDrawItem(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return OnMeasureItem(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnParentActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_bParentActive = (LOWORD(wParam) != WA_INACTIVE);
		if(!m_bParentActive && m_bUseKeyboardCues && m_bShowKeyboardCues)
		{
			ShowKeyboardCues(false);   // this will repaint our window
		}
		else
		{
			Invalidate();
			UpdateWindow();
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnParentCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		LRESULT lRet = CDRF_DODEFAULT;
		bHandled = FALSE;
		if(pnmh->hwndFrom == m_hWnd)
		{
			LPNMTBCUSTOMDRAW lpTBCustomDraw = (LPNMTBCUSTOMDRAW)pnmh;
			if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
			{
				lRet = CDRF_NOTIFYITEMDRAW;
				bHandled = TRUE;
			}
			else if(lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
			{
				if(m_bFlatMenus)
				{
#ifndef COLOR_MENUHILIGHT
					const int COLOR_MENUHILIGHT = 29;
#endif // !COLOR_MENUHILIGHT
					bool bDisabled = ((lpTBCustomDraw->nmcd.uItemState & CDIS_DISABLED) == CDIS_DISABLED);
					if(!bDisabled && ((lpTBCustomDraw->nmcd.uItemState & CDIS_HOT) == CDIS_HOT || 
						(lpTBCustomDraw->nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED))
					{
						::FillRect(lpTBCustomDraw->nmcd.hdc, &lpTBCustomDraw->nmcd.rc, ::GetSysColorBrush(COLOR_MENUHILIGHT));
						::FrameRect(lpTBCustomDraw->nmcd.hdc, &lpTBCustomDraw->nmcd.rc, ::GetSysColorBrush(COLOR_HIGHLIGHT));
						lpTBCustomDraw->clrText = ::GetSysColor(m_bParentActive ? COLOR_HIGHLIGHTTEXT : COLOR_GRAYTEXT);
					}
					else if(bDisabled || !m_bParentActive)
					{
						lpTBCustomDraw->clrText = ::GetSysColor(COLOR_GRAYTEXT);
					}
					CDCHandle dc = lpTBCustomDraw->nmcd.hdc;
					dc.SetTextColor(lpTBCustomDraw->clrText);
					dc.SetBkMode(lpTBCustomDraw->nStringBkMode);
					HFONT hFont = GetFont();
					HFONT hFontOld = NULL;
					if(hFont != NULL)
						hFontOld = dc.SelectFont(hFont);
					const int cchText = 200;
					TCHAR szText[cchText] = { 0 };
					TBBUTTONINFO tbbi = { 0 };
					tbbi.cbSize = sizeof(TBBUTTONINFO);
					tbbi.dwMask = TBIF_TEXT;
					tbbi.pszText = szText;
					tbbi.cchText = cchText;
					GetButtonInfo((int)lpTBCustomDraw->nmcd.dwItemSpec, &tbbi);
					dc.DrawText(szText, -1, &lpTBCustomDraw->nmcd.rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));
					if(hFont != NULL)
						dc.SelectFont(hFontOld);
					lRet = CDRF_SKIPDEFAULT;
					bHandled = TRUE;
				}
				else if(!m_bParentActive)
				{
					lpTBCustomDraw->clrText = ::GetSysColor(COLOR_GRAYTEXT);
					bHandled = TRUE;
				}
			}
		}
		return lRet;
	}

// Message hook handlers
	LRESULT OnHookMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		static POINT s_point = { -1, -1 };
		DWORD dwPoint = ::GetMessagePos();
		POINT point = { GET_X_LPARAM(dwPoint), GET_Y_LPARAM(dwPoint) };

		bHandled = FALSE;
		if(m_bMenuActive)
		{
			if(::WindowFromPoint(point) == m_hWnd)
			{
				ScreenToClient(&point);
				int nHit = HitTest(&point);

				if((point.x != s_point.x || point.y != s_point.y) && nHit >= 0 && nHit < ::GetMenuItemCount(m_hMenu) && nHit != m_nPopBtn && m_nPopBtn != -1)
				{
					TBBUTTON tbb = { 0 };
					GetButton(nHit, &tbb);
					if((tbb.fsState & TBSTATE_ENABLED) != 0)
					{
						m_nNextPopBtn = nHit | 0xFFFF0000;
						HWND hWndMenu = m_stackMenuWnd.GetCurrent();
						ATLASSERT(hWndMenu != NULL);

						// this one is needed to close a menu if mouse button was down
						::PostMessage(hWndMenu, WM_LBUTTONUP, 0, MAKELPARAM(point.x, point.y));
						// this one closes a popup menu
						::PostMessage(hWndMenu, WM_KEYDOWN, VK_ESCAPE, 0L);

						bHandled = TRUE;
					}
				}
			}
		}
		else
		{
			ScreenToClient(&point);
		}

		s_point = point;
		return 0;
	}

	LRESULT OnHookSysKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook WM_SYSKEYDOWN (0x%2.2X)\n"), wParam);
#endif

		if(wParam == VK_MENU && m_bParentActive && m_bUseKeyboardCues && !m_bShowKeyboardCues && m_bAllowKeyboardCues)
			ShowKeyboardCues(true);

		if(wParam != VK_SPACE && !m_bMenuActive && ::GetFocus() == m_hWnd)
		{
			m_bAllowKeyboardCues = false;
			PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
			T* pT = static_cast<T*>(this);
			pT->GiveFocusBack();
			m_bSkipMsg = true;
		}
		else
		{
			if(wParam == VK_SPACE && m_bUseKeyboardCues && m_bShowKeyboardCues)
			{
				m_bAllowKeyboardCues = true;
				ShowKeyboardCues(false);
			}
			m_uSysKey = (UINT)wParam;
		}
		return 0;
	}

	LRESULT OnHookSysKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(!m_bAllowKeyboardCues)
			m_bAllowKeyboardCues = true;
		bHandled = FALSE;
		wParam;
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook WM_SYSKEYUP (0x%2.2X)\n"), wParam);
#endif
		return 0;
	}

	LRESULT OnHookSysChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook WM_SYSCHAR (0x%2.2X)\n"), wParam);
#endif

		if(!m_bMenuActive && m_hWndHook != m_hWnd && wParam != VK_SPACE)
			bHandled = TRUE;
		return 0;
	}

	LRESULT OnHookKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook WM_KEYDOWN (0x%2.2X)\n"), wParam);
#endif
		bHandled = FALSE;
		T* pT = static_cast<T*>(this);

		if(wParam == VK_ESCAPE && m_stackMenuWnd.GetSize() <= 1)
		{
			if(m_bMenuActive && !m_bContextMenu)
			{
				int nHot = GetHotItem();
				if(nHot == -1)
					nHot = m_nPopBtn;
				if(nHot == -1)
					nHot = 0;
				SetHotItem(nHot);
				bHandled = TRUE;
				pT->TakeFocus();
				m_bEscapePressed = true; // To keep focus
				m_bSkipPostDown = false;
			}
			else if(::GetFocus() == m_hWnd && m_wndParent.IsWindow())
			{
				SetHotItem(-1);
				pT->GiveFocusBack();
				bHandled = TRUE;
			}
		}
		else if(wParam == VK_RETURN || wParam == VK_UP || wParam == VK_DOWN)
		{
			if(!m_bMenuActive && ::GetFocus() == m_hWnd && m_wndParent.IsWindow())
			{
				int nHot = GetHotItem();
				if(nHot != -1)
				{
					if(wParam != VK_RETURN)
					{
						if(!m_bSkipPostDown)
						{
// IE4 only: WM_KEYDOWN doesn't generate TBN_DROPDOWN, we need to simulate a mouse click
#if (_WIN32_IE < 0x0500)
							DWORD dwMajor = 0, dwMinor = 0;
							ATL::AtlGetCommCtrlVersion(&dwMajor, &dwMinor);
							if(dwMajor <= 4 || (dwMajor == 5 && dwMinor < 80))
							{
								RECT rect;
								GetItemRect(nHot, &rect);
								PostMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(rect.left, rect.top));
							}
#endif // (_WIN32_IE < 0x0500)
							PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
							m_bSkipPostDown = true;
						}
						else
						{
							ATLTRACE2(atlTraceUI, 0, _T("CmdBar - skipping posting another VK_DOWN\n"));
							m_bSkipPostDown = false;
						}
					}
				}
				else
				{
					ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Can't find hot button\n"));
				}
			}
			if(wParam == VK_RETURN && m_bMenuActive)
			{
				PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
				m_nNextPopBtn = -1;
				pT->GiveFocusBack();
			}
		}
		else if(wParam == VK_LEFT || wParam == VK_RIGHT)
		{
			WPARAM wpNext = m_bLayoutRTL ? VK_LEFT : VK_RIGHT;
			WPARAM wpPrev = m_bLayoutRTL ? VK_RIGHT : VK_LEFT;

			if(m_bMenuActive && !m_bContextMenu && !(wParam == wpNext && m_bPopupItem))
			{
				bool bAction = false;
				if(wParam == wpPrev && s_pCurrentBar->m_stackMenuWnd.GetSize() == 1)
				{
					m_nNextPopBtn = pT->GetPreviousMenuItem(m_nPopBtn);
					if(m_nNextPopBtn != -1)
						bAction = true;
				}
				else if(wParam == wpNext)
				{
					m_nNextPopBtn = pT->GetNextMenuItem(m_nPopBtn);
					if(m_nNextPopBtn != -1)
						bAction = true;
				}
				HWND hWndMenu = m_stackMenuWnd.GetCurrent();
				ATLASSERT(hWndMenu != NULL);

				// Close the popup menu
				if(bAction)
				{
					::PostMessage(hWndMenu, WM_KEYDOWN, VK_ESCAPE, 0L);
					if(wParam == wpNext)
					{
						int cItem = m_stackMenuWnd.GetSize() - 1;
						while(cItem >= 0)
						{
							hWndMenu = m_stackMenuWnd[cItem];
							if(hWndMenu != NULL)
								::PostMessage(hWndMenu, WM_KEYDOWN, VK_ESCAPE, 0L);
							cItem--;
						}
					}
#if (_WIN32_IE >= 0x0500)
					if(m_nNextPopBtn == -2)
					{
						m_nNextPopBtn = -1;
						pT->DisplayChevronMenu();
					}
#endif // (_WIN32_IE >= 0x0500)
					bHandled = TRUE;
				}
			}
		}
		return 0;
	}

	LRESULT OnHookNextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook WM_NEXTMENU\n"));
#endif
		bHandled = FALSE;
		return 1;
	}

 	LRESULT OnHookChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook WM_CHAR (0x%2.2X)\n"), wParam);
#endif
		bHandled = (wParam == VK_ESCAPE);
		return 0;
	}

// Implementation - ownerdraw overrideables and helpers
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		T* pT = static_cast<T*>(this);
		if(m_bFlatMenus)
			pT->DrawItemFlat(lpDrawItemStruct);
		else
			pT->DrawItem3D(lpDrawItemStruct);

	}

	void DrawItem3D(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_MenuItemData* pmd = (_MenuItemData*)lpDrawItemStruct->itemData;
		CDCHandle dc = lpDrawItemStruct->hDC;
		const RECT& rcItem = lpDrawItemStruct->rcItem;
		T* pT = static_cast<T*>(this);

		if(pmd->fType & MFT_SEPARATOR)
		{
			// draw separator
			RECT rc = rcItem;
			rc.top += (rc.bottom - rc.top) / 2;      // vertical center
			dc.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);   // draw separator line
		}
		else		// not a separator
		{
			BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
			BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
			BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
			BOOL bHasImage = FALSE;

			if(LOWORD(lpDrawItemStruct->itemID) == (WORD)-1)
				bSelected = FALSE;
			RECT rcButn = { rcItem.left, rcItem.top, rcItem.left + m_szButton.cx, rcItem.top + m_szButton.cy };   // button rect
			::OffsetRect(&rcButn, 0, ((rcItem.bottom - rcItem.top) - (rcButn.bottom - rcButn.top)) / 2);          // center vertically

			int iButton = pmd->iButton;
			if(iButton >= 0)
			{
				bHasImage = TRUE;

				// calc drawing point
				SIZE sz = { rcButn.right - rcButn.left - m_szBitmap.cx, rcButn.bottom - rcButn.top - m_szBitmap.cy };
				sz.cx /= 2;
				sz.cy /= 2;
				POINT point = { rcButn.left + sz.cx, rcButn.top + sz.cy };

				// fill background depending on state
				if(!bChecked || (bSelected && !bDisabled))
				{
					if(!bDisabled)
						dc.FillRect(&rcButn, (bChecked && !bSelected) ? COLOR_3DLIGHT : COLOR_MENU);
					else
						dc.FillRect(&rcButn, COLOR_MENU);
				}
				else
				{
					COLORREF crTxt = dc.SetTextColor(::GetSysColor(COLOR_BTNFACE));
					COLORREF crBk = dc.SetBkColor(::GetSysColor(COLOR_BTNHILIGHT));
					CBrush hbr(CDCHandle::GetHalftoneBrush());
					dc.SetBrushOrg(rcButn.left, rcButn.top);
					dc.FillRect(&rcButn, hbr);
					dc.SetTextColor(crTxt);
					dc.SetBkColor(crBk);
				}

				// draw disabled or normal
				if(!bDisabled)
				{
					// draw pushed-in or popped-out edge
					if(bSelected || bChecked)
					{
						RECT rc2 = rcButn;
						dc.DrawEdge(&rc2, bChecked ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
					}
					// draw the image
					::ImageList_Draw(m_hImageList, iButton, dc, point.x, point.y, ILD_TRANSPARENT);
				}
				else
				{
					HBRUSH hBrushBackground = bChecked ? NULL : ::GetSysColorBrush(COLOR_MENU);
					pT->DrawBitmapDisabled(dc, iButton, point, hBrushBackground);
				}
			}
			else
			{
				// no image - look for custom checked/unchecked bitmaps
				CMenuItemInfo info;
				info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
				::GetMenuItemInfo((HMENU)lpDrawItemStruct->hwndItem, lpDrawItemStruct->itemID, MF_BYCOMMAND, &info);
				if(bChecked || info.hbmpUnchecked != NULL)
				{
					BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
					bHasImage = pT->DrawCheckmark(dc, rcButn, bSelected, bDisabled, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
				}
			}

			// draw item text
			int cxButn = m_szButton.cx;
			COLORREF colorBG = ::GetSysColor(bSelected ? COLOR_HIGHLIGHT : COLOR_MENU);
			if(bSelected || lpDrawItemStruct->itemAction == ODA_SELECT)
			{
				RECT rcBG = rcItem;
				if(bHasImage)
					rcBG.left += cxButn + s_kcxGap;
				dc.FillRect(&rcBG, bSelected ? COLOR_HIGHLIGHT : COLOR_MENU);
			}

			// calc text rectangle and colors
			RECT rcText = rcItem;
			rcText.left += cxButn + s_kcxGap + s_kcxTextMargin;
			rcText.right -= cxButn;
			dc.SetBkMode(TRANSPARENT);
			COLORREF colorText = ::GetSysColor(bDisabled ?  (bSelected ? COLOR_GRAYTEXT : COLOR_3DSHADOW) : (bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));

			// font already selected by Windows
			if(bDisabled && (!bSelected || colorText == colorBG))
			{
				// disabled - draw shadow text shifted down and right 1 pixel (unles selected)
				RECT rcDisabled = rcText;
				::OffsetRect(&rcDisabled, 1, 1);
				pT->DrawMenuText(dc, rcDisabled, pmd->lpstrText, ::GetSysColor(COLOR_3DHILIGHT));
			}
			pT->DrawMenuText(dc, rcText, pmd->lpstrText, colorText); // finally!
		}
	}

	void DrawItemFlat(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_MenuItemData* pmd = (_MenuItemData*)lpDrawItemStruct->itemData;
		CDCHandle dc = lpDrawItemStruct->hDC;
		const RECT& rcItem = lpDrawItemStruct->rcItem;
		T* pT = static_cast<T*>(this);

#ifndef COLOR_MENUHILIGHT
		const int COLOR_MENUHILIGHT = 29;
#endif // !COLOR_MENUHILIGHT

		BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
		BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
		BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;

		// paint background
		if(bSelected || lpDrawItemStruct->itemAction == ODA_SELECT)
		{
			if(bSelected)
			{
				dc.FillRect(&rcItem, ::GetSysColorBrush(COLOR_MENUHILIGHT));
				dc.FrameRect(&rcItem, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			}
			else
			{
				dc.FillRect(&rcItem, ::GetSysColorBrush(COLOR_MENU));
			}
		}

		if(pmd->fType & MFT_SEPARATOR)
		{
			// draw separator
			RECT rc = rcItem;
			rc.top += (rc.bottom - rc.top) / 2;      // vertical center
			dc.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);   // draw separator line
		}
		else		// not a separator
		{
			if(LOWORD(lpDrawItemStruct->itemID) == (WORD)-1)
				bSelected = FALSE;
			RECT rcButn = { rcItem.left, rcItem.top, rcItem.left + m_szButton.cx, rcItem.top + m_szButton.cy };   // button rect
			::OffsetRect(&rcButn, 0, ((rcItem.bottom - rcItem.top) - (rcButn.bottom - rcButn.top)) / 2);          // center vertically

			// draw background and border for checked items
			if(bChecked)
			{
				RECT rcCheck = rcButn;
				::InflateRect(&rcCheck, -1, -1);
				if(bSelected)
					dc.FillRect(&rcCheck, ::GetSysColorBrush(COLOR_MENU));
				dc.FrameRect(&rcCheck, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			}

			int iButton = pmd->iButton;
			if(iButton >= 0)
			{
				// calc drawing point
				SIZE sz = { rcButn.right - rcButn.left - m_szBitmap.cx, rcButn.bottom - rcButn.top - m_szBitmap.cy };
				sz.cx /= 2;
				sz.cy /= 2;
				POINT point = { rcButn.left + sz.cx, rcButn.top + sz.cy };

				// draw disabled or normal
				if(!bDisabled)
				{
					::ImageList_Draw(m_hImageList, iButton, dc, point.x, point.y, ILD_TRANSPARENT);
				}
				else
				{
					HBRUSH hBrushBackground = ::GetSysColorBrush((bSelected && !(bDisabled && bChecked)) ? COLOR_MENUHILIGHT : COLOR_MENU);
					HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW);
					pT->DrawBitmapDisabled(dc, iButton, point, hBrushBackground, hBrushBackground, hBrushDisabledImage);
				}
			}
			else
			{
				// no image - look for custom checked/unchecked bitmaps
				CMenuItemInfo info;
				info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
				::GetMenuItemInfo((HMENU)lpDrawItemStruct->hwndItem, lpDrawItemStruct->itemID, MF_BYCOMMAND, &info);
				if(bChecked || info.hbmpUnchecked != NULL)
				{
					BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
					pT->DrawCheckmark(dc, rcButn, bSelected, bDisabled, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
				}
			}

			// draw item text
			int cxButn = m_szButton.cx;
			// calc text rectangle and colors
			RECT rcText = rcItem;
			rcText.left += cxButn + s_kcxGap + s_kcxTextMargin;
			rcText.right -= cxButn;
			dc.SetBkMode(TRANSPARENT);
			COLORREF colorText = ::GetSysColor(bDisabled ?  (bSelected ? COLOR_GRAYTEXT : COLOR_3DSHADOW) : (bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));

			pT->DrawMenuText(dc, rcText, pmd->lpstrText, colorText); // finally!
		}
	}

	void DrawMenuText(CDCHandle& dc, RECT& rc, LPCTSTR lpstrText, COLORREF color)
	{
		int nTab = -1;
		for(int i = 0; i < lstrlen(lpstrText); i++)
		{
			if(lpstrText[i] == _T('\t'))
			{
				nTab = i;
				break;
			}
		}
		dc.SetTextColor(color);
		dc.DrawText(lpstrText, nTab, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));
		if(nTab != -1)
			dc.DrawText(&lpstrText[nTab + 1], -1, &rc, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));
	}

	void DrawBitmapDisabled(CDCHandle& dc, int nImage, POINT point,
			HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_3DFACE),
			HBRUSH hBrush3DEffect = ::GetSysColorBrush(COLOR_3DHILIGHT),
			HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW))
	{
#if (_WIN32_WINNT >= 0x0501) && (_WIN32_IE >= 0x0501)
		if(m_bAlphaImages)
		{
			IMAGELISTDRAWPARAMS ildp = { 0 };
			ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
			ildp.himl = m_hImageList;
			ildp.i = nImage;
			ildp.hdcDst = dc;
			ildp.x = point.x;
			ildp.y = point.y;
			ildp.cx = 0;
			ildp.cy = 0;
			ildp.xBitmap = 0;
			ildp.yBitmap = 0;
			ildp.fStyle = ILD_TRANSPARENT;
			ildp.fState = ILS_SATURATE;
			ildp.Frame = 0;
			::ImageList_DrawIndirect(&ildp);
		}
		else
#endif // (_WIN32_WINNT >= 0x0501) && (_WIN32_IE >= 0x0501)
		{
			// create memory DC
			CDC dcMem;
			dcMem.CreateCompatibleDC(dc);
			// create mono or color bitmap
			CBitmap bmp;
			bmp.CreateCompatibleBitmap(dc, m_szBitmap.cx, m_szBitmap.cy);
			ATLASSERT(bmp.m_hBitmap != NULL);
			// draw image into memory DC--fill BG white first
			HBITMAP hBmpOld = dcMem.SelectBitmap(bmp);
			dcMem.PatBlt(0, 0, m_szBitmap.cx, m_szBitmap.cy, WHITENESS);
			// If white is the text color, we can't use the normal painting since
			// it would blend with the WHITENESS, but the mask is OK
			UINT uDrawStyle = (::GetSysColor(COLOR_BTNTEXT) == RGB(255, 255, 255)) ? ILD_MASK : ILD_NORMAL;
			::ImageList_Draw(m_hImageList, nImage, dcMem, 0, 0, uDrawStyle);
			dc.DitherBlt(point.x, point.y, m_szBitmap.cx, m_szBitmap.cy, dcMem, NULL, 0, 0, hBrushBackground, hBrush3DEffect, hBrushDisabledImage);
			dcMem.SelectBitmap(hBmpOld);   // restore
		}
	}

	// old name
	BOOL Draw3DCheckmark(CDCHandle& dc, const RECT& rc, BOOL bSelected, BOOL bDisabled, BOOL bRadio, HBITMAP hBmpCheck)
	{
		return DrawCheckmark(dc, rc, bSelected, bDisabled, bRadio, hBmpCheck);
	}

	BOOL DrawCheckmark(CDCHandle& dc, const RECT& rc, BOOL bSelected, BOOL bDisabled, BOOL bRadio, HBITMAP hBmpCheck)
	{
		// get checkmark bitmap, if none, use Windows standard
		SIZE size = { 0, 0 };
		CBitmapHandle bmp = hBmpCheck;
		if(hBmpCheck != NULL)
		{
			bmp.GetSize(size);
		}
		else
		{
			size.cx = ::GetSystemMetrics(SM_CXMENUCHECK); 
			size.cy = ::GetSystemMetrics(SM_CYMENUCHECK); 
			bmp.CreateCompatibleBitmap(dc, size.cx, size.cy);
			ATLASSERT(bmp.m_hBitmap != NULL);
		}
		// center bitmap in caller's rectangle
		RECT rcDest = rc;
		if((rc.right - rc.left) > size.cx)
		{
			rcDest.left = rc.left + (rc.right - rc.left - size.cx) / 2;
			rcDest.right = rcDest.left + size.cx;
		}
		if((rc.bottom - rc.top) > size.cy)
		{
			rcDest.top = rc.top + (rc.bottom - rc.top - size.cy) / 2;
			rcDest.bottom = rcDest.top + size.cy;
		}
		// paint background
		if(!m_bFlatMenus)
		{
			if(bSelected && !bDisabled)
			{
				dc.FillRect(&rcDest, COLOR_MENU);
			}
			else
			{
				COLORREF clrTextOld = dc.SetTextColor(::GetSysColor(COLOR_BTNFACE));
				COLORREF clrBkOld = dc.SetBkColor(::GetSysColor(COLOR_BTNHILIGHT));
				CBrush hbr(CDCHandle::GetHalftoneBrush());
				dc.SetBrushOrg(rcDest.left, rcDest.top);
				dc.FillRect(&rcDest, hbr);
				dc.SetTextColor(clrTextOld);
				dc.SetBkColor(clrBkOld);
			}
		}

		// create source image
		CDC dcSource;
		dcSource.CreateCompatibleDC(dc);
		HBITMAP hBmpOld = dcSource.SelectBitmap(bmp);
		// set colors
		const COLORREF clrBlack = RGB(0, 0, 0);
		const COLORREF clrWhite = RGB(255, 255, 255);
		COLORREF clrTextOld = dc.SetTextColor(clrBlack);
		COLORREF clrBkOld = dc.SetBkColor(clrWhite);
		// create mask
		CDC dcMask;
		dcMask.CreateCompatibleDC(dc);
		CBitmap bmpMask;
		bmpMask.CreateBitmap(size.cx, size.cy, 1, 1, NULL);
		HBITMAP hBmpOld1 = dcMask.SelectBitmap(bmpMask);

		// draw the checkmark transparently
		int cx = rcDest.right - rcDest.left;
		int cy = rcDest.bottom - rcDest.top;
		if(hBmpCheck != NULL)
		{
			// build mask based on transparent color	
			dcSource.SetBkColor(m_clrMask);
			dcMask.SetBkColor(clrBlack);
			dcMask.SetTextColor(clrWhite);
			dcMask.BitBlt(0, 0, size.cx, size.cy, dcSource, 0, 0, SRCCOPY);
			// draw bitmap using the mask
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, SRCINVERT);
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask, 0, 0, SRCAND);
			dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, SRCINVERT);
		}
		else
		{
			const DWORD ROP_DSno = 0x00BB0226L;
			const DWORD ROP_DSa = 0x008800C6L;
			const DWORD ROP_DSo = 0x00EE0086L;
			const DWORD ROP_DSna = 0x00220326L;

			// draw mask
			RECT rcSource = { 0, 0, min(size.cx, rc.right - rc.left), min(size.cy, rc.bottom - rc.top) };
			dcMask.DrawFrameControl(&rcSource, DFC_MENU, bRadio ? DFCS_MENUBULLET : DFCS_MENUCHECK);

			// draw shadow if disabled
			if(!m_bFlatMenus && bDisabled)
			{
				// offset by one pixel
				int x = rcDest.left + 1;
				int y = rcDest.top + 1;
				// paint source bitmap
				const int nColor = COLOR_3DHILIGHT;
				dcSource.FillRect(&rcSource, nColor);
				// draw checkmark - special case black and white colors
				COLORREF clrCheck = ::GetSysColor(nColor);
				if(clrCheck == clrWhite)
				{
					dc.BitBlt(x, y, cx, cy, dcMask,  0, 0,   ROP_DSno);
					dc.BitBlt(x, y, cx, cy, dcSource, 0, 0, ROP_DSa);
				}
				else
				{
					if(clrCheck != clrBlack)
					{
						ATLASSERT(dcSource.GetTextColor() == clrBlack);
						ATLASSERT(dcSource.GetBkColor() == clrWhite);
						dcSource.BitBlt(0, 0, size.cx, size.cy, dcMask, 0, 0, ROP_DSna);
					}
					dc.BitBlt(x, y, cx, cy, dcMask,  0,  0,  ROP_DSa);
					dc.BitBlt(x, y, cx, cy, dcSource, 0, 0, ROP_DSo);
				}
			}

			// paint source bitmap
			const int nColor = bDisabled ? COLOR_BTNSHADOW : COLOR_MENUTEXT;
			dcSource.FillRect(&rcSource, nColor);
			// draw checkmark - special case black and white colors
			COLORREF clrCheck = ::GetSysColor(nColor);
			if(clrCheck == clrWhite)
			{
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask,  0, 0,   ROP_DSno);
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, ROP_DSa);
			}
			else
			{
				if(clrCheck != clrBlack)
				{
					ATLASSERT(dcSource.GetTextColor() == clrBlack);
					ATLASSERT(dcSource.GetBkColor() == clrWhite);
					dcSource.BitBlt(0, 0, size.cx, size.cy, dcMask, 0, 0, ROP_DSna);
				}
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcMask,  0,  0,  ROP_DSa);
				dc.BitBlt(rcDest.left, rcDest.top, cx, cy, dcSource, 0, 0, ROP_DSo);
			}
		}
		// restore all
		dc.SetTextColor(clrTextOld);
		dc.SetBkColor(clrBkOld);
		dcSource.SelectBitmap(hBmpOld);
		dcMask.SelectBitmap(hBmpOld1);
		if(hBmpCheck == NULL)
			bmp.DeleteObject();
		// draw pushed-in hilight
		if(!m_bFlatMenus && !bDisabled)
		{
			if(rc.right - rc.left > size.cx)
				::InflateRect(&rcDest, 1,1);   // inflate checkmark by one pixel all around
			dc.DrawEdge(&rcDest, BDR_SUNKENOUTER, BF_RECT);
		}

		return TRUE;
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		_MenuItemData* pmd = (_MenuItemData*)lpMeasureItemStruct->itemData;

		if(pmd->fType & MFT_SEPARATOR)   // separator - use half system height and zero width
		{
			lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENU) / 2;
			lpMeasureItemStruct->itemWidth  = 0;
		}
		else
		{
			// compute size of text - use DrawText with DT_CALCRECT
			CWindowDC dc(NULL);
			CFont fontBold;
			HFONT hOldFont = NULL;
			if(pmd->fState & MFS_DEFAULT)
			{
				// need bold version of font
				LOGFONT lf = { 0 };
				m_fontMenu.GetLogFont(lf);
				lf.lfWeight += 200;
				fontBold.CreateFontIndirect(&lf);
				ATLASSERT(fontBold.m_hFont != NULL);
				hOldFont = dc.SelectFont(fontBold);
			}
			else
			{
				hOldFont = dc.SelectFont(m_fontMenu);
			}

			RECT rcText = { 0, 0, 0, 0 };
			dc.DrawText(pmd->lpstrText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
			int cx = rcText.right - rcText.left;
			dc.SelectFont(hOldFont);

			LOGFONT lf = { 0 };
			m_fontMenu.GetLogFont(lf);
			int cy = lf.lfHeight;
			if(cy < 0)
				cy = -cy;
			const int cyMargin = 8;
			cy += cyMargin;

			// height of item is the bigger of these two
			lpMeasureItemStruct->itemHeight = max(cy, (int)m_szButton.cy);

			// width is width of text plus a bunch of stuff
			cx += 2 * s_kcxTextMargin;   // L/R margin for readability
			cx += s_kcxGap;              // space between button and menu text
			cx += 2 * m_szButton.cx;     // button width (L=button; R=empty margin)
			cx += m_cxExtraSpacing;      // extra between item text and accelerator keys

			// Windows adds 1 to returned value
			cx -= ::GetSystemMetrics(SM_CXMENUCHECK) - 1;
			lpMeasureItemStruct->itemWidth = cx;   // done deal
		}
	}

// Implementation - Hook procs
	static LRESULT CALLBACK CreateHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		const int cchClassName = 7;
		TCHAR szClassName[cchClassName] = { 0 };

		if(nCode == HCBT_CREATEWND)
		{
			HWND hWndMenu = (HWND)wParam;
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("CmdBar - HCBT_CREATEWND (HWND = %8.8X)\n"), hWndMenu);
#endif

			::GetClassName(hWndMenu, szClassName, cchClassName);
			if(!lstrcmp(_T("#32768"), szClassName))
				s_pCurrentBar->m_stackMenuWnd.Push(hWndMenu);
		}
		else if(nCode == HCBT_DESTROYWND)
		{
			HWND hWndMenu = (HWND)wParam;
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("CmdBar - HCBT_DESTROYWND (HWND = %8.8X)\n"), hWndMenu);
#endif

			::GetClassName(hWndMenu, szClassName, cchClassName);
			if(!lstrcmp(_T("#32768"), szClassName))
			{
				ATLASSERT(hWndMenu == s_pCurrentBar->m_stackMenuWnd.GetCurrent());
				s_pCurrentBar->m_stackMenuWnd.Pop();
			}
		}

		return ::CallNextHookEx(s_hCreateHook, nCode, wParam, lParam);
	}

	static LRESULT CALLBACK MessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		LPMSG pMsg = (LPMSG)lParam;

		if(nCode == HC_ACTION && wParam == PM_REMOVE && pMsg->message != GetGetBarMessage() && pMsg->message != WM_FORWARDMSG)
		{
			CCommandBarCtrlBase* pCmdBar = NULL;
			HWND hWnd = pMsg->hwnd;
			DWORD dwPID = 0;
			while(pCmdBar == NULL && hWnd != NULL)
			{
				pCmdBar = (CCommandBarCtrlBase*)::SendMessage(hWnd, GetGetBarMessage(), (WPARAM)&dwPID, 0L);
				hWnd = ::GetParent(hWnd);
			}

			if(pCmdBar != NULL && dwPID == GetCurrentProcessId())
			{
				pCmdBar->m_hWndHook = pMsg->hwnd;
				ATLASSERT(pCmdBar->IsCommandBarBase());

				if(::IsWindow(pCmdBar->m_hWnd))
					pCmdBar->SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
				else
					ATLTRACE2(atlTraceUI, 0, _T("CmdBar - Hook skipping message, can't find command bar!\n"));
			}
		}

		LRESULT lRet = 0;
		ATLASSERT(s_pmapMsgHook != NULL);
		if(s_pmapMsgHook != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			_MsgHookData* pData = s_pmapMsgHook->Lookup(dwThreadID);
			if(pData != NULL)
			{
				lRet = ::CallNextHookEx(pData->hMsgHook, nCode, wParam, lParam);
			}
		}
		return lRet;
	}

// Implementation
	void DoPopupMenu(int nIndex, bool bAnimate)
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - DoPopupMenu, bAnimate = %s\n"), bAnimate ? "true" : "false");
#endif

		// Menu animation flags
#ifndef TPM_VERPOSANIMATION
		const UINT TPM_VERPOSANIMATION = 0x1000L;
#endif
#ifndef TPM_NOANIMATION
		const UINT TPM_NOANIMATION = 0x4000L;
#endif
		T* pT = static_cast<T*>(this);

		// get popup menu and it's position
		RECT rect = { 0 };
		GetItemRect(nIndex, &rect);
		POINT pt = { rect.left, rect.bottom };
		MapWindowPoints(NULL, &pt, 1);
		MapWindowPoints(NULL, &rect);
		TPMPARAMS TPMParams = { 0 };
		TPMParams.cbSize = sizeof(TPMPARAMS);
		TPMParams.rcExclude = rect;
		HMENU hMenuPopup = ::GetSubMenu(m_hMenu, nIndex);
		ATLASSERT(hMenuPopup != NULL);

		// get button ID
		TBBUTTON tbb = { 0 };
		GetButton(nIndex, &tbb);
		int nCmdID = tbb.idCommand;

		m_nPopBtn = nIndex;   // remember current button's index

		// press button and display popup menu
		PressButton(nCmdID, TRUE);
		SetHotItem(nCmdID);
		pT->DoTrackPopupMenu(hMenuPopup, TPM_LEFTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN |
			(s_bW2K ? (bAnimate ? TPM_VERPOSANIMATION : TPM_NOANIMATION) : 0), pt.x, pt.y, &TPMParams);
		PressButton(nCmdID, FALSE);
		if(::GetFocus() != m_hWnd)
			SetHotItem(-1);

		m_nPopBtn = -1;   // restore

		// eat next message if click is on the same button
		MSG msg = { 0 };
		if(::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE) && ::PtInRect(&rect, msg.pt))
			::PeekMessage(&msg, m_hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);

		// check if another popup menu should be displayed
		if(m_nNextPopBtn != -1)
		{
			PostMessage(GetAutoPopupMessage(), m_nNextPopBtn & 0xFFFF);
			if(!(m_nNextPopBtn & 0xFFFF0000) && !m_bPopupItem)
				PostMessage(WM_KEYDOWN, VK_DOWN, 0);
			m_nNextPopBtn = -1;
		}
		else
		{
			m_bContextMenu = false;
			// If user didn't hit escape, give focus back
			if(!m_bEscapePressed)
			{
				if(m_bUseKeyboardCues && m_bShowKeyboardCues)
					m_bAllowKeyboardCues = false;
				pT->GiveFocusBack();
			}
			else
			{
				SetHotItem(nCmdID);
				SetAnchorHighlight(TRUE);
			}
		}
	}

	BOOL DoTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, LPTPMPARAMS lpParams = NULL)
	{
		CMenuHandle menuPopup = hMenu;

		CWindowCreateCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CCommandBarCtrlImpl::DoTrackPopupMenu.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		ATLASSERT(s_hCreateHook == NULL);

		s_pCurrentBar = static_cast<CCommandBarCtrlBase*>(this);

		s_hCreateHook = ::SetWindowsHookEx(WH_CBT, CreateHookProc, ModuleHelper::GetModuleInstance(), GetCurrentThreadId());
		ATLASSERT(s_hCreateHook != NULL);

		m_bPopupItem = false;
		m_bMenuActive = true;

		BOOL bTrackRet = menuPopup.TrackPopupMenuEx(uFlags, x, y, m_hWnd, lpParams);
		m_bMenuActive = false;

		::UnhookWindowsHookEx(s_hCreateHook);

		s_hCreateHook = NULL;
		s_pCurrentBar = NULL;

		lock.Unlock();

		// cleanup - convert menus back to original state
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - TrackPopupMenu - cleanup\n"));
#endif

		ATLASSERT(m_stackMenuWnd.GetSize() == 0);

		UpdateWindow();
		ATL::CWindow wndTL = GetTopLevelParent();
		wndTL.UpdateWindow();

		// restore the menu items to the previous state for all menus that were converted
		if(m_bImagesVisible)
		{
			HMENU hMenuSav = NULL;
			while((hMenuSav = m_stackMenuHandle.Pop()) != NULL)
			{
				menuPopup = hMenuSav;
				BOOL bRet = FALSE;
				// restore state and delete menu item data
				for(int i = 0; i < menuPopup.GetMenuItemCount(); i++)
				{
					CMenuItemInfo mii;
					mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID;
					bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
					ATLASSERT(bRet);

					_MenuItemData* pMI = (_MenuItemData*)mii.dwItemData;
					if(pMI != NULL && pMI->IsCmdBarMenuItem())
					{
						mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
						mii.fType = pMI->fType;
						mii.fState = pMI->fState;
						mii.dwTypeData = pMI->lpstrText;
						mii.cch = lstrlen(pMI->lpstrText);
						mii.dwItemData = NULL;

						bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
						// this one triggers WM_MEASUREITEM
						menuPopup.ModifyMenu(i, MF_BYPOSITION | mii.fType | mii.fState, mii.wID, pMI->lpstrText);
						ATLASSERT(bRet);

						delete [] pMI->lpstrText;
						delete pMI;
					}
				}
			}
		}
		return bTrackRet;
	}

	int GetPreviousMenuItem(int nBtn) const
	{
		if(nBtn == -1)
			return -1;
#if (_WIN32_IE >= 0x0500)
		RECT rcClient;
		GetClientRect(&rcClient);
#endif // (_WIN32_IE >= 0x0500)
		int nNextBtn;
		for(nNextBtn = nBtn - 1; nNextBtn != nBtn; nNextBtn--)
		{
			if(nNextBtn < 0)
				nNextBtn = ::GetMenuItemCount(m_hMenu) - 1;
			TBBUTTON tbb = { 0 };
			GetButton(nNextBtn, &tbb);
#if (_WIN32_IE >= 0x0500)
			RECT rcBtn;
			GetItemRect(nNextBtn, &rcBtn);
			if(rcBtn.right > rcClient.right)
			{
				nNextBtn = -2;   // chevron
				break;
			}
#endif // (_WIN32_IE >= 0x0500)
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0)
				break;
		}
		return (nNextBtn != nBtn) ? nNextBtn : -1;
	}

	int GetNextMenuItem(int nBtn) const
	{
		if(nBtn == -1)
			return -1;
#if (_WIN32_IE >= 0x0500)
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
#endif // (_WIN32_IE >= 0x0500)
		int nNextBtn = 0;
		int nCount = ::GetMenuItemCount(m_hMenu);
		for(nNextBtn = nBtn + 1; nNextBtn != nBtn; nNextBtn++)
		{
			if(nNextBtn >= nCount)
				nNextBtn = 0;
			TBBUTTON tbb = { 0 };
			GetButton(nNextBtn, &tbb);
#if (_WIN32_IE >= 0x0500)
			RECT rcBtn = { 0 };
			GetItemRect(nNextBtn, &rcBtn);
			if(rcBtn.right > rcClient.right)
			{
				nNextBtn = -2;   // chevron
				break;
			}
#endif // (_WIN32_IE >= 0x0500)
			if((tbb.fsState & TBSTATE_ENABLED) != 0 && (tbb.fsState & TBSTATE_HIDDEN) == 0)
				break;
		}
		return (nNextBtn != nBtn) ? nNextBtn : -1;
	}

#if (_WIN32_IE >= 0x0500)
	bool DisplayChevronMenu()
	{
		// assume we are in a rebar
		HWND hWndReBar = GetParent();
		int nCount = (int)::SendMessage(hWndReBar, RB_GETBANDCOUNT, 0, 0L);
		bool bRet = false;
		for(int i = 0; i < nCount; i++)
		{
			REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_CHILD | RBBIM_STYLE };
			BOOL bRetBandInfo = (BOOL)::SendMessage(hWndReBar, RB_GETBANDINFO, i, (LPARAM)&rbbi);
			if(bRetBandInfo && rbbi.hwndChild == m_hWnd)
			{
				if((rbbi.fStyle & RBBS_USECHEVRON) != 0)
				{
					::PostMessage(hWndReBar, RB_PUSHCHEVRON, i, 0L);
					PostMessage(WM_KEYDOWN, VK_DOWN, 0L);
					bRet = true;
				}
				break;
			}
		}
		return bRet;
	}
#endif // (_WIN32_IE >= 0x0500)

	void GetSystemSettings()
	{
		// refresh our font
		NONCLIENTMETRICS info = { RunTimeHelper::SizeOf_NONCLIENTMETRICS() };
		BOOL bRet = ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		ATLASSERT(bRet);
		if(bRet)
		{
			LOGFONT logfont = { 0 };
			if(m_fontMenu.m_hFont != NULL)
				m_fontMenu.GetLogFont(logfont);
			if(logfont.lfHeight != info.lfMenuFont.lfHeight ||
			   logfont.lfWidth != info.lfMenuFont.lfWidth ||
			   logfont.lfEscapement != info.lfMenuFont.lfEscapement ||
			   logfont.lfOrientation != info.lfMenuFont.lfOrientation ||
			   logfont.lfWeight != info.lfMenuFont.lfWeight ||
			   logfont.lfItalic != info.lfMenuFont.lfItalic ||
			   logfont.lfUnderline != info.lfMenuFont.lfUnderline ||
			   logfont.lfStrikeOut != info.lfMenuFont.lfStrikeOut ||
			   logfont.lfCharSet != info.lfMenuFont.lfCharSet ||
			   logfont.lfOutPrecision != info.lfMenuFont.lfOutPrecision ||
			   logfont.lfClipPrecision != info.lfMenuFont.lfClipPrecision ||
			   logfont.lfQuality != info.lfMenuFont.lfQuality ||
			   logfont.lfPitchAndFamily != info.lfMenuFont.lfPitchAndFamily ||
			   lstrcmp(logfont.lfFaceName, info.lfMenuFont.lfFaceName) != 0)
			{
				HFONT hFontMenu = ::CreateFontIndirect(&info.lfMenuFont);
				ATLASSERT(hFontMenu != NULL);
				if(hFontMenu != NULL)
				{
					if(m_fontMenu.m_hFont != NULL)
						m_fontMenu.DeleteObject();
					m_fontMenu.Attach(hFontMenu);
					SetFont(m_fontMenu);
					AddStrings(_T("NS\0"));   // for proper item height
					AutoSize();
				}
			}
		}

		// check if we need extra spacing for menu item text
		CWindowDC dc(m_hWnd);
		HFONT hFontOld = dc.SelectFont(m_fontMenu);
		RECT rcText = { 0, 0, 0, 0 };
		dc.DrawText(_T("\t"), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
		if((rcText.right - rcText.left) < 4)
		{
			::SetRectEmpty(&rcText);
			dc.DrawText(_T("x"), -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);
			m_cxExtraSpacing = rcText.right - rcText.left;
		}
		else
		{
			m_cxExtraSpacing = 0;
		}
		dc.SelectFont(hFontOld);

		// get Windows version
		OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
		::GetVersionEx(&ovi);

		// query keyboard cues mode (Windows 2000 or later)
		if(ovi.dwMajorVersion >= 5)
		{
#ifndef SPI_GETKEYBOARDCUES
			const UINT SPI_GETKEYBOARDCUES = 0x100A;
#endif // !SPI_GETKEYBOARDCUES
			BOOL bRetVal = TRUE;
			bRet = ::SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &bRetVal, 0);
			m_bUseKeyboardCues = (bRet && !bRetVal);
			m_bAllowKeyboardCues = true;
			ShowKeyboardCues(!m_bUseKeyboardCues);
		}

		// query flat menu mode (Windows XP or later)
		if((ovi.dwMajorVersion == 5 && ovi.dwMinorVersion >= 1) || (ovi.dwMajorVersion > 5))
		{
#ifndef SPI_GETFLATMENU
			const UINT SPI_GETFLATMENU = 0x1022;
#endif // !SPI_GETFLATMENU
			BOOL bRetVal = FALSE;
			bRet = ::SystemParametersInfo(SPI_GETFLATMENU, 0, &bRetVal, 0);
			m_bFlatMenus = (bRet && bRetVal);
		}

#if _WTL_CMDBAR_VISTA_MENUS
		// check if we should use Vista menus
		bool bVistaMenus = (RunTimeHelper::IsVista() && RunTimeHelper::IsCommCtrl6() && ((m_dwExtendedStyle & CBR_EX_NOVISTAMENUS) == 0));

		if(bVistaMenus)
		{
			HMODULE hThemeDLL = ::LoadLibrary(_T("uxtheme.dll"));
			if(hThemeDLL != NULL)
			{
				typedef BOOL (STDAPICALLTYPE *PFN_IsThemeActive)();
				PFN_IsThemeActive pfnIsThemeActive = (PFN_IsThemeActive)::GetProcAddress(hThemeDLL, "IsThemeActive");
				ATLASSERT(pfnIsThemeActive != NULL);
				bVistaMenus = bVistaMenus && (pfnIsThemeActive != NULL) && (pfnIsThemeActive() != FALSE);

				typedef BOOL (STDAPICALLTYPE *PFN_IsAppThemed)();
				PFN_IsAppThemed pfnIsAppThemed = (PFN_IsAppThemed)::GetProcAddress(hThemeDLL, "IsAppThemed");
				ATLASSERT(pfnIsAppThemed != NULL);
				bVistaMenus = bVistaMenus && (pfnIsAppThemed != NULL) && (pfnIsAppThemed() != FALSE);

				::FreeLibrary(hThemeDLL);
			}
		}

		if(!bVistaMenus && m_bVistaMenus && (m_hMenu != NULL) && (m_arrCommand.GetSize() > 0))
		{
			T* pT = static_cast<T*>(this);
			pT->_RemoveVistaBitmapsFromMenu();
		}

		m_bVistaMenus = bVistaMenus;
#endif // _WTL_CMDBAR_VISTA_MENUS

#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("CmdBar - GetSystemSettings:\n     m_bFlatMenus = %s\n     m_bUseKeyboardCues = %s     m_bVistaMenus = %s\n"),
			m_bFlatMenus ? "true" : "false", m_bUseKeyboardCues ? "true" : "false", m_bVistaMenus ? "true" : "false");
#endif
	}

// Implementation - alternate focus mode support
	void TakeFocus()
	{
		if((m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && m_hWndFocus == NULL)
			m_hWndFocus = ::GetFocus();
		SetFocus();
	}

	void GiveFocusBack()
	{
		if(m_bParentActive)
		{
			if((m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && ::IsWindow(m_hWndFocus))
				::SetFocus(m_hWndFocus);
			else if(!(m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && m_wndParent.IsWindow())
				m_wndParent.SetFocus();
		}
		m_hWndFocus = NULL;
		SetAnchorHighlight(FALSE);
		if(m_bUseKeyboardCues && m_bShowKeyboardCues)
			ShowKeyboardCues(false);
		m_bSkipPostDown = false;
	}

	void ShowKeyboardCues(bool bShow)
	{
		m_bShowKeyboardCues = bShow;
		SetDrawTextFlags(DT_HIDEPREFIX, m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX);
		Invalidate();
		UpdateWindow();
	}

// Implementation - internal message helpers
	static UINT GetAutoPopupMessage()
	{
		static UINT uAutoPopupMessage = 0;
		if(uAutoPopupMessage == 0)
		{
			CStaticDataInitCriticalSectionLock lock;
			if(FAILED(lock.Lock()))
			{
				ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CCommandBarCtrlImpl::GetAutoPopupMessage.\n"));
				ATLASSERT(FALSE);
				return 0;
			}

			if(uAutoPopupMessage == 0)
				uAutoPopupMessage = ::RegisterWindowMessage(_T("WTL_CmdBar_InternalAutoPopupMsg"));

			lock.Unlock();
		}
		ATLASSERT(uAutoPopupMessage != 0);
		return uAutoPopupMessage;
	}

	static UINT GetGetBarMessage()
	{
		static UINT uGetBarMessage = 0;
		if(uGetBarMessage == 0)
		{
			CStaticDataInitCriticalSectionLock lock;
			if(FAILED(lock.Lock()))
			{
				ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CCommandBarCtrlImpl::GetGetBarMessage.\n"));
				ATLASSERT(FALSE);
				return 0;
			}

			if(uGetBarMessage == 0)
				uGetBarMessage = ::RegisterWindowMessage(_T("WTL_CmdBar_InternalGetBarMsg"));

			lock.Unlock();
		}
		ATLASSERT(uGetBarMessage != 0);
		return uGetBarMessage;
	}

// Implementation
	bool CreateInternalImageList(int cImages)
	{
		UINT uFlags = (m_bAlphaImages ? ILC_COLOR32 : ILC_COLOR24) | ILC_MASK;
		m_hImageList = ::ImageList_Create(m_szBitmap.cx, m_szBitmap.cy, uFlags, cImages, 1);
		ATLASSERT(m_hImageList != NULL);
		return (m_hImageList != NULL);
	}

// Implementation - support for Vista menus
#if _WTL_CMDBAR_VISTA_MENUS
	void _AddVistaBitmapsFromImageList(int nStartIndex, int nCount)
	{
		// Create display compatible memory DC
		CClientDC dc(NULL);
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBitmapSave = dcMem.GetCurrentBitmap();

		T* pT = static_cast<T*>(this);
		// Create bitmaps for all menu items
		for(int i = 0; i < nCount; i++)
		{
			HBITMAP hBitmap = pT->_CreateVistaBitmapHelper(nStartIndex + i, dc, dcMem);
			dcMem.SelectBitmap(hBitmapSave);
			m_arrVistaBitmap.Add(hBitmap);
		}
	}

	void _AddVistaBitmapFromImageList(int nIndex)
	{
		// Create display compatible memory DC
		CClientDC dc(NULL);
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBitmapSave = dcMem.GetCurrentBitmap();

		// Create bitmap for menu item
		T* pT = static_cast<T*>(this);
		HBITMAP hBitmap = pT->_CreateVistaBitmapHelper(nIndex, dc, dcMem);

		// Select saved bitmap back and add bitmap to the array
		dcMem.SelectBitmap(hBitmapSave);
		m_arrVistaBitmap.Add(hBitmap);
	}

	void _ReplaceVistaBitmapFromImageList(int nIndex)
	{
		// Delete existing bitmap
		if(m_arrVistaBitmap[nIndex] != NULL)
			::DeleteObject(m_arrVistaBitmap[nIndex]);

		// Create display compatible memory DC
		CClientDC dc(NULL);
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBitmapSave = dcMem.GetCurrentBitmap();

		// Create bitmap for menu item
		T* pT = static_cast<T*>(this);
		HBITMAP hBitmap = pT->_CreateVistaBitmapHelper(nIndex, dc, dcMem);

		// Select saved bitmap back and replace bitmap in the array
		dcMem.SelectBitmap(hBitmapSave);
		m_arrVistaBitmap.SetAtIndex(nIndex, hBitmap);
	}

	HBITMAP _CreateVistaBitmapHelper(int nIndex, HDC hDCSource, HDC hDCTarget)
	{
		// Create 32-bit bitmap
		BITMAPINFO bi = { 0 };
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = m_szBitmap.cx;
		bi.bmiHeader.biHeight = m_szBitmap.cy;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		bi.bmiHeader.biSizeImage = 0;
		bi.bmiHeader.biXPelsPerMeter = 0;
		bi.bmiHeader.biYPelsPerMeter = 0;
		bi.bmiHeader.biClrUsed = 0;
		bi.bmiHeader.biClrImportant = 0;
		HBITMAP hBitmap = ::CreateDIBSection(hDCSource, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
		ATLASSERT(hBitmap != NULL);

		// Select bitmap into target DC and draw from image list to it
		if(hBitmap != NULL)
		{
			::SelectObject(hDCTarget, hBitmap);

			IMAGELISTDRAWPARAMS ildp = { 0 };
			ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
			ildp.himl = m_hImageList;
			ildp.i = nIndex;
			ildp.hdcDst = hDCTarget;
			ildp.x = 0;
			ildp.y = 0;
			ildp.cx = 0;
			ildp.cy = 0;
			ildp.xBitmap = 0;
			ildp.yBitmap = 0;
			ildp.fStyle = ILD_TRANSPARENT;
			ildp.fState = ILS_ALPHA;
			ildp.Frame = 255;
			::ImageList_DrawIndirect(&ildp);
		}

		return hBitmap;
	}

	void _RemoveVistaBitmapsFromMenu()
	{
		CMenuHandle menu = m_hMenu;
		for(int i = 0; i < m_arrCommand.GetSize(); i++)
		{
			CMenuItemInfo mii;
			mii.fMask = MIIM_BITMAP;
			mii.hbmpItem = NULL;
			menu.SetMenuItemInfo(m_arrCommand[i], FALSE, &mii);
		}
	}
#endif // _WTL_CMDBAR_VISTA_MENUS
};


class CCommandBarCtrl : public CCommandBarCtrlImpl<CCommandBarCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_CommandBar"), GetWndClassName())
};


///////////////////////////////////////////////////////////////////////////////
// CMDICommandBarCtrl - ATL implementation of Command Bars for MDI apps

template <class T, class TBase = CCommandBarCtrlBase, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CMDICommandBarCtrlImpl : public CCommandBarCtrlImpl< T, TBase, TWinTraits>
{
public:
// Data members
	ATL::CContainedWindow m_wndMDIClient;
	bool m_bChildMaximized;
	HWND m_hWndChildMaximized;
	HICON m_hIconChildMaximized;
	int m_nBtnPressed;
	int m_nBtnWasPressed;

	int m_cxyOffset;      // offset between nonclient elements
	int m_cxIconWidth;    // small icon width
	int m_cyIconHeight;   // small icon height
	int m_cxBtnWidth;     // nonclient button width
	int m_cyBtnHeight;    // nonclient button height
	int m_cxLeft;         // left nonclient area width
	int m_cxRight;        // right nonclient area width

// Theme declarations and data members
#ifndef _WTL_NO_AUTO_THEME
#ifndef _UXTHEME_H_
	typedef HANDLE HTHEME;
#endif // !_UXTHEME_H_
	typedef HTHEME (STDAPICALLTYPE *PFN_OpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
	typedef HRESULT (STDAPICALLTYPE *PFN_CloseThemeData)(HTHEME hTheme);
	typedef HRESULT (STDAPICALLTYPE *PFN_DrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
	typedef HRESULT (STDAPICALLTYPE *PFN_DrawThemeParentBackground)(HWND hwnd, HDC hdc, OPTIONAL RECT* prc);

	HMODULE m_hThemeDLL;
	HTHEME m_hTheme;
	PFN_DrawThemeBackground m_pfnDrawThemeBackground;
	PFN_DrawThemeParentBackground m_pfnDrawThemeParentBackground;
#endif // !_WTL_NO_AUTO_THEME

// Constructor/destructor
	CMDICommandBarCtrlImpl() : 
			m_wndMDIClient(this, 2), m_bChildMaximized(false), 
			m_hWndChildMaximized(NULL), m_hIconChildMaximized(NULL), 
			m_nBtnPressed(-1), m_nBtnWasPressed(-1),
#ifndef _WTL_NO_AUTO_THEME
			m_hThemeDLL(NULL), m_hTheme(NULL), m_pfnDrawThemeBackground(NULL), m_pfnDrawThemeParentBackground(NULL), 
#endif // !_WTL_NO_AUTO_THEME
			m_cxyOffset(2),
			m_cxIconWidth(16), m_cyIconHeight(16),
			m_cxBtnWidth(16), m_cyBtnHeight(14),
			m_cxLeft(20), m_cxRight(55)
	{ }

	~CMDICommandBarCtrlImpl()
	{
		if(m_wndMDIClient.IsWindow())
/*scary!*/			m_wndMDIClient.UnsubclassWindow();
	}

// Operations
	BOOL SetMDIClient(HWND hWndMDIClient)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsWindow(hWndMDIClient));
		if(!::IsWindow(hWndMDIClient))
			return FALSE;

#ifdef _DEBUG
		// BLOCK: Test if the passed window is MDICLIENT
		{
			LPCTSTR lpszMDIClientClass = _T("MDICLIENT");
			const int nNameLen = 9 + 1;   // "MDICLIENT" + NULL
			TCHAR szClassName[nNameLen] = { 0 };
			::GetClassName(hWndMDIClient, szClassName, nNameLen);
			ATLASSERT(lstrcmpi(szClassName, lpszMDIClientClass) == 0);
		}
#endif // _DEBUG

		if(m_wndMDIClient.IsWindow())
/*scary!*/		m_wndMDIClient.UnsubclassWindow();

		return m_wndMDIClient.SubclassWindow(hWndMDIClient);
	}

// Message maps
	typedef CCommandBarCtrlImpl< T, TBase, TWinTraits >   _baseClass;
	BEGIN_MSG_MAP(CMDICommandBarCtrlImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
#ifndef _WTL_NO_AUTO_THEME
		MESSAGE_HANDLER(_GetThemeChangedMsg(), OnThemeChanged)
#endif // !_WTL_NO_AUTO_THEME
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_NCCALCSIZE, OnNcCalcSize)
		MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNcHitTest)
		MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNcLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_NCLBUTTONDBLCLK, OnNcLButtonDblClk)
		MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
		CHAIN_MSG_MAP(_baseClass)
	ALT_MSG_MAP(1)   // Parent window messages
		MESSAGE_HANDLER(WM_ACTIVATE, OnParentActivate)
		CHAIN_MSG_MAP_ALT(_baseClass, 1)
	ALT_MSG_MAP(2)   // MDI client window messages
		MESSAGE_HANDLER(WM_MDISETMENU, OnMDISetMenu)
		// no chaining needed since this was moved from the base class here
	ALT_MSG_MAP(3)   // Message hook messages
		MESSAGE_RANGE_HANDLER(0, 0xFFFF, OnAllHookMessages)
		CHAIN_MSG_MAP_ALT(_baseClass, 3)
	END_MSG_MAP()

// Additional MDI message handlers
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRet = _baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
		if(lRet == (LRESULT)-1)
			return lRet;

#ifndef _WTL_NO_AUTO_THEME
		// this will fail if theming is not supported
		m_hThemeDLL = ::LoadLibrary(_T("uxtheme.dll"));
		if(m_hThemeDLL != NULL)
		{
			m_pfnDrawThemeBackground = (PFN_DrawThemeBackground)::GetProcAddress(m_hThemeDLL, "DrawThemeBackground");
			ATLASSERT(m_pfnDrawThemeBackground != NULL);
			if(m_pfnDrawThemeBackground != NULL)
			{
				T* pT = static_cast<T*>(this);
				pT->_OpenThemeData();
			}
			else
			{
				::FreeLibrary(m_hThemeDLL);
				m_hThemeDLL = NULL;
			}
			m_pfnDrawThemeParentBackground = (PFN_DrawThemeParentBackground)::GetProcAddress(m_hThemeDLL, "DrawThemeParentBackground");
			ATLASSERT(m_pfnDrawThemeParentBackground != NULL);
		}
#endif // !_WTL_NO_AUTO_THEME

		return lRet;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRet = _baseClass::OnDestroy(uMsg, wParam, lParam, bHandled);

#ifndef _WTL_NO_AUTO_THEME
		if(m_hThemeDLL != NULL)
		{
			T* pT = static_cast<T*>(this);
			pT->_CloseThemeData();
			::FreeLibrary(m_hThemeDLL);
			m_hThemeDLL = NULL;
		}
#endif // !_WTL_NO_AUTO_THEME

		return lRet;
	}

#ifndef _WTL_NO_AUTO_THEME
	LRESULT OnThemeChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(m_hThemeDLL != NULL)
		{
			T* pT = static_cast<T*>(this);
			pT->_CloseThemeData();
			pT->_OpenThemeData();
		}
		return 0;
	}
#endif // !_WTL_NO_AUTO_THEME

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		T* pT = static_cast<T*>(this);
		pT->_AdjustBtnSize(GET_Y_LPARAM(lParam));
		return lRet;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		if(m_bChildMaximized && (BOOL)wParam)
		{
			LPNCCALCSIZE_PARAMS lpParams = (LPNCCALCSIZE_PARAMS)lParam;
			if(m_bLayoutRTL)
			{
				lpParams->rgrc[0].left += m_cxRight;
				lpParams->rgrc[0].right -= m_cxLeft;
			}
			else
			{
				lpParams->rgrc[0].left += m_cxLeft;
				lpParams->rgrc[0].right -= m_cxRight;
			}
		}

		return lRet;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		if(!m_bChildMaximized)
			return lRet;

		ATLASSERT(m_hWndChildMaximized != NULL && m_hIconChildMaximized != NULL);

		// get DC and window rectangle
		CWindowDC dc(m_hWnd);
		RECT rect;
		GetWindowRect(&rect);
		int cxWidth = rect.right - rect.left;
		int cyHeight = rect.bottom - rect.top;

		// paint left side nonclient background and draw icon
		::SetRect(&rect, 0, 0, m_cxLeft, cyHeight);
#ifndef _WTL_NO_AUTO_THEME
		if(m_hTheme != NULL)
		{
			if(m_pfnDrawThemeParentBackground != NULL)
				m_pfnDrawThemeParentBackground(m_hWnd, dc, &rect);
			else
				dc.FillRect(&rect, COLOR_WINDOW);
		}
		else
#endif // !_WTL_NO_AUTO_THEME
		{
			if((m_dwExtendedStyle & CBR_EX_TRANSPARENT) != 0)
				dc.FillRect(&rect, COLOR_3DFACE);
			else
				dc.FillRect(&rect, COLOR_MENU);
		}

		RECT rcIcon = { 0 };
		T* pT = static_cast<T*>(this);
		pT->_CalcIconRect(cxWidth, cyHeight, rcIcon);
		dc.DrawIconEx(rcIcon.left, rcIcon.top, m_hIconChildMaximized, m_cxIconWidth, m_cyIconHeight);

		// paint right side nonclient background
		::SetRect(&rect, cxWidth - m_cxRight, 0, cxWidth, cyHeight);
#ifndef _WTL_NO_AUTO_THEME
		if(m_hTheme != NULL)
		{
			if(m_pfnDrawThemeParentBackground != NULL)
			{
				// this is to account for the left non-client area
				POINT ptOrg = { 0, 0 };
				dc.GetViewportOrg(&ptOrg);
				dc.SetViewportOrg(ptOrg.x + m_cxLeft, ptOrg.y);
				::OffsetRect(&rect, -m_cxLeft, 0);

				m_pfnDrawThemeParentBackground(m_hWnd, dc, &rect);

				// restore
				dc.SetViewportOrg(ptOrg);
				::OffsetRect(&rect, m_cxLeft, 0);
			}
			else
			{
				dc.FillRect(&rect, COLOR_3DFACE);
			}
		}
		else
#endif // !_WTL_NO_AUTO_THEME
		{
			if((m_dwExtendedStyle & CBR_EX_TRANSPARENT) != 0)
				dc.FillRect(&rect, COLOR_3DFACE);
			else
				dc.FillRect(&rect, COLOR_MENU);
		}

		// draw buttons
		RECT arrRect[3] = { 0 };
		pT->_CalcBtnRects(cxWidth, cyHeight, arrRect);
		pT->_DrawMDIButton(dc, arrRect, -1);   // draw all buttons

		return lRet;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		if(m_bChildMaximized)
		{
			RECT rect = { 0 };
			GetWindowRect(&rect);
			POINT pt = { GET_X_LPARAM(lParam) - rect.left, GET_Y_LPARAM(lParam) - rect.top };
			if(m_bLayoutRTL)
			{
				if((pt.x < m_cxRight) || (pt.x > ((rect.right - rect.left) - m_cxLeft)))
					lRet = HTBORDER;
			}
			else
			{
				if((pt.x < m_cxLeft) || (pt.x > ((rect.right - rect.left) - m_cxRight)))
					lRet = HTBORDER;
			}
		}
		return lRet;
	}

	LRESULT OnNcLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if(!m_bChildMaximized)
		{
			bHandled = FALSE;
			return 1;
		}

		ATLASSERT(_DebugCheckChild());

		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		RECT rect = { 0 };
		GetWindowRect(&rect);
		pt.x -= rect.left;
		pt.y -= rect.top;

		RECT rcIcon = { 0 };
		T* pT = static_cast<T*>(this);
		pT->_CalcIconRect(rect.right - rect.left, rect.bottom - rect.top, rcIcon, m_bLayoutRTL);
		RECT arrRect[3] = { 0 };
		pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect, m_bLayoutRTL);

		if(::PtInRect(&rcIcon, pt))
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonDown: icon\n"));
#endif
#ifndef TPM_VERPOSANIMATION
			const UINT TPM_VERPOSANIMATION = 0x1000L;   // Menu animation flag
#endif
			CMenuHandle menu = ::GetSystemMenu(m_hWndChildMaximized, FALSE);
			UINT uRet = (UINT)menu.TrackPopupMenu(TPM_LEFTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD |  
				(s_bW2K ? TPM_VERPOSANIMATION : 0), m_bLayoutRTL ? rect.right : rect.left, rect.bottom, m_hWndChildMaximized);

			// eat next message if click is on the same button
			::OffsetRect(&rcIcon, rect.left, rect.top);
			MSG msg = { 0 };
			if(::PeekMessage(&msg, m_hWnd, WM_NCLBUTTONDOWN, WM_NCLBUTTONDOWN, PM_NOREMOVE) && ::PtInRect(&rcIcon, msg.pt))
				::PeekMessage(&msg, m_hWnd, WM_NCLBUTTONDOWN, WM_NCLBUTTONDOWN, PM_REMOVE);

			if(uRet != 0)
				::SendMessage(m_hWndChildMaximized, WM_SYSCOMMAND, uRet, 0L);
		}
		else if(::PtInRect(&arrRect[0], pt))
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonDown: close button\n"));
#endif
			m_nBtnWasPressed = m_nBtnPressed = 0;
		}
		else if(::PtInRect(&arrRect[1], pt))
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonDown: restore button\n"));
#endif
			m_nBtnWasPressed = m_nBtnPressed = 1;
		}
		else if(::PtInRect(&arrRect[2], pt))
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonDown: minimize button\n"));
#endif
			m_nBtnWasPressed = m_nBtnPressed = 2;
		}
		else
		{
			bHandled = FALSE;
		}

		// draw the button state if it was pressed
		if(m_nBtnPressed != -1)
		{
			SetCapture();
			CWindowDC dc(m_hWnd);
			pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect);
			pT->_DrawMDIButton(dc, arrRect, m_nBtnPressed);
		}

		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if(!m_bChildMaximized || ::GetCapture() != m_hWnd || m_nBtnWasPressed == -1)
		{
			bHandled = FALSE;
			return 1;
		}

		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ClientToScreen(&pt);
		RECT rect = { 0 };
		GetWindowRect(&rect);
		pt.x -= rect.left;
		pt.y -= rect.top;
		RECT arrRect[3] = { 0 };
		T* pT = static_cast<T*>(this);
		pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect, m_bLayoutRTL);
		int nOldBtnPressed = m_nBtnPressed;
		m_nBtnPressed = ::PtInRect(&arrRect[m_nBtnWasPressed], pt) ? m_nBtnWasPressed : -1;
		if(nOldBtnPressed != m_nBtnPressed)
		{
			CWindowDC dc(m_hWnd);
			pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect);
			pT->_DrawMDIButton(dc, arrRect, (m_nBtnPressed != -1) ? m_nBtnPressed : nOldBtnPressed);
		}

		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if(!m_bChildMaximized || ::GetCapture() != m_hWnd || m_nBtnWasPressed == -1)
		{
			bHandled = FALSE;
			return 1;
		}

		ATLASSERT(_DebugCheckChild());

		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ClientToScreen(&pt);
		RECT rect = { 0 };
		GetWindowRect(&rect);
		pt.x -= rect.left;
		pt.y -= rect.top;

		int nBtn = m_nBtnWasPressed;
		ReleaseCapture();

		RECT arrRect[3] = { 0 };
		T* pT = static_cast<T*>(this);
		pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect, m_bLayoutRTL);
		if(::PtInRect(&arrRect[nBtn], pt))
		{
			switch(nBtn)
			{
			case 0:		// close
#ifdef _CMDBAR_EXTRA_TRACE
				ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonUp: close button\n"));
#endif
				::SendMessage(m_hWndChildMaximized, WM_SYSCOMMAND, SC_CLOSE, 0L);
				break;
			case 1:		// restore
#ifdef _CMDBAR_EXTRA_TRACE
				ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonUp: restore button\n"));
#endif
				::SendMessage(m_hWndChildMaximized, WM_SYSCOMMAND, SC_RESTORE, 0L);
				break;
			case 2:		// minimize
#ifdef _CMDBAR_EXTRA_TRACE
				ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - LButtonUp: minimize button\n"));
#endif
				::SendMessage(m_hWndChildMaximized, WM_SYSCOMMAND, SC_MINIMIZE, 0L);
				break;
			default:
				break;
			}
		}

		return 0;
	}

	LRESULT OnNcLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if(!m_bChildMaximized || m_nBtnWasPressed != -1)
		{
			bHandled = FALSE;
			return 1;
		}

		ATLASSERT(_DebugCheckChild());

		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		RECT rect = { 0 };
		GetWindowRect(&rect);
		pt.x -= rect.left;
		pt.y -= rect.top;

		RECT rcIcon = { 0 };
		T* pT = static_cast<T*>(this);
		pT->_CalcIconRect(rect.right - rect.left, rect.bottom - rect.top, rcIcon, m_bLayoutRTL);
		RECT arrRect[3] = { 0 };
		pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect, m_bLayoutRTL);

		if(::PtInRect(&rcIcon, pt))
		{
			CMenuHandle menu = ::GetSystemMenu(m_hWndChildMaximized, FALSE);
			UINT uDefID = menu.GetMenuDefaultItem();
			if(uDefID == (UINT)-1)
				uDefID = SC_CLOSE;
			::SendMessage(m_hWndChildMaximized, WM_SYSCOMMAND, uDefID, 0L);
		}

		return 0;
	}

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(m_bChildMaximized)
		{
			if(m_nBtnPressed != -1)
			{
				ATLASSERT(m_nBtnPressed == m_nBtnWasPressed);   // must be
				m_nBtnPressed = -1;
				RECT rect = { 0 };
				GetWindowRect(&rect);
				RECT arrRect[3] = { 0 };
				T* pT = static_cast<T*>(this);
				pT->_CalcBtnRects(rect.right - rect.left, rect.bottom - rect.top, arrRect);
				CWindowDC dc(m_hWnd);
				pT->_DrawMDIButton(dc, arrRect, m_nBtnWasPressed);
			}
			m_nBtnWasPressed = -1;
		}
		else
		{
			bHandled = FALSE;
		}
		return 0;
	}

// Parent window message handlers
	LRESULT OnParentActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_bParentActive = (LOWORD(wParam) != WA_INACTIVE);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
		bHandled = FALSE;
		return 1;
	}

// MDI client window message handlers
	LRESULT OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		m_wndMDIClient.DefWindowProc(uMsg, NULL, lParam);
		HMENU hOldMenu = GetMenu();
		BOOL bRet = AttachMenu((HMENU)wParam);
		bRet;   // avoid level 4 warning
		ATLASSERT(bRet);

#if (_WIN32_IE >= 0x0400)
		T* pT = static_cast<T*>(this);
		pT->UpdateRebarBandIdealSize();
#endif // (_WIN32_IE >= 0x0400)

		return (LRESULT)hOldMenu;
	}

// All messages from the message hook
	LRESULT OnAllHookMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->_ProcessAllHookMessages(uMsg, wParam, lParam);

		bHandled = FALSE;
		return 1;
	}

// Overrideables
	// override this to provide different ideal size
	void UpdateRebarBandIdealSize()
	{
		// assuming we are in a rebar, change ideal size to our size
		// we hope that if we are not in a rebar, nCount will be 0
		int nCount = (int)::SendMessage(GetParent(), RB_GETBANDCOUNT, 0, 0L);
		for(int i = 0; i < nCount; i++)
		{
			REBARBANDINFO rbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE };
			::SendMessage(GetParent(), RB_GETBANDINFO, i, (LPARAM)&rbi);
			if(rbi.hwndChild == m_hWnd)
			{
				rbi.fMask = RBBIM_IDEALSIZE;
				rbi.cxIdeal = m_bChildMaximized ? m_cxLeft + m_cxRight : 0;
				int nBtnCount = GetButtonCount();
				if(nBtnCount > 0)
				{
					RECT rect = { 0 };
					GetItemRect(nBtnCount - 1, &rect);
					rbi.cxIdeal += rect.right;
				}
				::SendMessage(GetParent(), RB_SETBANDINFO, i, (LPARAM)&rbi);
				break;
			}
		}
	}

	// all hook messages - check for the maximized MDI child window change
	void _ProcessAllHookMessages(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
	{
		if(uMsg == WM_MDIGETACTIVE || uMsg == WM_MDISETMENU)
			return;

		BOOL bMaximized = FALSE;
		HWND hWndChild = (HWND)::SendMessage(m_wndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized);
		bool bMaxOld = m_bChildMaximized;
		m_bChildMaximized = (hWndChild != NULL && bMaximized);
		HICON hIconOld = m_hIconChildMaximized;

		if(m_bChildMaximized)
		{
			if(m_hWndChildMaximized != hWndChild)
			{
				ATL::CWindow wnd = m_hWndChildMaximized = hWndChild;
				m_hIconChildMaximized = wnd.GetIcon(FALSE);
				if(m_hIconChildMaximized == NULL)
				{
					m_hIconChildMaximized = wnd.GetIcon(TRUE);
					if(m_hIconChildMaximized == NULL)
					{
						// no icon set with WM_SETICON, get the class one
// need conditional code because types don't match in winuser.h
#ifdef _WIN64
						m_hIconChildMaximized = (HICON)::GetClassLongPtr(wnd, GCLP_HICONSM);
#else
						m_hIconChildMaximized = (HICON)LongToHandle(::GetClassLongPtr(wnd, GCLP_HICONSM));
#endif
					}
				}
			}
		}
		else
		{
			m_hWndChildMaximized = NULL;
			m_hIconChildMaximized = NULL;
		}

		if(bMaxOld != m_bChildMaximized)
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - All messages hook change: m_bChildMaximized = %s\n"), m_bChildMaximized ? "true" : "false");
#endif
			// assuming we are in a rebar, change our size to accomodate new state
			// we hope that if we are not in a rebar, nCount will be 0
			int nCount = (int)::SendMessage(GetParent(), RB_GETBANDCOUNT, 0, 0L);
			int cxDiff = (m_bChildMaximized ? 1 : -1) * (m_cxLeft + m_cxRight);
			for(int i = 0; i < nCount; i++)
			{
#if (_WIN32_IE >= 0x0500)
				REBARBANDINFO rbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_STYLE };
				::SendMessage(GetParent(), RB_GETBANDINFO, i, (LPARAM)&rbi);
				if(rbi.hwndChild == m_hWnd)
				{
					if((rbi.fStyle & RBBS_USECHEVRON) != 0)
					{
						rbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
						rbi.cxMinChild += cxDiff;
						rbi.cxIdeal += cxDiff;
						::SendMessage(GetParent(), RB_SETBANDINFO, i, (LPARAM)&rbi);
					}
					break;
				}
#elif (_WIN32_IE >= 0x0400)
				REBARBANDINFO rbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE };
				::SendMessage(GetParent(), RB_GETBANDINFO, i, (LPARAM)&rbi);
				if(rbi.hwndChild == m_hWnd)
				{
					rbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
					rbi.cxMinChild += cxDiff;
					rbi.cxIdeal += cxDiff;
					::SendMessage(GetParent(), RB_SETBANDINFO, i, (LPARAM)&rbi);
					break;
				}
#else // (_WIN32_IE < 0x0400)
				REBARBANDINFO rbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_CHILD | RBBIM_CHILDSIZE };
				::SendMessage(GetParent(), RB_GETBANDINFO, i, (LPARAM)&rbi);
				if(rbi.hwndChild == m_hWnd)
				{
					rbi.fMask = RBBIM_CHILDSIZE;
					rbi.cxMinChild += cxDiff;
					::SendMessage(GetParent(), RB_SETBANDINFO, i, (LPARAM)&rbi);
					break;
				}
#endif // (_WIN32_IE < 0x0400)
			}
		}

		if(bMaxOld != m_bChildMaximized || hIconOld != m_hIconChildMaximized)
		{
			// force size change and redraw everything
			RECT rect = { 0 };
			GetWindowRect(&rect);
			::MapWindowPoints(NULL, GetParent(), (LPPOINT)&rect, 2);
			SetRedraw(FALSE);
			SetWindowPos(NULL, 0, 0, 1, 1, SWP_NOZORDER | SWP_NOMOVE);
			SetWindowPos(NULL, &rect, SWP_NOZORDER | SWP_NOMOVE);
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

// Implementation
	void GetSystemSettings()
	{
#ifdef _CMDBAR_EXTRA_TRACE
		ATLTRACE2(atlTraceUI, 0, _T("MDI CmdBar - GetSystemSettings\n"));
#endif
		_baseClass::GetSystemSettings();

		NONCLIENTMETRICS info = { RunTimeHelper::SizeOf_NONCLIENTMETRICS() };
		BOOL bRet = ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		ATLASSERT(bRet);
		if(bRet)
		{
			m_cxIconWidth = ::GetSystemMetrics(SM_CXSMICON);
			m_cyIconHeight = ::GetSystemMetrics(SM_CYSMICON);
			m_cxLeft = m_cxIconWidth;

#ifndef _WTL_NO_AUTO_THEME
			if(m_hTheme != NULL)
			{
				m_cxBtnWidth = info.iCaptionWidth - 2 * m_cxyOffset;
				m_cyBtnHeight = info.iCaptionHeight - 2 * m_cxyOffset;
				m_cxRight = 3 * m_cxBtnWidth;
			}
			else
#endif // !_WTL_NO_AUTO_THEME
			{
				m_cxBtnWidth = info.iCaptionWidth - m_cxyOffset;
				m_cyBtnHeight = info.iCaptionHeight - 2 * m_cxyOffset;
				m_cxRight = 3 * m_cxBtnWidth + m_cxyOffset;
			}
		}

		RECT rect = { 0 };
		GetClientRect(&rect);
		T* pT = static_cast<T*>(this);
		pT->_AdjustBtnSize(rect.bottom);
	}

	void _AdjustBtnSize(int cyHeight)
	{
		if(cyHeight > 1 && m_cyBtnHeight > cyHeight)
		{
#ifndef _WTL_NO_AUTO_THEME
			if(m_hTheme != NULL)
			{
				m_cyBtnHeight = cyHeight;
				m_cxBtnWidth = cyHeight;
				m_cxRight = 3 * m_cxBtnWidth;
			}
			else
#endif // !_WTL_NO_AUTO_THEME
			{
				m_cyBtnHeight = cyHeight;
				m_cxBtnWidth = cyHeight + m_cxyOffset;
				m_cxRight = 3 * m_cxBtnWidth + m_cxyOffset;
			}
		}
	}

	void _CalcIconRect(int cxWidth, int cyHeight, RECT& rect, bool bInvertX = false) const
	{
		int xStart = (m_cxLeft - m_cxIconWidth) / 2;
		if(xStart < 0)
			xStart = 0;
		int yStart = (cyHeight - m_cyIconHeight) / 2;
		if(yStart < 0)
			yStart = 0;

		if(bInvertX)
			::SetRect(&rect, cxWidth - (xStart + m_cxBtnWidth), yStart, cxWidth - xStart, yStart + m_cyBtnHeight);
		else
			::SetRect(&rect, xStart, yStart, xStart + m_cxBtnWidth, yStart + m_cyBtnHeight);
	}

	void _CalcBtnRects(int cxWidth, int cyHeight, RECT arrRect[3], bool bInvertX = false) const
	{
		int yStart = (cyHeight - m_cyBtnHeight) / 2;
		if(yStart < 0)
			yStart = 0;

		RECT rcBtn = { cxWidth - m_cxBtnWidth, yStart, cxWidth, yStart + m_cyBtnHeight };
		int nDirection = -1;
		if(bInvertX)
		{
			::SetRect(&rcBtn, 0, yStart, m_cxBtnWidth, yStart + m_cyBtnHeight);
			nDirection = 1;
		}

		arrRect[0] = rcBtn;
#ifndef _WTL_NO_AUTO_THEME
		if(m_hTheme != NULL)
			::OffsetRect(&rcBtn, nDirection * m_cxBtnWidth, 0);
		else
#endif // !_WTL_NO_AUTO_THEME
			::OffsetRect(&rcBtn, nDirection * (m_cxBtnWidth + m_cxyOffset), 0);
		arrRect[1] = rcBtn;
		::OffsetRect(&rcBtn, nDirection * m_cxBtnWidth, 0);
		arrRect[2] = rcBtn;
	}

	void _DrawMDIButton(CWindowDC& dc, LPRECT pRects, int nBtn)
	{
#ifndef _WTL_NO_AUTO_THEME
		if(m_hTheme != NULL)
		{
#ifndef TMSCHEMA_H
			const int WP_MDICLOSEBUTTON = 20;
			const int CBS_NORMAL = 1;
			const int CBS_PUSHED = 3;
			const int CBS_DISABLED = 4;
			const int WP_MDIRESTOREBUTTON = 22;
			const int RBS_NORMAL = 1;
			const int RBS_PUSHED = 3;
			const int RBS_DISABLED = 4;
			const int WP_MDIMINBUTTON = 16;
			const int MINBS_NORMAL = 1;
			const int MINBS_PUSHED = 3;
			const int MINBS_DISABLED = 4;
#endif // TMSCHEMA_H
			if(nBtn == -1 || nBtn == 0)
				m_pfnDrawThemeBackground(m_hTheme, dc, WP_MDICLOSEBUTTON, m_bParentActive ? ((m_nBtnPressed == 0) ? CBS_PUSHED : CBS_NORMAL) : CBS_DISABLED, &pRects[0], NULL);
			if(nBtn == -1 || nBtn == 1)
				m_pfnDrawThemeBackground(m_hTheme, dc, WP_MDIRESTOREBUTTON, m_bParentActive ? ((m_nBtnPressed == 1) ? RBS_PUSHED : RBS_NORMAL) : RBS_DISABLED, &pRects[1], NULL);
			if(nBtn == -1 || nBtn == 2)
				m_pfnDrawThemeBackground(m_hTheme, dc, WP_MDIMINBUTTON, m_bParentActive ? ((m_nBtnPressed == 2) ? MINBS_PUSHED : MINBS_NORMAL) : MINBS_DISABLED, &pRects[2], NULL);
		}
		else
#endif // !_WTL_NO_AUTO_THEME
		{
			if(nBtn == -1 || nBtn == 0)
				dc.DrawFrameControl(&pRects[0], DFC_CAPTION, DFCS_CAPTIONCLOSE | ((m_nBtnPressed == 0) ? DFCS_PUSHED : 0));
			if(nBtn == -1 || nBtn == 1)
				dc.DrawFrameControl(&pRects[1], DFC_CAPTION, DFCS_CAPTIONRESTORE | ((m_nBtnPressed == 1) ? DFCS_PUSHED : 0));
			if(nBtn == -1 || nBtn == 2)
				dc.DrawFrameControl(&pRects[2], DFC_CAPTION, DFCS_CAPTIONMIN | ((m_nBtnPressed == 2) ? DFCS_PUSHED : 0));
		}
	}

#ifndef _WTL_NO_AUTO_THEME
	static UINT _GetThemeChangedMsg()
	{
#ifndef WM_THEMECHANGED
		static const UINT WM_THEMECHANGED = 0x031A;
#endif // !WM_THEMECHANGED
		return WM_THEMECHANGED;
	}

	void _OpenThemeData()
	{
		ATLASSERT(m_hThemeDLL != NULL);

		PFN_OpenThemeData pfnOpenThemeData = (PFN_OpenThemeData)::GetProcAddress(m_hThemeDLL, "OpenThemeData");
		ATLASSERT(pfnOpenThemeData != NULL);
		if(pfnOpenThemeData != NULL)
			m_hTheme = pfnOpenThemeData(m_hWnd, L"Window");
	}

	void _CloseThemeData()
	{
		ATLASSERT(m_hThemeDLL != NULL);

		if(m_hTheme == NULL)
			return;   // nothing to do

		PFN_CloseThemeData pfnCloseThemeData = (PFN_CloseThemeData)::GetProcAddress(m_hThemeDLL, "CloseThemeData");
		ATLASSERT(pfnCloseThemeData != NULL);
		if(pfnCloseThemeData != NULL)
		{
			pfnCloseThemeData(m_hTheme);
			m_hTheme = NULL;
		}
	}
#endif // !_WTL_NO_AUTO_THEME

	bool _DebugCheckChild()
	{
#ifdef _DEBUG
		BOOL bMaximized = FALSE;
		HWND hWndChild = (HWND)::SendMessage(m_wndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized);
		return (bMaximized && hWndChild == m_hWndChildMaximized);
#else // !_DEBUG
		return true;
#endif // !_DEBUG
	}
};

class CMDICommandBarCtrl : public CMDICommandBarCtrlImpl<CMDICommandBarCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_MDICommandBar"), GetWndClassName())
};

}; // namespace WTL

#endif // __ATLCTRLW_H__
