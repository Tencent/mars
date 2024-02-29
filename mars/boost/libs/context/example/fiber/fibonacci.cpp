
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <memory>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

int main() {
    int a;
    ctx::fiber f{
        [&a](ctx::fiber && f){
            a=0;
            int b=1;
            for(;;){
                f = std::move( f).resume();
                int next=a+b;
                a=b;
                b=next;
            }
            return std::move( f);
        }};
    for ( int j = 0; j < 10; ++j) {
        f = std::move( f).resume();
        std::cout << a << " ";
    }
    std::cout << std::endl;
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
