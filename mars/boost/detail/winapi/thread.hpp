//  thread.hpp  --------------------------------------------------------------//

//  Copyright 2010 Vicente J. Botet Escriba
//  Copyright 2015 Andrey Semashev

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DETAIL_WINAPI_THREAD_HPP
#define BOOST_DETAIL_WINAPI_THREAD_HPP

#include <boost/detail/winapi/basic_types.hpp>
#include <boost/detail/winapi/GetCurrentThread.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined( BOOST_USE_WINDOWS_H )
extern "C" {
// Windows CE define GetCurrentThreadId as an inline function in kfuncs.h
#if !defined( UNDER_CE )
BOOST_SYMBOL_IMPORT mars_boost::detail::winapi::DWORD_ WINAPI GetCurrentThreadId(BOOST_DETAIL_WINAPI_VOID);
#endif
BOOST_SYMBOL_IMPORT mars_boost::detail::winapi::DWORD_ WINAPI
SleepEx(
    mars_boost::detail::winapi::DWORD_ dwMilliseconds,
    mars_boost::detail::winapi::BOOL_ bAlertable);
BOOST_SYMBOL_IMPORT mars_boost::detail::winapi::VOID_ WINAPI Sleep(mars_boost::detail::winapi::DWORD_ dwMilliseconds);
BOOST_SYMBOL_IMPORT mars_boost::detail::winapi::BOOL_ WINAPI SwitchToThread(BOOST_DETAIL_WINAPI_VOID);
}
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace detail {
namespace winapi {
using ::GetCurrentThreadId;
using ::SleepEx;
using ::Sleep;
using ::SwitchToThread;
}
}
}

#endif // BOOST_DETAIL_WINAPI_THREAD_HPP
