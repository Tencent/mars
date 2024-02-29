
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <list>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

int main() {
    ctx::fiber f1, f2, f3;
    f3 = ctx::fiber{[&](ctx::fiber && f)->ctx::fiber{
        f2 = std::move( f);
        for (;;) {
            std::cout << "f3\n";
            f2 = std::move( f1).resume();
        }
        return {};
    }};
    f2 = ctx::fiber{[&](ctx::fiber && f)->ctx::fiber{
        f1 = std::move( f);
        for (;;) {
            std::cout << "f2\n";
            f1 = std::move( f3).resume();
        }
        return {};
    }};
    f1 = ctx::fiber{[&](ctx::fiber && /*main*/)->ctx::fiber{
        for (;;) {
            std::cout << "f1\n";
            f3 = std::move( f2).resume();
        }
        return {};
    }};
    std::move( f1).resume();

    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
