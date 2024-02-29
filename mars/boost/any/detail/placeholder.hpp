// Copyright Antony Polukhin, 2021-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ANY_ANYS_DETAIL_PLACEHOLDER_HPP
#define BOOST_ANY_ANYS_DETAIL_PLACEHOLDER_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <boost/type_index.hpp>

/// @cond
namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace anys {
namespace detail {

class BOOST_SYMBOL_VISIBLE placeholder {
public:
    virtual ~placeholder() {}
    virtual const mars_boost::typeindex::type_info& type() const noexcept = 0;
};

} // namespace detail
} // namespace anys
} // namespace mars_boost
/// @endcond

#endif  // #ifndef BOOST_ANY_ANYS_DETAIL_PLACEHOLDER_HPP
