// Copyright 2018 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

mars_boost::shared_ptr<int> dll_test_41();
mars_boost::shared_ptr<int> dll_test_42();
mars_boost::shared_ptr<int> dll_test_43();
mars_boost::shared_ptr<int[]> dll_test_44();
mars_boost::shared_ptr<int[]> dll_test_45();

int main()
{
    {
        mars_boost::shared_ptr<int> p = dll_test_41();
        BOOST_TEST_EQ( *p, 41 );
    }

    {
        mars_boost::shared_ptr<int> p = dll_test_42();
        BOOST_TEST_EQ( *p, 42 );
    }

    {
        mars_boost::shared_ptr<int> p = dll_test_43();
        BOOST_TEST_EQ( *p, 43 );
    }

    {
        mars_boost::shared_ptr<int[]> p = dll_test_44();
        BOOST_TEST_EQ( p[0], 44 );
    }

    {
        mars_boost::shared_ptr<int[]> p = dll_test_45();
        BOOST_TEST_EQ( p[0], 45 );
    }

    return mars_boost::report_errors();
}
