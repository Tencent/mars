// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_TAG_TYPE_HPP
#define BOOST_PARAMETER_AUX_PACK_TAG_TYPE_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // helper for tag_type<...>, below.
    template <typename T>
    struct get_tag_type0
    {
        typedef typename T::key_type type;
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/deduced.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/utility.hpp>
#else
#include <boost/mpl/eval_if.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    struct get_tag_type
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_if<
#else
      : ::mars_boost::mpl::eval_if<
#endif
            ::mars_boost::parameter::aux::is_deduced0<T>
          , ::mars_boost::parameter::aux::get_tag_type0<typename T::key_type>
          , ::mars_boost::parameter::aux::get_tag_type0<T>
        >
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/required.hpp>
#include <boost/parameter/optional.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/integral.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    using tag_type = ::mars_boost::mp11::mp_if<
        ::mars_boost::mp11::mp_if<
            ::mars_boost::parameter::aux::is_optional<T>
          , ::mars_boost::mp11::mp_true
          , ::mars_boost::parameter::aux::is_required<T>
        >
      , ::mars_boost::parameter::aux::get_tag_type<T>
      , ::mars_boost::mp11::mp_identity<T>
    >;
}}} // namespace mars_boost::parameter::aux

#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/identity.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
    struct tag_type
      : ::mars_boost::mpl::eval_if<
            typename ::mars_boost::mpl::if_<
                ::mars_boost::parameter::aux::is_optional<T>
              , ::mars_boost::mpl::true_
              , ::mars_boost::parameter::aux::is_required<T>
            >::type
          , ::mars_boost::parameter::aux::get_tag_type<T>
          , ::mars_boost::mpl::identity<T>
        >
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

