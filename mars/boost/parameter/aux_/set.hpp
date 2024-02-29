// Copyright Daniel Wallin 2006.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_SET_060912_HPP
#define BOOST_PARAMETER_SET_060912_HPP

#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <boost/mp11/list.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    typedef ::mars_boost::mp11::mp_list<> set0;
}}} // namespace mars_boost::parameter::aux

#include <boost/mp11/algorithm.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename S, typename K>
    struct insert_
    {
        using type = ::mars_boost::mp11::mp_insert_c<S,0,K>;
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/mp11/integral.hpp>
#include <boost/mp11/utility.hpp>
#include <type_traits>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Set, typename K>
    struct has_key_
    {
        using type = ::mars_boost::mp11::mp_if<
            ::mars_boost::mp11::mp_empty<Set>
          , ::mars_boost::mp11::mp_false
          , ::std::is_same<
                ::mars_boost::mp11::mp_find<Set,K>
              , ::mars_boost::mp11::mp_size<Set>
            >
        >;
    };
}}} // namespace mars_boost::parameter::aux

#elif BOOST_WORKAROUND(BOOST_BORLANDC, BOOST_TESTED_AT(0x564))
#include <boost/mpl/list.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    typedef ::mars_boost::mpl::list0<> set0;
}}} // namespace mars_boost::parameter::aux

#include <boost/mpl/push_front.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Set, typename K>
    struct insert_ : ::mars_boost::mpl::push_front<Set,K>
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/mpl/bool.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Set, typename K>
    struct has_key_
    {
        typedef typename ::mars_boost::mpl::find<Set,K>::type iter;
        typedef typename ::mars_boost::mpl::if_<
            ::mars_boost::is_same<iter,typename ::mars_boost::mpl::end<Set>::type>
          , ::mars_boost::mpl::false_
          , ::mars_boost::mpl::true_
        >::type type;
    };
}}} // namespace mars_boost::parameter::aux

#else   // !BOOST_PARAMETER_CAN_USE_MP11 && Borland workarounds not needed
#include <boost/mpl/set/set0.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    typedef ::mars_boost::mpl::set0<> set0;
}}} // namespace mars_boost::parameter::aux

#include <boost/mpl/insert.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Set, typename K>
    struct insert_ : ::mars_boost::mpl::insert<Set,K>
    {
    };
}}} // namespace mars_boost::parameter::aux

#include <boost/mpl/has_key.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Set, typename K>
    struct has_key_ : ::mars_boost::mpl::has_key<Set,K>
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11 || Borland workarounds needed
#endif  // include guard

