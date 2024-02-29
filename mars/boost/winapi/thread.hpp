/*
 * Copyright 2010 Vicente J. Botet Escriba
 * Copyright 2015 Andrey Semashev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_THREAD_HPP_INCLUDED_
#define BOOST_WINAPI_THREAD_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/get_current_thread.hpp>
#include <boost/winapi/get_current_thread_id.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if BOOST_WINAPI_PARTITION_APP_SYSTEM

#include <boost/winapi/detail/header.hpp>

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {
BOOST_WINAPI_IMPORT mars_boost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
SleepEx(
    mars_boost::winapi::DWORD_ dwMilliseconds,
    mars_boost::winapi::BOOL_ bAlertable);
BOOST_WINAPI_IMPORT_EXCEPT_WM mars_boost::winapi::VOID_ BOOST_WINAPI_WINAPI_CC Sleep(mars_boost::winapi::DWORD_ dwMilliseconds);
BOOST_WINAPI_IMPORT mars_boost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC SwitchToThread(BOOST_WINAPI_DETAIL_VOID);
} // extern "C"
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace winapi {
using ::SleepEx;
using ::Sleep;
using ::SwitchToThread;
}
}

#include <boost/winapi/detail/footer.hpp>

#endif // BOOST_WINAPI_PARTITION_APP_SYSTEM
#endif // BOOST_WINAPI_THREAD_HPP_INCLUDED_
