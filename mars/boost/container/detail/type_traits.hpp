//////////////////////////////////////////////////////////////////////////////
// (C) Copyright John Maddock 2000.
// (C) Copyright Ion Gaztanaga 2005-2015.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
// The alignment and Type traits implementation comes from
// John Maddock's TypeTraits library.
//
// Some other tricks come from Howard Hinnant's papers and StackOverflow replies
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_TYPE_TRAITS_HPP
#define BOOST_CONTAINER_CONTAINER_DETAIL_TYPE_TRAITS_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/move/detail/type_traits.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace container {
namespace container_detail {

using ::mars_boost::move_detail::enable_if;
using ::mars_boost::move_detail::enable_if_and;
using ::mars_boost::move_detail::is_same;
using ::mars_boost::move_detail::is_different;
using ::mars_boost::move_detail::is_pointer;
using ::mars_boost::move_detail::add_reference;
using ::mars_boost::move_detail::add_const;
using ::mars_boost::move_detail::add_const_reference;
using ::mars_boost::move_detail::remove_const;
using ::mars_boost::move_detail::remove_reference;
using ::mars_boost::move_detail::make_unsigned;
using ::mars_boost::move_detail::is_floating_point;
using ::mars_boost::move_detail::is_integral;
using ::mars_boost::move_detail::is_enum;
using ::mars_boost::move_detail::is_pod;
using ::mars_boost::move_detail::is_empty;
using ::mars_boost::move_detail::is_trivially_destructible;
using ::mars_boost::move_detail::is_trivially_default_constructible;
using ::mars_boost::move_detail::is_trivially_copy_constructible;
using ::mars_boost::move_detail::is_trivially_move_constructible;
using ::mars_boost::move_detail::is_trivially_copy_assignable;
using ::mars_boost::move_detail::is_trivially_move_assignable;
using ::mars_boost::move_detail::is_nothrow_default_constructible;
using ::mars_boost::move_detail::is_nothrow_copy_constructible;
using ::mars_boost::move_detail::is_nothrow_move_constructible;
using ::mars_boost::move_detail::is_nothrow_copy_assignable;
using ::mars_boost::move_detail::is_nothrow_move_assignable;
using ::mars_boost::move_detail::is_nothrow_swappable;
using ::mars_boost::move_detail::alignment_of;
using ::mars_boost::move_detail::aligned_storage;
using ::mars_boost::move_detail::nat;
using ::mars_boost::move_detail::max_align_t;

}  //namespace container_detail {
}  //namespace container {
}  //namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

#endif   //#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_TYPE_TRAITS_HPP
