// Copyright 2011, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    {
        mars_boost::hash< mars_boost::weak_ptr<int> > hasher;

        mars_boost::shared_ptr<int> p1, p2( p1 ), p3( new int ), p4( p3 ), p5( new int );
        mars_boost::weak_ptr<int> q1( p1 ), q2( p2 ), q3( p3 ), q4( p4 ), q5( p5 );

        BOOST_TEST_EQ( hasher( q1 ), hasher( q2 ) );
        BOOST_TEST_NE( hasher( q1 ), hasher( q3 ) );
        BOOST_TEST_EQ( hasher( q3 ), hasher( q4 ) );
        BOOST_TEST_NE( hasher( q3 ), hasher( q5 ) );

        p3.reset();
        p4.reset();
        p5.reset();

        BOOST_TEST_EQ( hasher( q1 ), hasher( q2 ) );
        BOOST_TEST_NE( hasher( q1 ), hasher( q3 ) );
        BOOST_TEST_EQ( hasher( q3 ), hasher( q4 ) );
        BOOST_TEST_NE( hasher( q3 ), hasher( q5 ) );
    }

    {
        mars_boost::hash< mars_boost::weak_ptr<int[]> > hasher;

        mars_boost::shared_ptr<int[]> p1, p2( p1 ), p3( new int[1] ), p4( p3 ), p5( new int[1] );
        mars_boost::weak_ptr<int[]> q1( p1 ), q2( p2 ), q3( p3 ), q4( p4 ), q5( p5 );

        BOOST_TEST_EQ( hasher( q1 ), hasher( q2 ) );
        BOOST_TEST_NE( hasher( q1 ), hasher( q3 ) );
        BOOST_TEST_EQ( hasher( q3 ), hasher( q4 ) );
        BOOST_TEST_NE( hasher( q3 ), hasher( q5 ) );

        p3.reset();
        p4.reset();
        p5.reset();

        BOOST_TEST_EQ( hasher( q1 ), hasher( q2 ) );
        BOOST_TEST_NE( hasher( q1 ), hasher( q3 ) );
        BOOST_TEST_EQ( hasher( q3 ), hasher( q4 ) );
        BOOST_TEST_NE( hasher( q3 ), hasher( q5 ) );
    }

    {
        mars_boost::hash< mars_boost::weak_ptr<int[1]> > hasher;

        mars_boost::shared_ptr<int[1]> p1, p2( p1 ), p3( new int[1] ), p4( p3 ), p5( new int[1] );
        mars_boost::weak_ptr<int[1]> q1( p1 ), q2( p2 ), q3( p3 ), q4( p4 ), q5( p5 );

        BOOST_TEST_EQ( hasher( q1 ), hasher( q2 ) );
        BOOST_TEST_NE( hasher( q1 ), hasher( q3 ) );
        BOOST_TEST_EQ( hasher( q3 ), hasher( q4 ) );
        BOOST_TEST_NE( hasher( q3 ), hasher( q5 ) );

        p3.reset();
        p4.reset();
        p5.reset();

        BOOST_TEST_EQ( hasher( q1 ), hasher( q2 ) );
        BOOST_TEST_NE( hasher( q1 ), hasher( q3 ) );
        BOOST_TEST_EQ( hasher( q3 ), hasher( q4 ) );
        BOOST_TEST_NE( hasher( q3 ), hasher( q5 ) );
    }

    return mars_boost::report_errors();
}
