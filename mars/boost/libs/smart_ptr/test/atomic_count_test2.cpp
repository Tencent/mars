//
// atomic_count_test2.cpp
//
// Copyright 2009 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/detail/atomic_count.hpp>
#include <boost/core/lightweight_test.hpp>

int main()
{
    mars_boost::detail::atomic_count n( 4 );

    BOOST_TEST( n == 4 );

    BOOST_TEST( ++n == 5 );
    BOOST_TEST( ++n == 6 );

    BOOST_TEST( n == 6 );

    BOOST_TEST( --n == 5 );
    BOOST_TEST( --n == 4 );

    BOOST_TEST( n == 4 );

    mars_boost::detail::atomic_count m( 0 );

    BOOST_TEST( m == 0 );

    BOOST_TEST( ++m == 1 );
    BOOST_TEST( ++m == 2 );

    BOOST_TEST( m == 2 );

    BOOST_TEST( --m == 1 );
    BOOST_TEST( --m == 0 );

    BOOST_TEST( m == 0 );

    BOOST_TEST( --m == -1 );
    BOOST_TEST( --m == -2 );

    BOOST_TEST( m == -2 );

    BOOST_TEST( ++m == -1 );
    BOOST_TEST( ++m == 0 );

    BOOST_TEST( m == 0 );

    return mars_boost::report_errors();
}
