/*
 * Copyright 2010 Vicente J. Botet Escriba
 * Copyright 2015 Andrey Semashev
 * Copyright 2017 James E. King, III
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_WAIT_HPP_INCLUDED_
#define BOOST_WINAPI_WAIT_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/wait_constants.hpp>
#include <boost/winapi/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {

#if BOOST_WINAPI_PARTITION_APP || BOOST_WINAPI_PARTITION_SYSTEM
BOOST_WINAPI_IMPORT mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
WaitForSingleObjectEx(
    mars_boost::winapi::HANDLE_ hHandle,
    mars_boost::winapi::DWORD_ dwMilliseconds,
    mars_boost::winapi::BOOL_ bAlertable);
#endif

#if BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM
#if BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_NT4
BOOST_WINAPI_IMPORT mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
SignalObjectAndWait(
    mars_boost::winapi::HANDLE_ hObjectToSignal,
    mars_boost::winapi::HANDLE_ hObjectToWaitOn,
    mars_boost::winapi::DWORD_ dwMilliseconds,
    mars_boost::winapi::BOOL_ bAlertable);
#endif
#endif

#if BOOST_WINAPI_PARTITION_APP_SYSTEM
BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
WaitForSingleObject(
    mars_boost::winapi::HANDLE_ hHandle,
    mars_boost::winapi::DWORD_ dwMilliseconds);

BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
WaitForMultipleObjects(
    mars_boost::winapi::DWORD_ nCount,
    mars_boost::winapi::HANDLE_ const* lpHandles,
    mars_boost::winapi::BOOL_ bWaitAll,
    mars_boost::winapi::DWORD_ dwMilliseconds);

BOOST_WINAPI_IMPORT mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
WaitForMultipleObjectsEx(
    mars_boost::winapi::DWORD_ nCount,
    mars_boost::winapi::HANDLE_ const* lpHandles,
    mars_boost::winapi::BOOL_ bWaitAll,
    mars_boost::winapi::DWORD_ dwMilliseconds,
    mars_boost::winapi::BOOL_ bAlertable);
#endif // BOOST_WINAPI_PARTITION_APP_SYSTEM

} // extern "C"
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace winapi {

#if BOOST_WINAPI_PARTITION_APP || BOOST_WINAPI_PARTITION_SYSTEM
using ::WaitForSingleObjectEx;
#endif
#if BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM
#if BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_NT4
using ::SignalObjectAndWait;
#endif
#endif

#if BOOST_WINAPI_PARTITION_APP_SYSTEM
using ::WaitForMultipleObjects;
using ::WaitForMultipleObjectsEx;
using ::WaitForSingleObject;
#endif

}
}

#include <boost/winapi/detail/footer.hpp>

#endif // BOOST_WINAPI_WAIT_HPP_INCLUDED_
