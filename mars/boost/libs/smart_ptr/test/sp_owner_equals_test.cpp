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

        BOOST_TEST( p1.owner_equals( p2 ) );
        BOOST_TEST( p2.owner_equals( p1 ) );

        mars_boost::shared_ptr<int> p3( new int );

        BOOST_TEST( !p1.owner_equals( p3 ) );
        BOOST_TEST( !p3.owner_equals( p1 ) );

        mars_boost::shared_ptr<int> p4;
        mars_boost::shared_ptr<int> p5;

        BOOST_TEST( p4.owner_equals( p5 ) );
        BOOST_TEST( p5.owner_equals( p4 ) );

        BOOST_TEST( !p4.owner_equals( p3 ) );
        BOOST_TEST( !p3.owner_equals( p4 ) );

        mars_boost::shared_ptr<int> p6( static_cast<int*>(0) );

        BOOST_TEST( !p4.owner_equals( p6 ) );
        BOOST_TEST( !p6.owner_equals( p4 ) );

        mars_boost::shared_ptr<void> p7( p1 );

        BOOST_TEST( p1.owner_equals( p7 ) );
        BOOST_TEST( p7.owner_equals( p1 ) );

        mars_boost::shared_ptr<void> p8;

        BOOST_TEST( !p1.owner_equals( p8 ) );
        BOOST_TEST( !p8.owner_equals( p1 ) );

        BOOST_TEST( p4.owner_equals( p8 ) );
        BOOST_TEST( p8.owner_equals( p4 ) );

        mars_boost::weak_ptr<int> q1( p1 );

        BOOST_TEST( p1.owner_equals( q1 ) );
        BOOST_TEST( q1.owner_equals( p1 ) );

        mars_boost::weak_ptr<int> q2( p1 );

        BOOST_TEST( q1.owner_equals( q2 ) );
        BOOST_TEST( q2.owner_equals( q1 ) );

        mars_boost::weak_ptr<int> q3( p3 );

        BOOST_TEST( !p1.owner_equals( q3 ) );
        BOOST_TEST( !q3.owner_equals( p1 ) );

        BOOST_TEST( !q1.owner_equals( q3 ) );
        BOOST_TEST( !q3.owner_equals( q1 ) );

        mars_boost::weak_ptr<int> q4;

        BOOST_TEST( p4.owner_equals( q4 ) );
        BOOST_TEST( q4.owner_equals( p4 ) );

        BOOST_TEST( !q1.owner_equals( q4 ) );
        BOOST_TEST( !q4.owner_equals( q1 ) );

        mars_boost::weak_ptr<void> q5;

        BOOST_TEST( q4.owner_equals( q5 ) );
        BOOST_TEST( q5.owner_equals( q4 ) );

        mars_boost::weak_ptr<void> q7( p7 );

        BOOST_TEST( p1.owner_equals( q7 ) );
        BOOST_TEST( q7.owner_equals( p1 ) );

        BOOST_TEST( q1.owner_equals( q7 ) );
        BOOST_TEST( q7.owner_equals( q1 ) );

        p1.reset();
        p2.reset();
        p3.reset();
        p7.reset();

        BOOST_TEST( q1.expired() );
        BOOST_TEST( q2.expired() );
        BOOST_TEST( q3.expired() );
        BOOST_TEST( q7.expired() );

        BOOST_TEST( q1.owner_equals( q2 ) );
        BOOST_TEST( q2.owner_equals( q1 ) );

        BOOST_TEST( q1.owner_equals( q7 ) );
        BOOST_TEST( q7.owner_equals( q1 ) );

        BOOST_TEST( !q1.owner_equals( q3 ) );
        BOOST_TEST( !q3.owner_equals( q1 ) );

        BOOST_TEST( !q1.owner_equals( q4 ) );
        BOOST_TEST( !q4.owner_equals( q1 ) );
    }

    return mars_boost::report_errors();
}
