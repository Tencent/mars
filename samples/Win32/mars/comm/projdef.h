/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#ifndef PROJECT_DEFINE
#define PROJECT_DEFINE

// #define _POSIX_ 1

#include <basetsd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifdef WINAPI_FAMILY_PARTITION
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PC_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define UWP 1
#endif 
#endif

#if !UWP

#endif
#ifdef UWP
#define mkdir _mkdir4UWP
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define stricmp _stricmp
#define snprintf _snprintf
#else
#define mkdir _mkdir
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define stricmp _stricmp
#define snprintf _snprintf
#endif
#define access _access
#if !defined(WIN32)
    #define sscanf sscanf_s
#endif

#define strdup _strdup
#define strtoull _strtoui64

#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_WARNINGS 1

#define _SCL_SECURE_NO_WARNINGS 1
#define S_ISDIR(x) (_S_IFDIR & x)

#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    /// This code is for Windows phone 8
    #define PLATFORM_WP8
#if !UWP
    #define InitializeCriticalSection(x) InitializeCriticalSectionEx(x, 0 , 0)
#endif
#endif

#define WIN32_LEAN_AND_MEAN

#if !defined(WIN32)
#define sscanf sscanf_s
#endif
#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_WARNINGS 1
#define _SCL_SECURE_NO_WARNINGS 1
#define S_ISDIR(x) (_S_IFDIR & x) 
#define WIN32_LEAN_AND_MEAN
#endif

typedef SSIZE_T ssize_t;

