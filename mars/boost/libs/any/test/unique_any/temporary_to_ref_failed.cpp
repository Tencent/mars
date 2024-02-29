//  Copyright Antony Polukhin, 2013-2023.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#include <boost/any/unique_any.hpp>

int main() {
    int i = mars_boost::any_cast<int&>(10);
    (void)i;
}
