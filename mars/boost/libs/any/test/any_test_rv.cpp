//  Unit test for mars_boost::any.
//
//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Antony Polukhin, 2013-2023.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#include <boost/any.hpp>

#include "move_test.hpp"

int main() {
    return any_tests::move_tests<mars_boost::any>::run_tests();
}
