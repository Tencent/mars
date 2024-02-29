
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/context/continuation.hpp>

namespace ctx = mars_boost::context;

int main() {
    std::cout << "minimum stack size: " << ctx::stack_traits::minimum_size() << " byte\n";
    std::cout << "default stack size: " << ctx::stack_traits::default_size() << " byte\n";
    std::cout << "maximum stack size: ";
    if ( ctx::stack_traits::is_unbounded() ) {
        std::cout << "unlimited\n";
    } else {
        std::cout << ctx::stack_traits::maximum_size() << " byte\n";
    }
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
