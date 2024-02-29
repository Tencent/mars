//
//  sp_interlocked_test.cpp
//
//  Copyright 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )

#include <boost/smart_ptr/detail/sp_interlocked.hpp>
#include <boost/core/lightweight_test.hpp>

#ifndef __LP64__

typedef long long_type;

#else

// On Cygwin 64, long is 64 bit
typedef int long_type;

#endif

int main()
{
    long_type x = 0, r;

    r = BOOST_SP_INTERLOCKED_INCREMENT( &x );

    BOOST_TEST( x == 1 );
    BOOST_TEST( r == 1 );

    r = BOOST_SP_INTERLOCKED_DECREMENT( &x );

    BOOST_TEST( x == 0 );
    BOOST_TEST( r == 0 );

    r = BOOST_SP_INTERLOCKED_EXCHANGE( &x, 3 );

    BOOST_TEST( x == 3 );
    BOOST_TEST( r == 0 );

    r = BOOST_SP_INTERLOCKED_EXCHANGE_ADD( &x, 2 );

    BOOST_TEST( x == 5 );
    BOOST_TEST( r == 3 );

    r = BOOST_SP_INTERLOCKED_COMPARE_EXCHANGE( &x, 0, 3 );

    BOOST_TEST( x == 5 );
    BOOST_TEST( r == 5 );

    r = BOOST_SP_INTERLOCKED_COMPARE_EXCHANGE( &x, 0, 5 );

    BOOST_TEST( x == 0 );
    BOOST_TEST( r == 5 );

    return mars_boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
