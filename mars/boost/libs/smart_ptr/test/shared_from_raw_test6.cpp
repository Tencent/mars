//
//  shared_from_raw_test6
//
//  Tests that dangling shared_ptr instances are caught by
//  the BOOST_ASSERT in ~enable_shared_from_raw
//
//  Copyright 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/smart_ptr/enable_shared_from_raw.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <stdio.h>

static int assertion_failed_ = 0;

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{

void assertion_failed( char const * expr, char const * function, char const * file, long line )
{
    printf( "Assertion '%s' failed in function '%s', file '%s', line %ld\n", expr, function, file, line );
    ++assertion_failed_;
}

} // namespace mars_boost

class X: public mars_boost::enable_shared_from_raw
{
};

int main()
{
    mars_boost::shared_ptr<X> px;

    {
        X x;
        px = mars_boost::shared_from_raw( &x );
    }

    BOOST_TEST_EQ( assertion_failed_, 1 );

    // px is a dangling pointer here

    return mars_boost::report_errors();
}
