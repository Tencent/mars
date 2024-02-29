//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_ADDRESSOF_HPP
#define BOOST_CONTAINER_DETAIL_ADDRESSOF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/move/detail/addressof.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace container {
namespace dtl {

using mars_boost::move_detail::addressof;

}  //namespace dtl {
}  //namespace container {
}  //namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

#endif   //#ifndef BOOST_CONTAINER_DETAIL_ADDRESSOF_HPP
