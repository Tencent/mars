//  Copyright Antony Polukhin, 2013-2023.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#include <boost/any/basic_any.hpp>
#include <boost/any/unique_any.hpp>

int main() {
    mars_boost::anys::basic_any<> a;
    mars_boost::anys::unique_any b(a);
    (void)b;
}
