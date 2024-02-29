// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_MAKE_ITEMS_HPP
#define BOOST_PARAMETER_AUX_PACK_MAKE_ITEMS_HPP

#include <boost/parameter/aux_/void.hpp>
#include <boost/parameter/aux_/pack/item.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/utility.hpp>
#include <type_traits>
#else
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // Creates a item typelist.
    template <typename Spec, typename Arg, typename Tail>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    using make_items = ::mars_boost::mp11::mp_if<
        ::std::is_same<Arg,::mars_boost::parameter::void_>
      , ::mars_boost::mp11::mp_identity< ::mars_boost::parameter::void_>
      , ::mars_boost::parameter::aux::make_item<Spec,Arg,Tail>
    >;
#else
    struct make_items
      : ::mars_boost::mpl::eval_if<
            ::mars_boost::is_same<Arg,::mars_boost::parameter::void_>
          , ::mars_boost::mpl::identity< ::mars_boost::parameter::void_>
          , ::mars_boost::parameter::aux::make_item<Spec,Arg,Tail>
        >
    {
    };
#endif
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

