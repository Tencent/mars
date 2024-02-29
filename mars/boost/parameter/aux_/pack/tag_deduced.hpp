// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_TAG_DEDUCED_HPP
#define BOOST_PARAMETER_AUX_PACK_TAG_DEDUCED_HPP

#include <boost/parameter/aux_/set.hpp>
#include <boost/parameter/aux_/pack/tag_type.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/list.hpp>
#include <boost/mp11/utility.hpp>
#else
#include <boost/mpl/pair.hpp>
#include <boost/mpl/apply_wrap.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // Tags a deduced argument Arg with the keyword tag of Spec using TagFn.
    // Returns the tagged argument and the mpl::set<> UsedArgs with the
    // tag of Spec inserted.
    template <typename UsedArgs, typename Spec, typename Arg, typename TagFn>
    struct tag_deduced
    {
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using type = ::mars_boost::mp11::mp_list<
            ::mars_boost::mp11::mp_apply_q<
                TagFn
              , ::mars_boost::mp11::mp_list<
                    typename ::mars_boost::parameter::aux::tag_type<Spec>::type
                  , Arg
                >
            >
#else
        typedef ::mars_boost::mpl::pair<
            typename ::mars_boost::mpl::apply_wrap2<
                TagFn
              , typename ::mars_boost::parameter::aux::tag_type<Spec>::type
              , Arg
            >::type
#endif  // BOOST_PARAMETER_CAN_USE_MP11
          , typename ::mars_boost::parameter::aux::insert_<
                UsedArgs
              , typename ::mars_boost::parameter::aux::tag_type<Spec>::type
            >::type
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        >;
#else
        > type;
#endif
    };
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

