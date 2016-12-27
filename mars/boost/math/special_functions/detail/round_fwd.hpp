// Copyright John Maddock 2008.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_SPECIAL_ROUND_FWD_HPP
#define BOOST_MATH_SPECIAL_ROUND_FWD_HPP

#include <boost/config.hpp>
#include <boost/math/tools/promotion.hpp>

#ifdef _MSC_VER
#pragma once
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
   namespace math
   { 

   template <class T, class Policy>
   typename tools::promote_args<T>::type trunc(const T& v, const Policy& pol);
   template <class T>
   typename tools::promote_args<T>::type trunc(const T& v);
   template <class T, class Policy>
   int itrunc(const T& v, const Policy& pol);
   template <class T>
   int itrunc(const T& v);
   template <class T, class Policy>
   long ltrunc(const T& v, const Policy& pol);
   template <class T>
   long ltrunc(const T& v);
#ifdef BOOST_HAS_LONG_LONG
   template <class T, class Policy>
   mars_boost::long_long_type lltrunc(const T& v, const Policy& pol);
   template <class T>
   mars_boost::long_long_type lltrunc(const T& v);
#endif
   template <class T, class Policy>
   typename tools::promote_args<T>::type round(const T& v, const Policy& pol);
   template <class T>
   typename tools::promote_args<T>::type round(const T& v);
   template <class T, class Policy>
   int iround(const T& v, const Policy& pol);
   template <class T>
   int iround(const T& v);
   template <class T, class Policy>
   long lround(const T& v, const Policy& pol);
   template <class T>
   long lround(const T& v);
#ifdef BOOST_HAS_LONG_LONG
   template <class T, class Policy>
   mars_boost::long_long_type llround(const T& v, const Policy& pol);
   template <class T>
   mars_boost::long_long_type llround(const T& v);
#endif
   template <class T, class Policy>
   T modf(const T& v, T* ipart, const Policy& pol);
   template <class T>
   T modf(const T& v, T* ipart);
   template <class T, class Policy>
   T modf(const T& v, int* ipart, const Policy& pol);
   template <class T>
   T modf(const T& v, int* ipart);
   template <class T, class Policy>
   T modf(const T& v, long* ipart, const Policy& pol);
   template <class T>
   T modf(const T& v, long* ipart);
#ifdef BOOST_HAS_LONG_LONG
   template <class T, class Policy>
   T modf(const T& v, mars_boost::long_long_type* ipart, const Policy& pol);
   template <class T>
   T modf(const T& v, mars_boost::long_long_type* ipart);
#endif

   }
}

#undef BOOST_MATH_STD_USING
#define BOOST_MATH_STD_USING BOOST_MATH_STD_USING_CORE\
   using mars_boost::math::round;\
   using mars_boost::math::iround;\
   using mars_boost::math::lround;\
   using mars_boost::math::trunc;\
   using mars_boost::math::itrunc;\
   using mars_boost::math::ltrunc;\
   using mars_boost::math::modf;


#endif // BOOST_MATH_SPECIAL_ROUND_FWD_HPP

