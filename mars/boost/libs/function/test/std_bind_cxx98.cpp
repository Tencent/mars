// Function library

// Copyright (C) 2001-2003 Douglas Gregor

// Use, modification and distribution is subject to the Boost Software 
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt) 

// For more information, see http://www.boost.org/

#if defined(__clang__) && defined(__has_warning)
# if __has_warning( "-Wdeprecated-declarations" )
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
# endif
#endif

#if defined(__GNUC__) && __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <boost/function.hpp>
#include <iostream>
#include <functional>

struct X {
  int foo(int);
};
int X::foo(int x) { return -x; }

int main()
{
#ifndef BOOST_NO_CXX98_BINDERS
      mars_boost::function<int (int)> f;
  X x;
  f = std::bind1st(
        std::mem_fun(&X::foo), &x);
  f(5); // Call x.foo(5)
#endif
    return 0;
}
