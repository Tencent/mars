
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

class moveable {
public:
    int     value;

    moveable() :
        value( -1) {
        }

    moveable( int v) :
        value( v) {
        }

    moveable( moveable && other) {
        std::swap( value, other.value);
        }

    moveable & operator=( moveable && other) {
        if ( this == & other) return * this;
        value = other.value;
        other.value = -1;
        return * this;
    }

    moveable( moveable const& other) = delete;
    moveable & operator=( moveable const& other) = delete;
};

int main() {
    moveable data{ 1 };
    ctx::fiber f{ std::allocator_arg, ctx::fixedsize_stack{},
                     [&data](ctx::fiber && f){
                        std::cout << "entered first time: " << data.value << std::endl;
                        data = std::move( moveable{ 3 });
                        f = std::move( f).resume();
                        std::cout << "entered second time: " << data.value << std::endl;
                        data = std::move( moveable{});
                        return std::move( f);
                     }};
    f = std::move( f).resume();
    std::cout << "returned first time: " << data.value << std::endl;
    data.value = 5;
    f = std::move( f).resume();
    std::cout << "returned second time: " << data.value << std::endl;
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
