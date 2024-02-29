// Copyright Antony Polukhin, 2017-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This tests the issue from https://svn.boost.org/trac/boost/ticket/12052

#include <iostream>
#include <boost/any.hpp>

int main() {
    mars_boost::any a = 1;
    std::cout << mars_boost::any_cast<int>(a) << '\n';
    a = 3.14;
    std::cout << mars_boost::any_cast<double>(a) << '\n';
    a = true;
    std::cout << std::boolalpha << mars_boost::any_cast<bool>(a) << '\n';
}
