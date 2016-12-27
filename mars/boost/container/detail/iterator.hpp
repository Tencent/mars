//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_ITERATOR_HPP
#define BOOST_CONTAINER_DETAIL_ITERATOR_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/intrusive/detail/iterator.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace container {

using ::mars_boost::intrusive::iterator_traits;
using ::mars_boost::intrusive::iterator_distance;
using ::mars_boost::intrusive::iterator_advance;
using ::mars_boost::intrusive::iterator;
using ::mars_boost::intrusive::iterator_enable_if_tag;
using ::mars_boost::intrusive::iterator_disable_if_tag;
using ::mars_boost::intrusive::iterator_arrow_result;

}  //namespace container {
}  //namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

#endif   //#ifndef BOOST_CONTAINER_DETAIL_ITERATORS_HPP
