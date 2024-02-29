// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    {
        mars_boost::shared_ptr<int> p1( new int );
        mars_boost::shared_ptr<int> p2( p1 );

        BOOST_TEST_EQ( p1.owner_hash_value(), p2.owner_hash_value() );

        mars_boost::shared_ptr<int> p3( new int );

        BOOST_TEST_NE( p1.owner_hash_value(), p3.owner_hash_value() );

        mars_boost::shared_ptr<int> p4;
        mars_boost::shared_ptr<int> p5;

        BOOST_TEST_EQ( p4.owner_hash_value(), p5.owner_hash_value() );
        BOOST_TEST_NE( p4.owner_hash_value(), p3.owner_hash_value() );

        mars_boost::shared_ptr<int> p6( static_cast<int*>(0) );

        BOOST_TEST_NE( p4.owner_hash_value(), p6.owner_hash_value() );

        mars_boost::shared_ptr<void> p7( p1 );

        BOOST_TEST_EQ( p1.owner_hash_value(), p7.owner_hash_value() );

        mars_boost::shared_ptr<void> p8;

        BOOST_TEST_NE( p1.owner_hash_value(), p8.owner_hash_value() );
        BOOST_TEST_EQ( p4.owner_hash_value(), p8.owner_hash_value() );

        mars_boost::weak_ptr<int> q1( p1 );

        BOOST_TEST_EQ( p1.owner_hash_value(), q1.owner_hash_value() );

        mars_boost::weak_ptr<int> q2( p1 );

        BOOST_TEST_EQ( q1.owner_hash_value(), q2.owner_hash_value() );

        mars_boost::weak_ptr<int> q3( p3 );

        BOOST_TEST_NE( p1.owner_hash_value(), q3.owner_hash_value() );
        BOOST_TEST_NE( q1.owner_hash_value(), q3.owner_hash_value() );

        mars_boost::weak_ptr<int> q4;

        BOOST_TEST_EQ( p4.owner_hash_value(), q4.owner_hash_value() );
        BOOST_TEST_NE( q1.owner_hash_value(), q4.owner_hash_value() );

        mars_boost::weak_ptr<void> q5;

        BOOST_TEST_EQ( q4.owner_hash_value(), q5.owner_hash_value() );

        mars_boost::weak_ptr<void> q7( p7 );

        BOOST_TEST_EQ( p1.owner_hash_value(), q7.owner_hash_value() );
        BOOST_TEST_EQ( q1.owner_hash_value(), q7.owner_hash_value() );

        p1.reset();
        p2.reset();
        p3.reset();
        p7.reset();

        BOOST_TEST( q1.expired() );
        BOOST_TEST( q2.expired() );
        BOOST_TEST( q3.expired() );
        BOOST_TEST( q7.expired() );

        BOOST_TEST_EQ( q1.owner_hash_value(), q2.owner_hash_value() );
        BOOST_TEST_EQ( q1.owner_hash_value(), q7.owner_hash_value() );

        BOOST_TEST_NE( q1.owner_hash_value(), q3.owner_hash_value() );
        BOOST_TEST_NE( q1.owner_hash_value(), q4.owner_hash_value() );
    }

    return mars_boost::report_errors();
}
