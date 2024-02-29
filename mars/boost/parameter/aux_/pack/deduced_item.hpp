// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_DEDUCED_ITEM_HPP
#define BOOST_PARAMETER_AUX_PACK_DEDUCED_ITEM_HPP

#include <boost/parameter/aux_/void.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // A typelist that stored deduced parameter specs.
    template <
        typename ParameterSpec
      , typename Tail = ::mars_boost::parameter::void_
    >
    struct deduced_item
    {
        typedef ParameterSpec spec;
        typedef Tail tail;
    };

    // Evaluate Tail and construct deduced_item list.
    template <typename Spec, typename Tail>
    struct make_deduced_item
    {
        typedef ::mars_boost::parameter::aux
        ::deduced_item<Spec,typename Tail::type> type;
    };
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

