// Copyright Daniel Wallin, David Abrahams 2005.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_UNWRAP_CV_REFERENCE_HPP
#define BOOST_PARAMETER_AUX_UNWRAP_CV_REFERENCE_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

    template <typename T>
    class reference_wrapper;
} // namespace mars_boost

#include <boost/parameter/aux_/yesno.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    //
    // reference_wrapper support -- if perfect forwarding is unsupported,
    // then when passing arguments positionally by non-const reference,
    // we ask users of named parameter interfaces to use ref(x) to wrap them.
    //

    template <typename U>
    ::mars_boost::parameter::aux::yes_tag
        is_cv_reference_wrapper_check(
            ::mars_boost::reference_wrapper<U> const volatile*
        );

    ::mars_boost::parameter::aux::no_tag is_cv_reference_wrapper_check(...);
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/config.hpp>

#if !defined(BOOST_NO_CXX11_HDR_FUNCTIONAL)
#include <functional>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // Support for std::ref(x) -- Cromwell D. Enage
    template <typename U>
    ::mars_boost::parameter::aux::yes_tag
        is_cv_reference_wrapper_check(
            ::std::reference_wrapper<U> const volatile*
        );
}}} // namespace mars_boost::parameter::aux
#endif

#include <boost/parameter/aux_/preprocessor/nullptr.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11) && !( \
        BOOST_WORKAROUND(BOOST_MSVC, >= 1900) && \
        BOOST_WORKAROUND(BOOST_MSVC, < 1910) \
    )
#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>
#include <type_traits>
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11) || MSVC-14.0
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/remove_reference.hpp>
#if !BOOST_WORKAROUND(BOOST_BORLANDC, BOOST_TESTED_AT(0x564)) && \
    !BOOST_WORKAROUND(BOOST_GCC, < 40000)
#include <boost/mpl/eval_if.hpp>
#endif
#endif  // BOOST_PARAMETER_CAN_USE_MP11 && not MSVC-14.0

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

#if defined(BOOST_PARAMETER_CAN_USE_MP11) && !( \
        BOOST_WORKAROUND(BOOST_MSVC, >= 1900) && \
        BOOST_WORKAROUND(BOOST_MSVC, < 1910) \
    )
    // This metafunction returns mp11::mp_true if T is of type
    // reference_wrapper<U> cv.
    template <typename T>
    using is_cv_reference_wrapper = ::mars_boost::mp11::mp_bool<
        sizeof(
            ::mars_boost::parameter::aux::is_cv_reference_wrapper_check(
                static_cast<
                    typename ::std::remove_reference<T>::type*
                >(BOOST_PARAMETER_AUX_PP_NULLPTR)
            )
        ) == sizeof(::mars_boost::parameter::aux::yes_tag)
    >;

    // Needed for unwrap_cv_reference below. T might be const, so
    // mp_eval_if<> might fail because of deriving from T const on EDG.
    template <typename T>
    using unwrap_cv_reference_impl = typename ::std::remove_reference<T>::type;

    // Produces the unwrapped type to hold a reference to in
    // tagged_argument<>.  Can't use mars_boost::unwrap_reference<> here
    // because it doesn't handle the case where T = reference_wrapper<U> cv.
    template <typename T>
    using unwrap_cv_reference = ::mars_boost::mp11::mp_eval_if<
        ::mars_boost::parameter::aux::is_cv_reference_wrapper<T>
      , ::mars_boost::parameter::aux::unwrap_cv_reference_impl<T>
      , ::std::remove_reference
      , T
    >;
#else  // !defined(BOOST_PARAMETER_CAN_USE_MP11) || MSVC-14.0
    // This metafunction returns mpl::true_ if T is of type
    // reference_wrapper<U> cv.
    template <typename T>
    struct is_cv_reference_wrapper
    {
        BOOST_STATIC_CONSTANT(
            bool, value = (
                sizeof(
                    ::mars_boost::parameter::aux::is_cv_reference_wrapper_check(
                        static_cast<
                            typename ::mars_boost::remove_reference<T>::type*
                        >(BOOST_PARAMETER_AUX_PP_NULLPTR)
                    )
                ) == sizeof(::mars_boost::parameter::aux::yes_tag)
            )
        );

        typedef mars_boost::mpl::bool_<
#if BOOST_WORKAROUND(BOOST_BORLANDC, BOOST_TESTED_AT(0x564))
            is_cv_reference_wrapper::
#endif
        value> type;
    };

#if BOOST_WORKAROUND(BOOST_BORLANDC, BOOST_TESTED_AT(0x564)) || \
    BOOST_WORKAROUND(BOOST_GCC, < 40000)
    template <
        typename T
      , typename = typename ::mars_boost::parameter::aux
        ::is_cv_reference_wrapper<T>::type
    >
    struct unwrap_cv_reference : ::mars_boost::remove_reference<T>
    {
    };

    template <typename T>
    struct unwrap_cv_reference<T const,::mars_boost::mpl::false_>
    {
        typedef T const type;
    };

    template <typename T>
    struct unwrap_cv_reference<T,::mars_boost::mpl::true_> : T
    {
    };
#else   // no Borland or GCC 3- workarounds needed
    // Needed for unwrap_cv_reference below. T might be const, so
    // eval_if<> might fail because of deriving from T const on EDG.
    template <typename T>
    struct unwrap_cv_reference_impl : ::mars_boost::remove_reference<T>::type
    {
    };

    // Produces the unwrapped type to hold a reference to in
    // tagged_argument<>.  Can't use mars_boost::unwrap_reference<> here
    // because it doesn't handle the case where T = reference_wrapper<U> cv.
    template <typename T>
    struct unwrap_cv_reference
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::parameter::aux::is_cv_reference_wrapper<T>
          , ::mars_boost::parameter::aux::unwrap_cv_reference_impl<T>
          , ::mars_boost::remove_reference<T>
        >
    {
    };
#endif  // Borland or GCC 3- workarounds needed
#endif  // BOOST_PARAMETER_CAN_USE_MP11 && not MSVC-14.0
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

