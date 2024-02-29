/*
 * Copyright 2010 Vicente J. Botet Escriba
 * Copyright 2014 Renato Tegon Forti, Antony Polukhin
 * Copyright 2015, 2020 Andrey Semashev
 * Copyright 2015 Antony Polukhin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_DLL_HPP_INCLUDED_
#define BOOST_WINAPI_DLL_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/get_proc_address.hpp>
#include <boost/winapi/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {

#if !defined( BOOST_NO_ANSI_APIS )
BOOST_WINAPI_IMPORT mars_boost::winapi::HMODULE_ BOOST_WINAPI_WINAPI_CC
LoadLibraryA(mars_boost::winapi::LPCSTR_ lpFileName);

BOOST_WINAPI_IMPORT mars_boost::winapi::HMODULE_ BOOST_WINAPI_WINAPI_CC
LoadLibraryExA(
    mars_boost::winapi::LPCSTR_ lpFileName,
    mars_boost::winapi::HANDLE_ hFile,
    mars_boost::winapi::DWORD_ dwFlags
);

BOOST_WINAPI_IMPORT mars_boost::winapi::HMODULE_ BOOST_WINAPI_WINAPI_CC
GetModuleHandleA(mars_boost::winapi::LPCSTR_ lpFileName);

BOOST_WINAPI_IMPORT mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
GetModuleFileNameA(
    mars_boost::winapi::HMODULE_ hModule,
    mars_boost::winapi::LPSTR_ lpFilename,
    mars_boost::winapi::DWORD_ nSize
);
#endif

BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::HMODULE_ BOOST_WINAPI_WINAPI_CC
LoadLibraryW(mars_boost::winapi::LPCWSTR_ lpFileName);

BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::HMODULE_ BOOST_WINAPI_WINAPI_CC
LoadLibraryExW(
    mars_boost::winapi::LPCWSTR_ lpFileName,
    mars_boost::winapi::HANDLE_ hFile,
    mars_boost::winapi::DWORD_ dwFlags
);

BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::HMODULE_ BOOST_WINAPI_WINAPI_CC
GetModuleHandleW(mars_boost::winapi::LPCWSTR_ lpFileName);

BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
GetModuleFileNameW(
    mars_boost::winapi::HMODULE_ hModule,
    mars_boost::winapi::LPWSTR_ lpFilename,
    mars_boost::winapi::DWORD_ nSize
);

struct _MEMORY_BASIC_INFORMATION;

#if !defined( BOOST_WINAPI_IS_MINGW )
BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::SIZE_T_ BOOST_WINAPI_WINAPI_CC
VirtualQuery(
    mars_boost::winapi::LPCVOID_ lpAddress,
    ::_MEMORY_BASIC_INFORMATION* lpBuffer,
    mars_boost::winapi::SIZE_T_ dwLength
);
#else // !defined( BOOST_WINAPI_IS_MINGW )
BOOST_WINAPI_IMPORT mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
VirtualQuery(
    mars_boost::winapi::LPCVOID_ lpAddress,
    ::_MEMORY_BASIC_INFORMATION* lpBuffer,
    mars_boost::winapi::DWORD_ dwLength
);
#endif // !defined( BOOST_WINAPI_IS_MINGW )
} // extern "C"
#endif // #if !defined( BOOST_USE_WINDOWS_H )

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace winapi {

typedef struct BOOST_MAY_ALIAS MEMORY_BASIC_INFORMATION_ {
    PVOID_  BaseAddress;
    PVOID_  AllocationBase;
    DWORD_  AllocationProtect;
    SIZE_T_ RegionSize;
    DWORD_  State;
    DWORD_  Protect;
    DWORD_  Type;
} *PMEMORY_BASIC_INFORMATION_;

#if defined( BOOST_USE_WINDOWS_H )
BOOST_CONSTEXPR_OR_CONST DWORD_ DONT_RESOLVE_DLL_REFERENCES_           = DONT_RESOLVE_DLL_REFERENCES;
BOOST_CONSTEXPR_OR_CONST DWORD_ LOAD_WITH_ALTERED_SEARCH_PATH_         = LOAD_WITH_ALTERED_SEARCH_PATH;
#else // defined( BOOST_USE_WINDOWS_H )
BOOST_CONSTEXPR_OR_CONST DWORD_ DONT_RESOLVE_DLL_REFERENCES_           = 0x00000001;
BOOST_CONSTEXPR_OR_CONST DWORD_ LOAD_WITH_ALTERED_SEARCH_PATH_         = 0x00000008;
#endif // defined( BOOST_USE_WINDOWS_H )

// This one is not defined by MinGW
BOOST_CONSTEXPR_OR_CONST DWORD_ LOAD_IGNORE_CODE_AUTHZ_LEVEL_          = 0x00000010;

#if !defined( BOOST_NO_ANSI_APIS )
using ::LoadLibraryA;
using ::LoadLibraryExA;
using ::GetModuleHandleA;
using ::GetModuleFileNameA;
#endif // !defined( BOOST_NO_ANSI_APIS )
using ::LoadLibraryW;
using ::LoadLibraryExW;
using ::GetModuleHandleW;
using ::GetModuleFileNameW;

BOOST_FORCEINLINE SIZE_T_ VirtualQuery(LPCVOID_ lpAddress, MEMORY_BASIC_INFORMATION_* lpBuffer, SIZE_T_ dwLength)
{
    return ::VirtualQuery(lpAddress, reinterpret_cast< ::_MEMORY_BASIC_INFORMATION* >(lpBuffer), dwLength);
}

#if !defined( BOOST_NO_ANSI_APIS )
BOOST_FORCEINLINE HMODULE_ load_library(LPCSTR_ lpFileName)
{
    return ::LoadLibraryA(lpFileName);
}

BOOST_FORCEINLINE HMODULE_ load_library_ex(LPCSTR_ lpFileName, HANDLE_ hFile, DWORD_ dwFlags)
{
    return ::LoadLibraryExA(lpFileName, hFile, dwFlags);
}

BOOST_FORCEINLINE HMODULE_ get_module_handle(LPCSTR_ lpFileName)
{
    return ::GetModuleHandleA(lpFileName);
}

BOOST_FORCEINLINE DWORD_ get_module_file_name(HMODULE_ hModule, LPSTR_ lpFilename, DWORD_ nSize)
{
    return ::GetModuleFileNameA(hModule, lpFilename, nSize);
}
#endif // #if !defined( BOOST_NO_ANSI_APIS )

BOOST_FORCEINLINE HMODULE_ load_library(LPCWSTR_ lpFileName)
{
    return ::LoadLibraryW(lpFileName);
}

BOOST_FORCEINLINE HMODULE_ load_library_ex(LPCWSTR_ lpFileName, HANDLE_ hFile, DWORD_ dwFlags)
{
    return ::LoadLibraryExW(lpFileName, hFile, dwFlags);
}

BOOST_FORCEINLINE HMODULE_ get_module_handle(LPCWSTR_ lpFileName)
{
    return ::GetModuleHandleW(lpFileName);
}

BOOST_FORCEINLINE DWORD_ get_module_file_name(HMODULE_ hModule, LPWSTR_ lpFilename, DWORD_ nSize)
{
    return ::GetModuleFileNameW(hModule, lpFilename, nSize);
}

} // namespace winapi
} // namespace mars_boost

#endif // BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM

//
// FreeLibrary is in a different partition set (slightly)
//

#if BOOST_WINAPI_PARTITION_APP || BOOST_WINAPI_PARTITION_SYSTEM

#if !defined(BOOST_USE_WINDOWS_H)
extern "C" {
BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC
FreeLibrary(mars_boost::winapi::HMODULE_ hModule);
}
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace winapi {
using ::FreeLibrary;
}
}

#endif // BOOST_WINAPI_PARTITION_APP || BOOST_WINAPI_PARTITION_SYSTEM

#include <boost/winapi/detail/footer.hpp>

#endif // BOOST_WINAPI_DLL_HPP_INCLUDED_
