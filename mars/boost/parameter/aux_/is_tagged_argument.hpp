// Copyright Daniel Wallin, David Abrahams 2005.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_IS_TAGGED_ARGUMENT_HPP
#define BOOST_PARAMETER_IS_TAGGED_ARGUMENT_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    struct tagged_argument_base
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>

#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING) || \
    (0 < BOOST_PARAMETER_EXPONENTIAL_OVERLOAD_THRESHOLD_ARITY)
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // This metafunction identifies tagged_argument specializations
    // and their derived classes.
    template <typename T>
    struct is_tagged_argument
      : ::mars_boost::mpl::if_<
            // Cannot use is_convertible<> to check if T is derived from
            // tagged_argument_base. -- Cromwell D. Enage
            ::mars_boost::is_base_of<
                ::mars_boost::parameter::aux::tagged_argument_base
              , typename ::mars_boost::remove_const<
                    typename ::mars_boost::remove_reference<T>::type
                >::type
            >
          , ::mars_boost::mpl::true_
          , ::mars_boost::mpl::false_
        >::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#else   // no perfect forwarding support and no exponential overloads
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_lvalue_reference.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    struct is_tagged_argument_aux
      : ::mars_boost::is_convertible<
            T*
          , ::mars_boost::parameter::aux::tagged_argument_base const*
        >
    {
    };

    // This metafunction identifies tagged_argument specializations
    // and their derived classes.
    template <typename T>
    struct is_tagged_argument
      : ::mars_boost::mpl::if_<
            ::mars_boost::is_lvalue_reference<T>
          , ::mars_boost::mpl::false_
          , ::mars_boost::parameter::aux::is_tagged_argument_aux<T>
        >::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // perfect forwarding support, or exponential overloads

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <type_traits>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    using is_tagged_argument_mp11 = ::std::is_base_of<
        ::mars_boost::parameter::aux::tagged_argument_base
      , typename ::std::remove_const<
            typename ::std::remove_reference<T>::type
        >::type
    >;
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

