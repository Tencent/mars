//  process.hpp  --------------------------------------------------------------//

//  Copyright 2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DETAIL_WINAPI_PROCESS_HPP
#define BOOST_DETAIL_WINAPI_PROCESS_HPP

#include <boost/detail/winapi/basic_types.hpp>
#include <boost/detail/winapi/GetCurrentProcess.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

// Windows CE define GetCurrentProcessId as an inline function in kfuncs.h
#if !defined( BOOST_USE_WINDOWS_H ) && !defined( UNDER_CE )
extern "C" {
BOOST_SYMBOL_IMPORT mars_boost::detail::winapi::DWORD_ WINAPI GetCurrentProcessId(BOOST_DETAIL_WINAPI_VOID);
}
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace detail {
namespace winapi {
using ::GetCurrentProcessId;
}
}
}

#endif // BOOST_DETAIL_WINAPI_PROCESS_HPP
