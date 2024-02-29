
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

struct my_exception : public std::runtime_error {
    ctx::fiber    f;
    my_exception( ctx::fiber && f_, std::string const& what) :
        std::runtime_error{ what },
        f{ std::move( f_) } {
    }
};

int main() {
    ctx::fiber f{[](ctx::fiber && f) ->ctx::fiber {
        std::cout << "entered" << std::endl;
        try {
            f = std::move( f).resume();
        } catch ( my_exception & ex) {
            std::cerr << "my_exception: " << ex.what() << std::endl;
            return std::move( ex.f);
        }
        return {};
    }};
    f = std::move( f).resume();
    f = std::move( f).resume_with([](ctx::fiber && f) ->ctx::fiber {
        throw my_exception(std::move( f), "abc");
        return {};
    });

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
