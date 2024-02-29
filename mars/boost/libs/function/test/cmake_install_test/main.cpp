// Copyright 2017, 2021 Peter Dimov.
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// See library home page at http://www.boost.org/libs/function

#include <boost/function.hpp>
#include <cassert>

#define BOOST_TEST(expr) assert(expr)
#define BOOST_TEST_EQ(x1, x2) assert((x1)==(x2))

int add( int x, int y )
{
    return x + y;
}

int main()
{
    mars_boost::function<int(int, int)> fn( &add );
    BOOST_TEST_EQ( fn( 1, 2 ), 3 );
}
