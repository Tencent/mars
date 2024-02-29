
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

int main() {
    int data = 1;
    ctx::fiber f{
            [&data](ctx::fiber && f){
                std::cout << "entered first time: " << data << std::endl;
                data += 2;
                f = std::move( f).resume();
                std::cout << "entered second time: " << data << std::endl;
                return std::move( f);
            }};
    f = std::move( f).resume();
    std::cout << "returned first time: " << data << std::endl;
    data += 2;
    f = std::move( f).resume();
    if ( f) {
        std::cout << "returned second time: " << data << std::endl;
    } else {
        std::cout << "returned second time: execution context terminated" << std::endl;
    }
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
