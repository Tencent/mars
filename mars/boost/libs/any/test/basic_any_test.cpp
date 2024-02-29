// Copyright Kevlin Henney, 2000, 2001. All rights reserved.
// Copyright Antony Polukhin, 2013-2023.
// Copyright Ruslan Arutyunyan, 2019-2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "boost/any/basic_any.hpp"
#include "basic_test.hpp"

int main() {
    const int res1 = any_tests::basic_tests<mars_boost::anys::basic_any<> >::run_tests();
    if (res1) return 1;

    const int res2 = any_tests::basic_tests<mars_boost::anys::basic_any<256, 8> >::run_tests();
    if (res2) return 2;

    const int res3 = any_tests::basic_tests<mars_boost::anys::basic_any<1, 1> >::run_tests();
    if (res3) return 3;

    const int res4 = any_tests::basic_tests<mars_boost::anys::basic_any<64, 8> >::run_tests();
    if (res4) return 4;
}
