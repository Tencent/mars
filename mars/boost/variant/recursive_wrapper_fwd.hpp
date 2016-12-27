//-----------------------------------------------------------------------------
// boost variant/recursive_wrapper_fwd.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002
// Eric Friedman, Itay Maman
//
// Portions Copyright (C) 2002 David Abrahams
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_VARIANT_RECURSIVE_WRAPPER_FWD_HPP
#define BOOST_VARIANT_RECURSIVE_WRAPPER_FWD_HPP

#include "boost/mpl/aux_/config/ctps.hpp"
#include "boost/mpl/aux_/lambda_support.hpp"
#include <boost/type_traits/integral_constant.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

//////////////////////////////////////////////////////////////////////////
// class template recursive_wrapper
//
// Enables recursive types in templates by breaking cyclic dependencies.
//
// For example:
//
//   class my;
//
//   typedef variant< int, recursive_wrapper<my> > var;
//
//   class my {
//     var var_;
//     ...
//   };
//
template <typename T> class recursive_wrapper;

///////////////////////////////////////////////////////////////////////////////
// metafunction is_recursive_wrapper (modeled on code by David Abrahams)
//
// True iff specified type matches recursive_wrapper<T>.
//

namespace detail {


template <typename T>
struct is_recursive_wrapper_impl
    : mpl::false_
{
};

template <typename T>
struct is_recursive_wrapper_impl< recursive_wrapper<T> >
    : mpl::true_
{
};


} // namespace detail

template< typename T > struct is_recursive_wrapper
    : public ::mars_boost::integral_constant<bool,(::mars_boost::detail::is_recursive_wrapper_impl<T>::value)>
{
public:
    BOOST_MPL_AUX_LAMBDA_SUPPORT(1,is_recursive_wrapper,(T))
};

///////////////////////////////////////////////////////////////////////////////
// metafunction unwrap_recursive
//
// If specified type T matches recursive_wrapper<U>, then U; else T.
//


template <typename T>
struct unwrap_recursive
{
    typedef T type;

    BOOST_MPL_AUX_LAMBDA_SUPPORT(1,unwrap_recursive,(T))
};

template <typename T>
struct unwrap_recursive< recursive_wrapper<T> >
{
    typedef T type;

    BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(1,unwrap_recursive,(T))
};


} // namespace mars_boost

#endif // BOOST_VARIANT_RECURSIVE_WRAPPER_FWD_HPP
