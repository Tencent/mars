#ifndef BOOST_THREAD_MUTEX_HPP
#define BOOST_THREAD_MUTEX_HPP

//  mutex.hpp
//
//  (C) Copyright 2007 Anthony Williams
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <third_party/boost/thread/detail/platform.hpp>
#if defined(BOOST_THREAD_PLATFORM_WIN32)
#include <third_party/boost/thread/win32/mutex.hpp>
#elif defined(BOOST_THREAD_PLATFORM_PTHREAD)
#include <third_party/boost/thread/pthread/mutex.hpp>
#else
#error "Boost threads unavailable on this platform"
#endif

#include <third_party/boost/thread/lockable_traits.hpp>


namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
  namespace sync
  {
#ifdef BOOST_THREAD_NO_AUTO_DETECT_MUTEX_TYPES
    template<>
    struct is_basic_lockable<mutex>
    {
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_lockable<mutex>
    {
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_basic_lockable<timed_mutex>
    {
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
    template<>
    struct is_lockable<timed_mutex>
    {
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
#endif
  }
}

#endif
