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

#ifndef __ATLRESCE_H__
#define __ATLRESCE_H__

#pragma once

#ifndef _WIN32_WCE
	#error atlresCE.h is only for Windows CE
#endif


#ifdef RC_INVOKED
#ifndef _INC_WINDOWS

  #define VS_VERSION_INFO     1

  #ifdef APSTUDIO_INVOKED
    #define APSTUDIO_HIDDEN_SYMBOLS // Ignore following symbols
  #endif // APSTUDIO_INVOKED

  #ifndef WINVER
    #define WINVER 0x0400   // default to Windows Version 4.0
  #endif // !WINVER

  #if !defined(WCEOLE_ENABLE_DIALOGEX)
    #define DIALOGEX DIALOG DISCARDABLE
  #endif

  #include <commctrl.h>
  #define  SHMENUBAR RCDATA

  #if defined(SHELLSDK_MODULES_AYGSHELL)
    #include <aygshell.h> 
  #else
    #define NOMENU                 0xFFFF
    #define IDS_SHNEW              1
    #define IDM_SHAREDNEW          10
    #define IDM_SHAREDNEWDEFAULT   11
  #endif
  #ifndef I_IMAGENONE
	#define I_IMAGENONE            (-2)
  #endif

  #include <windows.h>

#endif // !_INC_WINDOWS
#endif // RC_INVOKED

#include "atlres.h"

#ifdef APSTUDIO_INVOKED
	#undef APSTUDIO_HIDDEN_SYMBOLS
#endif // APSTUDIO_INVOKED

// Visual Studio dialog editor bug fix
#ifndef DS_FIXEDSYS 
	#define DS_FIXEDSYS 0
#endif

#define IDC_INFOSTATIC 0xFFFE   // == IDC_STATIC -1

///////////////////////////////////////////////////////////////////////////////
// Smartphone and PPC 2005 Resource IDs

// Command and associated string resource IDs
#define ID_MENU_OK                      0xE790
#define ID_MENU_CANCEL                  0xE791
#define ID_MENU							0xE792
#define ID_ACTION						0xE793
#define ID_VIEW_FULLSCREEN              0xE802

// MenuBar resource IDs
#define ATL_IDM_MENU_DONE               0xE701
#define ATL_IDM_MENU_CANCEL             0xE702
#define ATL_IDM_MENU_DONECANCEL         0xE703

// Default device MenuBar control ID and MenuBar resource ID
#define ATL_IDW_MENU_BAR				0xE802  

// SmartPhone spinned controls ID offset for CSpinCtrl
#define ATL_IDW_SPIN_ID                 9999

#endif // __ATLRESCE_H__
