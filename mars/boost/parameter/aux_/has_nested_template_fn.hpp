// Copyright Cromwell D. Enage 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_HAS_NESTED_TEMPLATE_FN_HPP
#define BOOST_PARAMETER_AUX_HAS_NESTED_TEMPLATE_FN_HPP

#include <boost/parameter/aux_/yesno.hpp>
#include <boost/parameter/aux_/preprocessor/nullptr.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>
#else
#include <boost/mpl/bool.hpp>
#include <boost/mpl/identity.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    template <template <typename ...> class F>
    struct has_nested_template_fn_variadic
    {
    };
#else
    template <template <typename P0, typename P1> class F>
    struct has_nested_template_fn_arity_2
    {
    };
#endif

    template <typename T>
    class has_nested_template_fn_impl
    {
        template <typename U>
        static ::mars_boost::parameter::aux::no_tag _check(...);

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        template <typename U>
        static ::mars_boost::parameter::aux::yes_tag
            _check(
                ::mars_boost::mp11::mp_identity<U> const volatile*
              , ::mars_boost::parameter::aux::has_nested_template_fn_variadic<
                    U::template fn
                >* = BOOST_PARAMETER_AUX_PP_NULLPTR
            );
#else
        template <typename U>
        static BOOST_CONSTEXPR ::mars_boost::parameter::aux::yes_tag
            _check(
                ::mars_boost::mpl::identity<U> const volatile*
              , ::mars_boost::parameter::aux::has_nested_template_fn_arity_2<
                    U::template fn
                >* = BOOST_PARAMETER_AUX_PP_NULLPTR
            );
#endif

     public:
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using type = ::mars_boost::mp11::mp_bool<
#else
        typedef ::mars_boost::mpl::bool_<
#endif
            sizeof(
                ::mars_boost::parameter::aux::has_nested_template_fn_impl<T>
                ::template _check<T>(
                    static_cast<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
                        ::mars_boost::mp11::mp_identity<T> const volatile*
#else
                        ::mars_boost::mpl::identity<T> const volatile*
#endif
                    >(BOOST_PARAMETER_AUX_PP_NULLPTR)
                )
            ) == sizeof(::mars_boost::parameter::aux::yes_tag)
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        >;
#else
        > type;
#endif
    };
}}} // namespace mars_boost::parameter::aux

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <type_traits>
#else
#include <boost/type_traits/remove_const.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename T>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using has_nested_template_fn = typename ::mars_boost::parameter::aux
    ::has_nested_template_fn_impl<typename ::std::remove_const<T>::type>
    ::type;
#else
    struct has_nested_template_fn
      : ::mars_boost::parameter::aux::has_nested_template_fn_impl<
            typename ::mars_boost::remove_const<T>::type
        >::type
    {
    };
#endif
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

