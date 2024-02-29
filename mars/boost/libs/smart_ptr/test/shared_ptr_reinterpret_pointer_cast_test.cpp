//
//  shared_pointer_reinterpret_pointer_cast_test.cpp
//
//  Copyright (c) 2016 Chris Glover
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

struct X
{};

int main()
{
    {
        mars_boost::shared_ptr<char> pc;

        mars_boost::shared_ptr<int> pi = mars_boost::reinterpret_pointer_cast<int>(pc);
        BOOST_TEST(pi.get() == 0);

        mars_boost::shared_ptr<X> px = mars_boost::reinterpret_pointer_cast<X>(pc);
        BOOST_TEST(px.get() == 0);
    }

    {
        mars_boost::shared_ptr<int> pi(new int);
        mars_boost::shared_ptr<char> pc = mars_boost::reinterpret_pointer_cast<char>(pi);

        mars_boost::shared_ptr<int> pi2 = mars_boost::reinterpret_pointer_cast<int>(pc);
        BOOST_TEST(pi.get() == pi2.get());
        BOOST_TEST(!(pi < pi2 || pi2 < pi));
        BOOST_TEST(pi.use_count() == 3);
        BOOST_TEST(pc.use_count() == 3);
        BOOST_TEST(pi2.use_count() == 3);
    }

    {
        mars_boost::shared_ptr<X> px(new X);
        mars_boost::shared_ptr<char> pc = mars_boost::reinterpret_pointer_cast<char>(px);

        mars_boost::shared_ptr<X> px2 = mars_boost::reinterpret_pointer_cast<X>(pc);
        BOOST_TEST(px.get() == px2.get());
        BOOST_TEST(!(px < px2 || px2 < px));
        BOOST_TEST(px.use_count() == 3);
        BOOST_TEST(pc.use_count() == 3);
        BOOST_TEST(px2.use_count() == 3);
    }

    return mars_boost::report_errors();
}

