// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_REQUIRED_HPP
#define BOOST_PARAMETER_REQUIRED_HPP

#include <boost/parameter/aux_/use_default.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter {

    // This metafunction can be used to describe the treatment of particular
    // named parameters for the purposes of overload elimination with SFINAE,
    // by placing specializations in the parameters<...> list.  In order for
    // a treated function to participate in overload resolution:
    //
    //   - all keyword tags wrapped in required<...> must have a matching
    //     actual argument
    //
    //   - The actual argument type matched by every keyword tag
    //     associated with a predicate must satisfy that predicate
    template <
        typename Tag
      , typename Predicate = ::mars_boost::parameter::aux::use_default
    >
    struct required
    {
        typedef Tag key_type;
        typedef Predicate predicate;
    };
}}

#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/integral.hpp>
#else
#include <boost/mpl/bool.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    struct is_required
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_false
#else
      : ::mars_boost::mpl::false_
#endif
    {
    };

    template <typename Tag, typename Predicate>
    struct is_required< ::mars_boost::parameter::required<Tag,Predicate> >
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_true
#else
      : ::mars_boost::mpl::true_
#endif
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

