// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_MAKE_PARAMETER_SPEC_ITEMS_HPP
#define BOOST_PARAMETER_AUX_PACK_MAKE_PARAMETER_SPEC_ITEMS_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // This recursive metafunction forwards successive elements of
    // parameters::parameter_spec to make_deduced_items<>.
    // -- Cromwell D. Enage
    template <typename SpecSeq>
    struct make_deduced_list;

    // Helper for match_parameters_base_cond<...>, below.
    template <typename ArgumentPackAndError, typename SpecSeq>
    struct match_parameters_base_cond_helper;

    // Helper metafunction for make_parameter_spec_items<...>, below.
    template <typename SpecSeq, typename ...Args>
    struct make_parameter_spec_items_helper;
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/void.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename SpecSeq>
    struct make_parameter_spec_items_helper<SpecSeq>
    {
        typedef ::mars_boost::parameter::void_ type;
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/pack/make_deduced_items.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/list.hpp>
#else
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename SpecSeq>
    struct make_deduced_list_not_empty
      : ::mars_boost::parameter::aux::make_deduced_items<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::mars_boost::mp11::mp_front<SpecSeq>
#else
            typename ::mars_boost::mpl::front<SpecSeq>::type
#endif
          , ::mars_boost::parameter::aux::make_deduced_list<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
                ::mars_boost::mp11::mp_pop_front<SpecSeq>
#else
                typename ::mars_boost::mpl::pop_front<SpecSeq>::type
#endif
            >
        >
    {
    };
}}} // namespace mars_boost::parameter::aux

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/utility.hpp>
#else
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/identity.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename SpecSeq>
    struct make_deduced_list
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_if<
            ::mars_boost::mp11::mp_empty<SpecSeq>
          , ::mars_boost::mp11::mp_identity< ::mars_boost::parameter::void_>
#else
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::mpl::empty<SpecSeq>
          , ::mars_boost::mpl::identity< ::mars_boost::parameter::void_>
#endif
          , ::mars_boost::parameter::aux::make_deduced_list_not_empty<SpecSeq>
        >
    {
    };
}}} // namespace mars_boost::parameter::aux

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <type_traits>
#else
#include <boost/mpl/bool.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename ArgumentPackAndError>
    struct is_arg_pack_error_void
      : ::mars_boost::mpl::if_<
            ::mars_boost::is_same<
                typename ::mars_boost::mpl::second<ArgumentPackAndError>::type
              , ::mars_boost::parameter::void_
            >
          , ::mars_boost::mpl::true_
          , ::mars_boost::mpl::false_
        >::type
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // Checks if the arguments match the criteria of overload resolution.
    // If NamedList satisfies the PS0, PS1, ..., this is a metafunction
    // returning parameters.  Otherwise it has no nested ::type.
    template <typename ArgumentPackAndError, typename SpecSeq>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using match_parameters_base_cond = ::mars_boost::mp11::mp_if<
        ::mars_boost::mp11::mp_empty<SpecSeq>
      , ::std::is_same<
            ::mars_boost::mp11::mp_at_c<ArgumentPackAndError,1>
          , ::mars_boost::parameter::void_
        >
      , ::mars_boost::parameter::aux::match_parameters_base_cond_helper<
            ArgumentPackAndError
          , SpecSeq
        >
    >;
#else
    struct match_parameters_base_cond
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::mpl::empty<SpecSeq>
          , ::mars_boost::parameter::aux
            ::is_arg_pack_error_void<ArgumentPackAndError>
          , ::mars_boost::parameter::aux::match_parameters_base_cond_helper<
                ArgumentPackAndError
              , SpecSeq
            >
        >
    {
    };
#endif  // BOOST_PARAMETER_CAN_USE_MP11
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/pack/satisfies.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename ArgumentPackAndError, typename SpecSeq>
    struct match_parameters_base_cond_helper
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::mars_boost::mp11::mp_if<
#else
      : ::mars_boost::mpl::eval_if<
#endif
            ::mars_boost::parameter::aux::satisfies_requirements_of<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
                ::mars_boost::mp11::mp_at_c<ArgumentPackAndError,0>
              , ::mars_boost::mp11::mp_front<SpecSeq>
#else
                typename ::mars_boost::mpl::first<ArgumentPackAndError>::type
              , typename ::mars_boost::mpl::front<SpecSeq>::type
#endif
            >
          , ::mars_boost::parameter::aux::match_parameters_base_cond<
                ArgumentPackAndError
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
              , ::mars_boost::mp11::mp_pop_front<SpecSeq>
#else
              , typename ::mars_boost::mpl::pop_front<SpecSeq>::type
#endif
            >
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
          , ::mars_boost::mp11::mp_false
#else
          , ::mars_boost::mpl::false_
#endif
        >
    {
    };

    // This parameters item chaining metafunction class does not require
    // the lengths of the SpecSeq and of Args parameter pack to match.
    // Used by argument_pack to build the items in the resulting arg_list.
    // -- Cromwell D. Enage
    template <typename SpecSeq, typename ...Args>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using make_parameter_spec_items = ::mars_boost::mp11::mp_if<
        ::mars_boost::mp11::mp_empty<SpecSeq>
      , ::mars_boost::mp11::mp_identity< ::mars_boost::parameter::void_>
      , ::mars_boost::parameter::aux
        ::make_parameter_spec_items_helper<SpecSeq,Args...>
    >;
#else
    struct make_parameter_spec_items
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::mpl::empty<SpecSeq>
          , ::mars_boost::mpl::identity< ::mars_boost::parameter::void_>
          , ::mars_boost::parameter::aux
            ::make_parameter_spec_items_helper<SpecSeq,Args...>
        >
    {
    };
#endif
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/aux_/pack/make_items.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename SpecSeq, typename A0, typename ...Args>
    struct make_parameter_spec_items_helper<SpecSeq,A0,Args...>
      : ::mars_boost::parameter::aux::make_items<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
            ::mars_boost::mp11::mp_front<SpecSeq>
#else
            typename ::mars_boost::mpl::front<SpecSeq>::type
#endif
          , A0
          , ::mars_boost::parameter::aux::make_parameter_spec_items<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
                ::mars_boost::mp11::mp_pop_front<SpecSeq>
#else
                typename ::mars_boost::mpl::pop_front<SpecSeq>::type
#endif
              , Args...
            >
        >
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

