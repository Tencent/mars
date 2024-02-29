// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/smart_ptr/owner_less.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    mars_boost::owner_less<> const lt = {};

    {
        mars_boost::local_shared_ptr<int> p1( new int );
        mars_boost::local_shared_ptr<int> p2( p1 );

        BOOST_TEST( !lt( p1, p2 ) );
        BOOST_TEST( !lt( p2, p1 ) );

        mars_boost::local_shared_ptr<int> p3( new int );

        BOOST_TEST( lt( p1, p3 ) || lt( p3, p1 ) );

        mars_boost::local_shared_ptr<int> p4;
        mars_boost::local_shared_ptr<int> p5;

        BOOST_TEST( !lt( p4, p5 ) );
        BOOST_TEST( !lt( p5, p4 ) );

        BOOST_TEST( lt( p4, p3 ) || lt( p3, p4 ) );

        mars_boost::local_shared_ptr<int> p6( static_cast<int*>(0) );

        BOOST_TEST( lt( p4, p6 ) || lt( p6, p4 ) );

        mars_boost::local_shared_ptr<void> p7( p1 );

        BOOST_TEST( !lt( p1, p7 ) );
        BOOST_TEST( !lt( p7, p1 ) );

        mars_boost::local_shared_ptr<void> p8;

        BOOST_TEST( lt( p1, p8 ) || lt( p8, p1 ) );

        BOOST_TEST( !lt( p4, p8 ) );
        BOOST_TEST( !lt( p8, p4 ) );
/*
        mars_boost::local_weak_ptr<int> q1( p1 );

        BOOST_TEST( !lt( p1, q1 ) );
        BOOST_TEST( !lt( q1, p1 ) );

        mars_boost::local_weak_ptr<int> q2( p1 );

        BOOST_TEST( !lt( q1, q2 ) );
        BOOST_TEST( !lt( q2, q1 ) );

        mars_boost::local_weak_ptr<int> q3( p3 );

        BOOST_TEST( lt( p1, q3 ) || lt( q3, p1 ) );
        BOOST_TEST( lt( q1, q3 ) || lt( q3, q1 ) );

        mars_boost::local_weak_ptr<int> q4;

        BOOST_TEST( !lt( p4, q4 ) );
        BOOST_TEST( !lt( q4, p4 ) );

        BOOST_TEST( lt( q1, q4 ) || lt( q4, q1 ) );

        mars_boost::local_weak_ptr<void> q5;

        BOOST_TEST( !lt( q4, q5 ) );
        BOOST_TEST( !lt( q5, q4 ) );

        mars_boost::local_weak_ptr<void> q7( p7 );

        BOOST_TEST( !lt( p1, q7 ) );
        BOOST_TEST( !lt( q7, p1 ) );

        BOOST_TEST( !lt( q1, q7 ) );
        BOOST_TEST( !lt( q7, q1 ) );

        p1.reset();
        p2.reset();
        p3.reset();
        p7.reset();

        BOOST_TEST( q1.expired() );
        BOOST_TEST( q2.expired() );
        BOOST_TEST( q3.expired() );
        BOOST_TEST( q7.expired() );

        BOOST_TEST( !lt( q1, q2 ) );
        BOOST_TEST( !lt( q2, q1 ) );

        BOOST_TEST( !lt( q1, q7 ) );
        BOOST_TEST( !lt( q7, q1 ) );

        BOOST_TEST( lt( q1, q3 ) || lt( q3, q1 ) );

        BOOST_TEST( lt( q1, q4 ) || lt( q4, q1 ) );
*/
    }

    return mars_boost::report_errors();
}
