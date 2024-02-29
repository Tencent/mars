/*
 * Copyright 2010 Vicente J. Botet Escriba
 * Copyright 2015 Andrey Semashev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_HANDLES_HPP_INCLUDED_
#define BOOST_WINAPI_HANDLES_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {
BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC
CloseHandle(mars_boost::winapi::HANDLE_ handle);

BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC
DuplicateHandle(
    mars_boost::winapi::HANDLE_ hSourceProcessHandle,
    mars_boost::winapi::HANDLE_ hSourceHandle,
    mars_boost::winapi::HANDLE_ hTargetProcessHandle,
    mars_boost::winapi::HANDLE_* lpTargetHandle,
    mars_boost::winapi::DWORD_ dwDesiredAccess,
    mars_boost::winapi::BOOL_ bInheritHandle,
    mars_boost::winapi::DWORD_ dwOptions);

#if BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_WIN10 && (BOOST_WINAPI_PARTITION_APP || BOOST_WINAPI_PARTITION_SYSTEM)
BOOST_WINAPI_IMPORT mars_boost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC
CompareObjectHandles(
    mars_boost::winapi::HANDLE_ hFirstObjectHandle,
    mars_boost::winapi::HANDLE_ hSecondObjectHandle);
#endif
} // extern "C"
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace winapi {

using ::CloseHandle;
using ::DuplicateHandle;

#if BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_WIN10 && (BOOST_WINAPI_PARTITION_APP || BOOST_WINAPI_PARTITION_SYSTEM)
using ::CompareObjectHandles;
#endif

// Note: MSVC-14.1 does not interpret INVALID_HANDLE_VALUE_ initializer as a constant expression
#if defined( BOOST_USE_WINDOWS_H )
BOOST_CONSTEXPR_OR_CONST DWORD_ DUPLICATE_CLOSE_SOURCE_ = DUPLICATE_CLOSE_SOURCE;
BOOST_CONSTEXPR_OR_CONST DWORD_ DUPLICATE_SAME_ACCESS_ = DUPLICATE_SAME_ACCESS;
const HANDLE_ INVALID_HANDLE_VALUE_ = INVALID_HANDLE_VALUE;
#else
BOOST_CONSTEXPR_OR_CONST DWORD_ DUPLICATE_CLOSE_SOURCE_ = 1;
BOOST_CONSTEXPR_OR_CONST DWORD_ DUPLICATE_SAME_ACCESS_ = 2;
const HANDLE_ INVALID_HANDLE_VALUE_ = (HANDLE_)(-1);
#endif

BOOST_CONSTEXPR_OR_CONST DWORD_ duplicate_close_source = DUPLICATE_CLOSE_SOURCE_;
BOOST_CONSTEXPR_OR_CONST DWORD_ duplicate_same_access = DUPLICATE_SAME_ACCESS_;
// Note: The "unused" attribute here should not be necessary because the variable is a constant.
//       However, MinGW gcc 5.3 spams warnings about this particular constant.
const HANDLE_ invalid_handle_value BOOST_ATTRIBUTE_UNUSED = INVALID_HANDLE_VALUE_;

}
}

#include <boost/winapi/detail/footer.hpp>

#endif // BOOST_WINAPI_HANDLES_HPP_INCLUDED_
