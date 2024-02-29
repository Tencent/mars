
// Copyright 2018 Peter Dimov.
// Distributed under the Boost Software License, Version 1.0.

#include <boost/function.hpp>
#include <boost/core/lightweight_test.hpp>

//

mars_boost::function<int(int, int)> get_fn_1();
mars_boost::function2<int, int, int> get_fn_2();

//

int main()
{
    BOOST_TEST_EQ( get_fn_1()( 1, 2 ), 3 );
    BOOST_TEST_EQ( get_fn_2()( 1, 2 ), 3 );

    return mars_boost::report_errors();
}
