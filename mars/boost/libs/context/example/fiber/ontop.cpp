
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <tuple>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

int main() {
    int data = 0;
    ctx::fiber f{ [&data](ctx::fiber && f) {
                        std::cout << "f1: entered first time: " << data  << std::endl;
                        data += 1;
                        f = std::move( f).resume();
                        std::cout << "f1: entered second time: " << data  << std::endl;
                        data += 1;
                        f = std::move( f).resume();
                        std::cout << "f1: entered third time: " << data << std::endl;
                        return std::move( f);
                    }};
    f = std::move( f).resume();
    std::cout << "f1: returned first time: " << data << std::endl;
    data += 1;
    f = std::move( f).resume();
    std::cout << "f1: returned second time: " << data << std::endl;
    data += 1;
    f = std::move( f).resume_with([&data](ctx::fiber && f){
        std::cout << "f2: entered: " << data << std::endl;
        data = -1;
        return std::move( f);
    });
    std::cout << "f1: returned third time" << std::endl;
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
