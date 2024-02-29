//
// quick.cpp - a quick (CI) test for Boost.SmartPtr
//
// Copyright 2017 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/smart_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

class X
{
private:

    X( X const & );
    X & operator=( X const & );

public:

    static int instances;

    X()
    {
        ++instances;
    }

    ~X()
    {
        --instances;
    }
};

int X::instances = 0;

int main()
{
    {
        mars_boost::shared_ptr<X> p1( new X );
        BOOST_TEST_EQ( X::instances, 1 );
        BOOST_TEST_EQ( p1.use_count(), 1 );

        mars_boost::shared_ptr<X> p2( p1 );
        BOOST_TEST_EQ( p2.use_count(), 2 );

        mars_boost::weak_ptr<X> wp( p1 );
        BOOST_TEST_EQ( wp.use_count(), 2 );

        p1.reset();
        BOOST_TEST_EQ( wp.use_count(), 1 );

        p2.reset();
        BOOST_TEST_EQ( wp.use_count(), 0 );
        BOOST_TEST_EQ( X::instances, 0 );
    }

    {
        mars_boost::shared_ptr<X> p1 = mars_boost::make_shared<X>();
        BOOST_TEST_EQ( X::instances, 1 );
        BOOST_TEST_EQ( p1.use_count(), 1 );

        mars_boost::shared_ptr<X> p2( p1 );
        BOOST_TEST_EQ( p2.use_count(), 2 );

        mars_boost::weak_ptr<X> wp( p1 );
        BOOST_TEST_EQ( wp.use_count(), 2 );

        p1.reset();
        BOOST_TEST_EQ( wp.use_count(), 1 );

        p2.reset();
        BOOST_TEST_EQ( wp.use_count(), 0 );
        BOOST_TEST_EQ( X::instances, 0 );
    }

    return mars_boost::report_errors();
}
