
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <emmintrin.h>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

void echoSSE( int i) {
    __m128i xmm;
    xmm = _mm_set_epi32( i, i + 1, i + 2, i + 3);
    uint32_t v32[4];
    memcpy( & v32, & xmm, 16);
    std::cout << v32[0]; 
    std::cout << v32[1]; 
    std::cout << v32[2]; 
    std::cout << v32[3]; 
}


int main( int argc, char * argv[]) {
    int i = 0;
    ctx::fiber f{
        [&i](ctx::fiber && f) {
            for (;;) {
                std::cout << i;
                echoSSE( i);
                std::cout << " ";
                f = std::move( f).resume();
            }
            return std::move( f);
        }};
    for (; i < 11; ++i) {
        f = std::move( f).resume();
    }
    std::cout << "\nmain: done" << std::endl;
    return EXIT_SUCCESS;
}
