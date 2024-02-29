// Copyright Daniel Wallin 2006.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_TEMPLATE_KEYWORD_060203_HPP
#define BOOST_PARAMETER_TEMPLATE_KEYWORD_060203_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    struct template_keyword_base
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <type_traits>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    using is_template_keyword = ::std::is_base_of<
        ::mars_boost::parameter::aux::template_keyword_base
      , typename ::std::remove_const<
            typename ::std::remove_reference<T>::type
        >::type
    >;
}}} // namespace mars_boost::parameter::aux

#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/remove_const.hpp>

#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/remove_reference.hpp>
#else
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_lvalue_reference.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

#if !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
    template <typename T>
    struct is_template_keyword_aux
      : ::mars_boost::mpl::if_<
            ::mars_boost::is_convertible<
                T*
              , ::mars_boost::parameter::aux::template_keyword_base const*
            >
          , ::mars_boost::mpl::true_
          , ::mars_boost::mpl::false_
        >::type
    {
    };
#endif  // BOOST_PARAMETER_HAS_PERFECT_FORWARDING

    template <typename T>
    struct is_template_keyword
      : ::mars_boost::mpl::if_<
#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
            // Cannot use is_convertible<> to check if T is derived from
            // template_keyword_base. -- Cromwell D. Enage
            ::mars_boost::is_base_of<
                ::mars_boost::parameter::aux::template_keyword_base
              , typename ::mars_boost::remove_const<
                    typename ::mars_boost::remove_reference<T>::type
                >::type
            >
          , ::mars_boost::mpl::true_
          , ::mars_boost::mpl::false_
#else
            ::mars_boost::is_lvalue_reference<T>
          , ::mars_boost::mpl::false_
          , ::mars_boost::parameter::aux::is_template_keyword_aux<T>
#endif  // BOOST_PARAMETER_HAS_PERFECT_FORWARDING
        >::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

