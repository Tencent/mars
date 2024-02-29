
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/context/continuation.hpp>

namespace ctx = mars_boost::context;

ctx::continuation f1( ctx::continuation && c) {
    std::cout << "f1: entered first time" << std::endl;
    c = c.resume();
    std::cout << "f1: entered second time" << std::endl;
    return std::move( c);
}

int main() {
    ctx::continuation c = ctx::callcc( f1);
    std::cout << "f1: returned first time" << std::endl;
    c = c.resume();
    std::cout << "f1: returned second time" << std::endl;
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
