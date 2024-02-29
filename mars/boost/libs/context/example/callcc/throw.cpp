
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/context/continuation.hpp>

namespace ctx = mars_boost::context;

struct my_exception : public std::runtime_error {
    ctx::continuation    c;
    my_exception( ctx::continuation && c_, std::string const& what) :
        std::runtime_error{ what },
        c{ std::move( c_) } {
    }
};

int main() {
    ctx::continuation c = ctx::callcc([](ctx::continuation && c) {
        for (;;) {
            try {
                std::cout << "entered" << std::endl;
                c = c.resume();
            } catch ( my_exception & ex) {
                std::cerr << "my_exception: " << ex.what() << std::endl;
                return std::move( ex.c);
            }
        }
        return std::move( c);
    });
    c = c.resume_with(
           [](ctx::continuation && c){
               throw my_exception(std::move( c), "abc");
               return {};
           });

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
