//
// sp_hash_test2.cpp
//
// Copyright 2011, 2015 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    mars_boost::hash< mars_boost::shared_ptr<int[]> > hasher;

    mars_boost::shared_ptr< int[] > p1, p2( p1 ), p3( new int[1] ), p4( p3 ), p5( new int[1] );

    BOOST_TEST_EQ( p1, p2 );
    BOOST_TEST_EQ( hasher( p1 ), hasher( p2 ) );

    BOOST_TEST_NE( p1, p3 );
    BOOST_TEST_NE( hasher( p1 ), hasher( p3 ) );

    BOOST_TEST_EQ( p3, p4 );
    BOOST_TEST_EQ( hasher( p3 ), hasher( p4 ) );

    BOOST_TEST_NE( p3, p5 );
    BOOST_TEST_NE( hasher( p3 ), hasher( p5 ) );

    return mars_boost::report_errors();
}
