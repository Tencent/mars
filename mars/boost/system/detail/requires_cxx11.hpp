#ifndef BOOST_SYSTEM_DETAIL_REQUIRES_CXX11_HPP_INCLUDED
#define BOOST_SYSTEM_DETAIL_REQUIRES_CXX11_HPP_INCLUDED

// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>
#include <boost/config/pragma_message.hpp>

#if defined(BOOST_NO_CXX11_CONSTEXPR) || \
    defined(BOOST_NO_CXX11_NOEXCEPT) || \
    defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS) || \
    defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) || \
    defined(BOOST_NO_CXX11_HDR_SYSTEM_ERROR)

BOOST_PRAGMA_MESSAGE("C++03 support was deprecated in Boost.System 1.82 and will be removed in Boost.System 1.85.")

#endif

#endif // #ifndef BOOST_SYSTEM_DETAIL_REQUIRES_CXX11_HPP_INCLUDED
