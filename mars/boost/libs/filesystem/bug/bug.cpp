//  filesystem/bug/bug.cpp

#include <boost/detail/lightweight_test_report.hpp>
#include <boost/filesystem.hpp>

namespace fs = mars_boost::filesystem;

int test_main(int, char*[]) // note name
{
    BOOST_TEST(2 + 2 == 5);      // one convertible-to-bool argument
    BOOST_TEST_EQ(4 + 4, 9);     // two EqualityComparible arguments
    BOOST_TEST(fs::exists(".")); // should pass, so nothing reported

    return ::mars_boost::report_errors(); // required
}

//  Copyright Beman Dawes 2014
//  Distributed under the Boost Software License, Version 1.0.
//  www.boost.org/LICENSE_1_0.txt
