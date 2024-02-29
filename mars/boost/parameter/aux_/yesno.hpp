// Copyright Daniel Wallin, David Abrahams 2005.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_YESNO_HPP
#define BOOST_PARAMETER_AUX_YESNO_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // types used with the "sizeof trick" to capture the results of
    // overload resolution at compile-time.
    typedef char yes_tag;
    typedef char (&no_tag)[2];
}}} // namespace mars_boost::parameter::aux

#include <boost/mpl/bool.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // mpl::true_ and mpl::false_ are not distinguishable by sizeof(),
    // so we pass them through these functions to get a type that is.
    ::mars_boost::parameter::aux::yes_tag to_yesno(::mars_boost::mpl::true_);
    ::mars_boost::parameter::aux::no_tag to_yesno(::mars_boost::mpl::false_);
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/integral.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // mp11::mp_true and mp11::mp_false are not distinguishable by sizeof(),
    // so we pass them through these functions to get a type that is.
    ::mars_boost::parameter::aux::yes_tag to_yesno(::mars_boost::mp11::mp_true);
    ::mars_boost::parameter::aux::no_tag to_yesno(::mars_boost::mp11::mp_false);
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

