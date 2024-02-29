// Signals2 library
// test for compilation of boost/signals2/slot.hpp

// Copyright Frank Mori Hess 2008
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/signals2/slot.hpp>
#define BOOST_TEST_MODULE slot_compile_test
#include <boost/test/included/unit_test.hpp>

void myslot()
{}

int myslot2(int)
{
  return 0;
}

BOOST_AUTO_TEST_CASE(test_main)
{
  mars_boost::signals2::slot<void (void)> sl0(&myslot);
}
