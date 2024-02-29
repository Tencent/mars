// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/function.hpp>
#include <boost/core/lightweight_test.hpp>

static int f()
{
    return 1;
}

static int g()
{
    return 2;
}

int main()
{
    {
        mars_boost::function<int()> fn;
        BOOST_TEST( !fn.contains( f ) );
        BOOST_TEST( !fn.contains( g ) );
    }

    {
        mars_boost::function<int()> fn( f );
        BOOST_TEST( fn.contains( f ) );
        BOOST_TEST( !fn.contains( g ) );
    }

    return mars_boost::report_errors();
}
