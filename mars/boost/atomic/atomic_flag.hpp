/*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Copyright (c) 2011 Helge Bahmann
 * Copyright (c) 2013 Tim Blechmann
 * Copyright (c) 2014 Andrey Semashev
 */
/*!
 * \file   atomic/atomic_flag.hpp
 *
 * This header contains definition of \c atomic_flag.
 */

#ifndef BOOST_ATOMIC_ATOMIC_FLAG_HPP_INCLUDED_
#define BOOST_ATOMIC_ATOMIC_FLAG_HPP_INCLUDED_

#include <boost/atomic/capabilities.hpp>
#include <boost/atomic/detail/operations.hpp>
#include <boost/atomic/detail/atomic_flag.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

using atomics::atomic_flag;

} // namespace mars_boost

#endif // BOOST_ATOMIC_ATOMIC_FLAG_HPP_INCLUDED_
