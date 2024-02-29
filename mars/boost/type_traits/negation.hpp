/*
Copyright 2020 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License,
Version 1.0. (See accompanying file LICENSE_1_0.txt
or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_TT_NEGATION_HPP_INCLUDED
#define BOOST_TT_NEGATION_HPP_INCLUDED

#include <boost/type_traits/integral_constant.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

template<class T>
struct negation
    : integral_constant<bool, !bool(T::value)> { };

} /* boost */

#endif
