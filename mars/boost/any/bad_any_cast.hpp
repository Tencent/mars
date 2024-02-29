// Copyright Antony Polukhin, 2020-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/any for Documentation.

#ifndef BOOST_ANYS_BAD_ANY_CAST_HPP_INCLUDED
#define BOOST_ANYS_BAD_ANY_CAST_HPP_INCLUDED

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#ifndef BOOST_NO_RTTI
#include <typeinfo>
#endif

#include <stdexcept>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

/// The exception thrown in the event of a failed mars_boost::any_cast of
/// an mars_boost::any, mars_boost::anys::basic_any or mars_boost::anys::unique_any value.
class BOOST_SYMBOL_VISIBLE bad_any_cast :
#ifndef BOOST_NO_RTTI
    public std::bad_cast
#else
    public std::exception
#endif
{
public:
    const char * what() const BOOST_NOEXCEPT_OR_NOTHROW override
    {
        return "mars_boost::bad_any_cast: "
               "failed conversion using mars_boost::any_cast";
    }
};

} // namespace mars_boost


#endif // #ifndef BOOST_ANYS_BAD_ANY_CAST_HPP_INCLUDED
