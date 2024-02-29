// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_TAG_KEYWORD_ARG_REF_HPP
#define BOOST_PARAMETER_AUX_PACK_TAG_KEYWORD_ARG_REF_HPP

#include <boost/parameter/aux_/unwrap_cv_reference.hpp>
#include <boost/parameter/aux_/tagged_argument.hpp>
#include <boost/parameter/config.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <
        typename Keyword
      , typename ActualArg
#if BOOST_WORKAROUND(BOOST_BORLANDC, BOOST_TESTED_AT(0x564))
      , typename = typename ::mars_boost::parameter::aux
        ::is_cv_reference_wrapper<ActualArg>::type
#endif
    >
    struct tag_ref
    {
        typedef ::mars_boost::parameter::aux::tagged_argument<
            Keyword
          , typename ::mars_boost::parameter::aux
            ::unwrap_cv_reference<ActualArg>::type
        > type;
    };
}}} // namespace mars_boost::parameter::aux_

#if BOOST_WORKAROUND(BOOST_BORLANDC, BOOST_TESTED_AT(0x564))

#include <boost/mpl/bool.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Keyword, typename ActualArg>
    struct tag_ref<Keyword,ActualArg,::mars_boost::mpl::false_>
    {
        typedef ::mars_boost::parameter::aux
        ::tagged_argument<Keyword,ActualArg> type;
    };
}}} // namespace mars_boost::parameter::aux_

#endif  // Borland workarounds needed.

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    struct tag_keyword_arg_ref
    {
        template <typename K, typename T>
        struct apply
        {
            typedef typename ::mars_boost::parameter::aux::tag_ref<K,T>::type type;
        };

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        template <typename K, typename T>
        using fn = typename ::mars_boost::parameter::aux::tag_ref<K,T>::type;
#endif
    };
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

