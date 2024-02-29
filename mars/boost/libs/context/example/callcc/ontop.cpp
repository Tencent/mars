
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <tuple>

#include <boost/context/continuation.hpp>

namespace ctx = mars_boost::context;

int main() {
    int data = 0;
    ctx::continuation c = ctx::callcc( [&data](ctx::continuation && c) {
                        std::cout << "f1: entered first time: " << data  << std::endl;
                        data += 1;
                        c = c.resume();
                        std::cout << "f1: entered second time: " << data  << std::endl;
                        data += 1;
                        c = c.resume();
                        std::cout << "f1: entered third time: " << data << std::endl;
                        return std::move( c);
                    });
    std::cout << "f1: returned first time: " << data << std::endl;
    data += 1;
    c = c.resume();
    std::cout << "f1: returned second time: " << data << std::endl;
    data += 1;
    c = c.resume_with( [&data](ctx::continuation && c){
                            std::cout << "f2: entered: " << data << std::endl;
                            data = -1;
                            return std::move( c);
                        });
    std::cout << "f1: returned third time" << std::endl;
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
