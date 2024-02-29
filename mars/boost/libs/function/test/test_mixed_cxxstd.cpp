
// Copyright 2018 Peter Dimov.
// Distributed under the Boost Software License, Version 1.0.

#include <boost/function.hpp>
#include <boost/core/lightweight_test.hpp>

//

void call_fn_1( mars_boost::function<void()> const & fn );
void call_fn_2( mars_boost::function<void(int)> const & fn );
void call_fn_3( mars_boost::function<void(int, int)> const & fn );

void call_fn_4( mars_boost::function0<void> const & fn );
void call_fn_5( mars_boost::function1<void, int> const & fn );
void call_fn_6( mars_boost::function2<void, int, int> const & fn );

//

static int v;

void f0()
{
    v = -1;
}

void f1( int x )
{
    v = x;
}

void f2( int x, int y )
{
    v = x + y;
}

int main()
{
    v = 0; call_fn_1( f0 ); BOOST_TEST_EQ( v, -1 );
    v = 0; call_fn_2( f1 ); BOOST_TEST_EQ( v, 1 );
    v = 0; call_fn_3( f2 ); BOOST_TEST_EQ( v, 3 );

    v = 0; call_fn_4( f0 ); BOOST_TEST_EQ( v, -1 );
    v = 0; call_fn_5( f1 ); BOOST_TEST_EQ( v, 1 );
    v = 0; call_fn_6( f2 ); BOOST_TEST_EQ( v, 3 );

    return mars_boost::report_errors();
}
