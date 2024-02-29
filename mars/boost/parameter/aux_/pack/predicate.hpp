// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_PREDICATE_HPP
#define BOOST_PARAMETER_AUX_PACK_PREDICATE_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // helper for get_predicate<...>, below
    template <typename T>
    struct get_predicate_or_default
    {
        typedef T type;
    };

    // helper for predicate<...>, below
    template <typename T>
    struct get_predicate
      : ::mars_boost::parameter::aux
        ::get_predicate_or_default<typename T::predicate>
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/use_default.hpp>
#include <boost/parameter/aux_/always_true_predicate.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <>
    struct get_predicate_or_default< ::mars_boost::parameter::aux::use_default>
    {
        typedef ::mars_boost::parameter::aux::always_true_predicate type;
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/required.hpp>
#include <boost/parameter/optional.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>
#else
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using predicate = ::mars_boost::mp11::mp_if<
        ::mars_boost::mp11::mp_if<
            ::mars_boost::parameter::aux::is_optional<T>
          , ::mars_boost::mp11::mp_true
          , ::mars_boost::parameter::aux::is_required<T>
        >
      , ::mars_boost::parameter::aux::get_predicate<T>
      , ::mars_boost::mp11::mp_identity<
            ::mars_boost::parameter::aux::always_true_predicate
        >
    >;
#else
    struct predicate
      : ::mars_boost::mpl::eval_if<
            typename ::mars_boost::mpl::if_<
                ::mars_boost::parameter::aux::is_optional<T>
              , ::mars_boost::mpl::true_
              , ::mars_boost::parameter::aux::is_required<T>
            >::type
          , ::mars_boost::parameter::aux::get_predicate<T>
          , ::mars_boost::mpl::identity<
                ::mars_boost::parameter::aux::always_true_predicate
            >
        >
    {
    };
#endif  // BOOST_PARAMETER_CAN_USE_MP11
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

