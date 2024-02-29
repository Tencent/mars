// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace mars_boost::placeholders;

int f1() { return 1; }
int f2() { return 2; }

int main()
{
    {
        mars_boost::function<int()> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int, int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int, int, int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int, int, int, int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    {
        mars_boost::function<int(int, int, int, int, int, int, int, int, int)> fn( mars_boost::bind( f1 ) );

        BOOST_TEST( fn == mars_boost::bind( f1 ) );
        BOOST_TEST( fn != mars_boost::bind( f2 ) );
    }

    return mars_boost::report_errors();
}
