
//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <memory>

#include <boost/context/continuation.hpp>

namespace ctx = mars_boost::context;

#ifdef BOOST_MSVC //MS VisualStudio
__declspec(noinline) void access( char *buf);
#else // GCC
void access( char *buf) __attribute__ ((noinline));
#endif
void access( char *buf) {
  buf[0] = '\0';
}

void bar( int i) {
    char buf[4 * 1024];
    if ( i > 0) {
        access( buf);
        std::cout << i << ". iteration" << std::endl;
        bar( i - 1);
    }
}

int main() {
    int count = 100*1024;
#if defined(BOOST_USE_SEGMENTED_STACKS)
    std::cout << "using segmented_stack stacks: allocates " << count << " * 4kB == " << 4 * count << "kB on stack, ";
    std::cout << "initial stack size = " << ctx::segmented_stack::traits_type::default_size() / 1024 << "kB" << std::endl;
    std::cout << "application should not fail" << std::endl;
#else
    std::cout << "using standard stacks: allocates " << count << " * 4kB == " << 4 * count << "kB on stack, ";
    std::cout << "initial stack size = " << ctx::fixedsize_stack::traits_type::default_size() / 1024 << "kB" << std::endl;
    std::cout << "application might fail" << std::endl;
#endif
    ctx::continuation c = ctx::callcc(
        [count](ctx::continuation && c){
            bar( count);
            return std::move( c);
        });
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
