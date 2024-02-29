// Copyright Daniel Wallin, David Abrahams 2005.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_TAGGED_ARGUMENT_FWD_HPP
#define BOOST_PARAMETER_TAGGED_ARGUMENT_FWD_HPP

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Keyword, typename Arg>
    class tagged_argument;
}}} // namespace mars_boost::parameter::aux

#include <boost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename Keyword, typename Arg>
    struct tagged_argument_rref;
}}} // namespace mars_boost::parameter::aux

#endif

#if defined(BOOST_PARAMETER_CAN_USE_MP11)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    template <typename TaggedArg>
    struct tagged_argument_list_of_1;
}}} // namespace mars_boost::parameter::aux

#endif
#endif  // include guard

