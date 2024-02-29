
// Copyright 2018 Peter Dimov.
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

// See library home page at http://www.boost.org/libs/filesystem

#include <boost/filesystem.hpp>
#include <boost/core/lightweight_test.hpp>

namespace fs = mars_boost::filesystem;

int main()
{
    BOOST_TEST_THROWS(fs::copy("/tmp/non-existent-a", "/tmp/non-existent-b"), std::exception);
    return mars_boost::report_errors();
}
