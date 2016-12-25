//-----------------------------------------------------------------------------
// boost variant/detail/forced_return.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2003
// Eric Friedman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_VARIANT_DETAIL_FORCED_RETURN_HPP
#define BOOST_VARIANT_DETAIL_FORCED_RETURN_HPP

#include "boost/config.hpp"
#include "boost/variant/detail/generic_result_type.hpp"
#include "boost/assert.hpp"

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace detail { namespace variant {

///////////////////////////////////////////////////////////////////////////////
// (detail) function template forced_return
//
// Logical error to permit invocation at runtime, but (artificially) satisfies
// compile-time requirement of returning a result value.
//

#if !defined(BOOST_MSVC)                                \
 && !defined(BOOST_NO_VOID_RETURNS)

// "standard" implementation:

template <typename T>
inline T forced_return()
{
    // logical error: should never be here! (see above)
    BOOST_ASSERT(false);

    T (*dummy_function_ptr)() = 0;
    return dummy_function_ptr();
}

template <>
inline void forced_return<void>()
{
    // logical error: should never be here! (see above)
    BOOST_ASSERT(false);
}

#elif !defined(BOOST_MSVC)

// workaround implementation
//
// TODO: Determine the most efficient way to handle this -- as below? by
// throwing? by recursive call to forced_return itself? etc.
//

template <typename T>
inline
    BOOST_VARIANT_AUX_GENERIC_RESULT_TYPE(T)
forced_return()
{
    // logical error: should never be here! (see above)
    BOOST_ASSERT(false);

    BOOST_VARIANT_AUX_GENERIC_RESULT_TYPE(T) (*dummy)() = 0;
    return dummy();
}

#else // defined(BOOST_MSVC)

# pragma warning( push )
# pragma warning( disable : 4702 ) // unreachable code
// msvc-specific implementation
//
// Leverages __declspec(noreturn) for optimized implementation.
//

__declspec(noreturn)
inline void forced_return_no_return() {};

template <typename T>
inline
    BOOST_VARIANT_AUX_GENERIC_RESULT_TYPE(T)
forced_return()
{
    // logical error: should never be here! (see above)
    BOOST_ASSERT(false);

    forced_return_no_return();
}

# pragma warning( pop )

#endif // BOOST_MSVC optimization

}} // namespace detail::variant
} // namespace mars_boost

#endif // BOOST_VARIANT_DETAIL_FORCED_RETURN_HPP
