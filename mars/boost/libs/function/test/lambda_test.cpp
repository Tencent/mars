// Boost.Function library

//  Copyright Douglas Gregor 2002-2003. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <cstdlib>


static unsigned
func_impl(int arg1, bool arg2, double arg3)
{
  using namespace std;
  return abs (static_cast<int>((arg2 ? arg1 : 2 * arg1) * arg3));
}

int main()
{
  using mars_boost::function;
  using namespace mars_boost::lambda;

  function <unsigned(bool, double)> f1 = bind(func_impl, 15, _1, _2);
  BOOST_TEST_EQ( f1(true, 2.0), 30u );

  function <unsigned(double)>       f2 = mars_boost::lambda::bind(f1, false, _1);
  BOOST_TEST_EQ( f2(2.0), 60u );

  function <unsigned()>             f3 = mars_boost::lambda::bind(f2, 4.0);
  BOOST_TEST_EQ( f3(), 120u );

  return mars_boost::report_errors();
}
