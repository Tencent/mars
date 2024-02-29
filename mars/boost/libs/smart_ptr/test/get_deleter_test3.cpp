//
// get_deleter_test3.cpp
//
// Copyright 2002, 2017 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/core/lightweight_test.hpp>

struct deleter
{
};

struct deleter2;

struct X
{
};

int main()
{
    {
        mars_boost::shared_ptr<X> p = mars_boost::make_shared<X>();

        BOOST_TEST(mars_boost::get_deleter<void>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<void const>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<int>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<int const>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<X>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<X const>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<deleter>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<deleter const>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<deleter2>(p) == 0);
        BOOST_TEST(mars_boost::get_deleter<deleter2 const>(p) == 0);
    }

    return mars_boost::report_errors();
}

struct deleter2
{
};
