// Copyright Cromwell D. Enage 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUGMENT_PREDICATE_HPP
#define BOOST_PARAMETER_AUGMENT_PREDICATE_HPP

#include <boost/parameter/keyword_fwd.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/type_traits/is_lvalue_reference.hpp>
#include <boost/type_traits/is_scalar.hpp>
#include <boost/type_traits/is_same.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename V, typename R, typename Tag>
    struct augment_predicate_check_consume_ref
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::is_scalar<V>
          , ::mars_boost::mpl::true_
          , ::mars_boost::mpl::eval_if<
                ::mars_boost::is_same<
                    typename Tag::qualifier
                  , ::mars_boost::parameter::consume_reference
                >
              , ::mars_boost::mpl::if_<
                    ::mars_boost::is_lvalue_reference<R>
                  , ::mars_boost::mpl::false_
                  , ::mars_boost::mpl::true_
                >
              , mars_boost::mpl::true_
            >
        >::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/type_traits/is_const.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename V, typename R, typename Tag>
    struct augment_predicate_check_out_ref
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::is_same<
                typename Tag::qualifier
              , ::mars_boost::parameter::out_reference
            >
          , ::mars_boost::mpl::eval_if<
                ::mars_boost::is_lvalue_reference<R>
              , ::mars_boost::mpl::if_<
                    ::mars_boost::is_const<V>
                  , ::mars_boost::mpl::false_
                  , ::mars_boost::mpl::true_
                >
              , ::mars_boost::mpl::false_
            >
          , ::mars_boost::mpl::true_
        >::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/lambda_tag.hpp>
#include <boost/mpl/apply_wrap.hpp>
#include <boost/mpl/lambda.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <
        typename Predicate
      , typename R
      , typename Tag
      , typename T
      , typename Args
    >
    class augment_predicate
    {
        typedef typename ::mars_boost::mpl::lambda<
            Predicate
          , ::mars_boost::parameter::aux::lambda_tag
        >::type _actual_predicate;

     public:
        typedef typename ::mars_boost::mpl::eval_if<
            typename ::mars_boost::mpl::if_<
                ::mars_boost::parameter::aux
                ::augment_predicate_check_consume_ref<T,R,Tag>
              , ::mars_boost::parameter::aux
                ::augment_predicate_check_out_ref<T,R,Tag>
              , ::mars_boost::mpl::false_
            >::type
          , ::mars_boost::mpl::apply_wrap2<_actual_predicate,T,Args>
          , ::mars_boost::mpl::false_
        >::type type;
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>
#include <type_traits>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename V, typename R, typename Tag>
    using augment_predicate_check_consume_ref_mp11 = ::mars_boost::mp11::mp_if<
        ::std::is_scalar<V>
      , ::mars_boost::mp11::mp_true
      , ::mars_boost::mp11::mp_if<
            ::std::is_same<
                typename Tag::qualifier
              , ::mars_boost::parameter::consume_reference
            >
          , ::mars_boost::mp11::mp_if<
                ::std::is_lvalue_reference<R>
              , ::mars_boost::mp11::mp_false
              , ::mars_boost::mp11::mp_true
            >
          , mars_boost::mp11::mp_true
        >
    >;

    template <typename V, typename R, typename Tag>
    using augment_predicate_check_out_ref_mp11 = ::mars_boost::mp11::mp_if<
        ::std::is_same<
            typename Tag::qualifier
          , ::mars_boost::parameter::out_reference
        >
      , ::mars_boost::mp11::mp_if<
            ::std::is_lvalue_reference<R>
          , ::mars_boost::mp11::mp_if<
                ::std::is_const<V>
              , ::mars_boost::mp11::mp_false
              , ::mars_boost::mp11::mp_true
            >
          , ::mars_boost::mp11::mp_false
        >
      , ::mars_boost::mp11::mp_true
    >;
}}} // namespace mars_boost::parameter::aux

#include <boost/mp11/list.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <
        typename Predicate
      , typename R
      , typename Tag
      , typename T
      , typename Args
    >
    struct augment_predicate_mp11_impl
    {
        using type = ::mars_boost::mp11::mp_if<
            ::mars_boost::mp11::mp_if<
                ::mars_boost::parameter::aux
                ::augment_predicate_check_consume_ref_mp11<T,R,Tag>
              , ::mars_boost::parameter::aux
                ::augment_predicate_check_out_ref_mp11<T,R,Tag>
              , ::mars_boost::mp11::mp_false
            >
          , ::mars_boost::mp11
            ::mp_apply_q<Predicate,::mars_boost::mp11::mp_list<T,Args> >
          , ::mars_boost::mp11::mp_false
        >;
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/has_nested_template_fn.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <
        typename Predicate
      , typename R
      , typename Tag
      , typename T
      , typename Args
    >
    using augment_predicate_mp11 = ::mars_boost::mp11::mp_if<
        ::mars_boost::parameter::aux::has_nested_template_fn<Predicate>
      , ::mars_boost::parameter::aux
        ::augment_predicate_mp11_impl<Predicate,R,Tag,T,Args>
      , ::mars_boost::parameter::aux
        ::augment_predicate<Predicate,R,Tag,T,Args>
    >;
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

