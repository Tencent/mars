// Boost.Signals library

// Copyright (C) Douglas Gregor 2001-2006. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#define BOOST_TEST_MODULE dead_slot_test
#include <boost/test/included/unit_test.hpp>

using namespace mars_boost::placeholders;

typedef mars_boost::signals2::signal<int (int)> sig_type;

class with_constant {
public:
  with_constant(int c) : constant(c) {}

  int add(int i) { return i + constant; }

private:
  int constant;
};

void do_delayed_connect(mars_boost::shared_ptr<with_constant> &wc,
                        sig_type& sig,
                        sig_type::slot_type slot)
{
  // Should invalidate the slot, so that we cannot connect to it
  wc.reset();

  mars_boost::signals2::connection c = sig.connect(slot);
  BOOST_CHECK(!c.connected());
}

BOOST_AUTO_TEST_CASE(test_main)
{
  sig_type s1;
  mars_boost::shared_ptr<with_constant> wc1(new with_constant(7));

  do_delayed_connect(wc1, s1, sig_type::slot_type(&with_constant::add, wc1.get(), _1).track(wc1));
}
