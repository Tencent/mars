
#ifndef BOOST_MPL_SET_AUX_SET0_HPP_INCLUDED
#define BOOST_MPL_SET_AUX_SET0_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <third_party/boost/mpl/long.hpp>
#include <third_party/boost/mpl/void.hpp>
#include <third_party/boost/mpl/aux_/na.hpp>
#include <third_party/boost/mpl/set/aux_/tag.hpp>
#include <third_party/boost/mpl/aux_/yes_no.hpp>
#include <third_party/boost/mpl/aux_/overload_names.hpp>
#include <third_party/boost/mpl/aux_/config/operators.hpp>

#include <third_party/boost/preprocessor/cat.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace mpl {

#if defined(BOOST_MPL_CFG_USE_OPERATORS_OVERLOADING)

#   define BOOST_MPL_AUX_SET0_OVERLOAD(R, f, X, T) \
    friend R BOOST_PP_CAT(BOOST_MPL_AUX_OVERLOAD_,f)(X const&, T) \
/**/

#   define BOOST_MPL_AUX_SET_OVERLOAD(R, f, X, T) \
    BOOST_MPL_AUX_SET0_OVERLOAD(R, f, X, T) \
/**/

#else

#   define BOOST_MPL_AUX_SET0_OVERLOAD(R, f, X, T) \
    static R BOOST_PP_CAT(BOOST_MPL_AUX_OVERLOAD_,f)(X const&, T) \
/**/

#   define BOOST_MPL_AUX_SET_OVERLOAD(R, f, X, T) \
    BOOST_MPL_AUX_SET0_OVERLOAD(R, f, X, T); \
    using Base::BOOST_PP_CAT(BOOST_MPL_AUX_OVERLOAD_,f) \
/**/

#endif

template< typename Dummy = na > struct set0
{
    typedef set0<>          item_;
    typedef item_           type;
    typedef aux::set_tag    tag;
    typedef void_           last_masked_;
    typedef void_           item_type_;
    typedef long_<0>        size;
    typedef long_<1>        order;

    BOOST_MPL_AUX_SET0_OVERLOAD( aux::no_tag, ORDER_BY_KEY, set0<>, void const volatile* );
    BOOST_MPL_AUX_SET0_OVERLOAD( aux::yes_tag, IS_MASKED, set0<>, void const volatile* );
};

}}

#endif // BOOST_MPL_SET_AUX_SET0_HPP_INCLUDED
