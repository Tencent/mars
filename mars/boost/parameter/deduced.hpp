// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_DEDUCED_HPP
#define BOOST_PARAMETER_DEDUCED_HPP

#include <boost/parameter/aux_/use_default.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter {

    // This metafunction can be used to describe the treatment of particular
    // named parameters for the purposes of overload elimination with SFINAE,
    // by placing specializations in the parameters<...> list.
    //
    // If a keyword k is specified with deduced<...>, that keyword
    // will be automatically deduced from the argument list.
    template <typename Tag>
    struct deduced
    {
        typedef Tag key_type;
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
    struct is_deduced_aux
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_false
#else
      : ::mars_boost::mpl::false_
#endif
    {
    };

    template <typename Tag>
    struct is_deduced_aux< ::mars_boost::parameter::deduced<Tag> >
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_true
#else
      : ::mars_boost::mpl::true_
#endif
    {
    };

    template <typename T>
    struct is_deduced0
      : ::mars_boost::parameter::aux::is_deduced_aux<typename T::key_type>::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/required.hpp>
#include <boost/parameter/optional.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/utility.hpp>
#else
#include <boost/mpl/if.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    //
    // tag_type, has_default, and predicate --
    //
    // These metafunctions accept a ParameterSpec and extract the
    // keyword tag, whether or not a default is supplied for the
    // parameter, and the predicate that the corresponding actual
    // argument type is required match.
    //
    // a ParameterSpec is a specialization of either keyword<...>,
    // required<...>, optional<...>
    //

    template <typename T>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using has_default = ::mars_boost::mp11::mp_if<
        ::mars_boost::parameter::aux::is_required<T>
      , ::mars_boost::mp11::mp_false
      , ::mars_boost::mp11::mp_true
    >;
#else
    struct has_default
      : ::mars_boost::mpl::if_<
            ::mars_boost::parameter::aux::is_required<T>
          , ::mars_boost::mpl::false_
          , ::mars_boost::mpl::true_
        >::type
    {
    };
#endif

    template <typename T>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using is_deduced = ::mars_boost::mp11::mp_if<
        ::mars_boost::mp11::mp_if<
            ::mars_boost::parameter::aux::is_optional<T>
          , ::mars_boost::mp11::mp_true
          , ::mars_boost::parameter::aux::is_required<T>
        >
      , ::mars_boost::parameter::aux::is_deduced0<T>
      , ::mars_boost::mp11::mp_false
    >;
#else
    struct is_deduced
      : ::mars_boost::mpl::if_<
            typename ::mars_boost::mpl::if_<
                ::mars_boost::parameter::aux::is_optional<T>
              , ::mars_boost::mpl::true_
              , ::mars_boost::parameter::aux::is_required<T>
            >::type
          , ::mars_boost::parameter::aux::is_deduced0<T>
          , ::mars_boost::mpl::false_
        >::type
    {
    };
#endif  // BOOST_PARAMETER_CAN_USE_MP11
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

