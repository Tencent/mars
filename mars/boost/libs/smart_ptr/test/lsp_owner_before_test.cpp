// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    {
        mars_boost::local_shared_ptr<int> p1( new int );
        mars_boost::local_shared_ptr<int> p2( p1 );

        BOOST_TEST( !p1.owner_before( p2 ) );
        BOOST_TEST( !p2.owner_before( p1 ) );

        mars_boost::local_shared_ptr<int> p3( new int );

        BOOST_TEST( p1.owner_before( p3 ) || p3.owner_before( p1 ) );

        mars_boost::local_shared_ptr<int> p4;
        mars_boost::local_shared_ptr<int> p5;

        BOOST_TEST( !p4.owner_before( p5 ) );
        BOOST_TEST( !p5.owner_before( p4 ) );

        BOOST_TEST( p4.owner_before( p3 ) || p3.owner_before( p4 ) );

        mars_boost::local_shared_ptr<int> p6( static_cast<int*>(0) );

        BOOST_TEST( p4.owner_before( p6 ) || p6.owner_before( p4 ) );

        mars_boost::local_shared_ptr<void> p7( p1 );

        BOOST_TEST( !p1.owner_before( p7 ) );
        BOOST_TEST( !p7.owner_before( p1 ) );

        mars_boost::local_shared_ptr<void> p8;

        BOOST_TEST( p1.owner_before( p8 ) || p8.owner_before( p1 ) );

        BOOST_TEST( !p4.owner_before( p8 ) );
        BOOST_TEST( !p8.owner_before( p4 ) );
/*
        mars_boost::local_weak_ptr<int> q1( p1 );

        BOOST_TEST( !p1.owner_before( q1 ) );
        BOOST_TEST( !q1.owner_before( p1 ) );

        mars_boost::local_weak_ptr<int> q2( p1 );

        BOOST_TEST( !q1.owner_before( q2 ) );
        BOOST_TEST( !q2.owner_before( q1 ) );

        mars_boost::local_weak_ptr<int> q3( p3 );

        BOOST_TEST( p1.owner_before( q3 ) || q3.owner_before( p1 ) );
        BOOST_TEST( q1.owner_before( q3 ) || q3.owner_before( q1 ) );

        mars_boost::local_weak_ptr<int> q4;

        BOOST_TEST( !p4.owner_before( q4 ) );
        BOOST_TEST( !q4.owner_before( p4 ) );

        BOOST_TEST( q1.owner_before( q4 ) || q4.owner_before( q1 ) );

        mars_boost::local_weak_ptr<void> q5;

        BOOST_TEST( !q4.owner_before( q5 ) );
        BOOST_TEST( !q5.owner_before( q4 ) );

        mars_boost::local_weak_ptr<void> q7( p7 );

        BOOST_TEST( !p1.owner_before( q7 ) );
        BOOST_TEST( !q7.owner_before( p1 ) );

        BOOST_TEST( !q1.owner_before( q7 ) );
        BOOST_TEST( !q7.owner_before( q1 ) );

        p1.reset();
        p2.reset();
        p3.reset();
        p7.reset();

        BOOST_TEST( q1.expired() );
        BOOST_TEST( q2.expired() );
        BOOST_TEST( q3.expired() );
        BOOST_TEST( q7.expired() );

        BOOST_TEST( !q1.owner_before( q2 ) );
        BOOST_TEST( !q2.owner_before( q1 ) );

        BOOST_TEST( !q1.owner_before( q7 ) );
        BOOST_TEST( !q7.owner_before( q1 ) );

        BOOST_TEST( q1.owner_before( q3 ) || q3.owner_before( q1 ) );

        BOOST_TEST( q1.owner_before( q4 ) || q4.owner_before( q1 ) );
*/
    }

    return mars_boost::report_errors();
}
