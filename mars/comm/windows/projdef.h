// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


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

