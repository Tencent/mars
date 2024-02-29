
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CYCLE_I386_H
#define CYCLE_I386_H

#include <algorithm>
#include <numeric>
#include <cstddef>
#include <vector>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>

#define BOOST_CONTEXT_CYCLE

typedef mars_boost::uint64_t cycle_type;

#if _MSC_VER
inline
cycle_type cycles()
{
    cycle_type c;
    __asm {
        cpuid 
        rdtsc
        mov dword ptr [c + 0], eax
        mov dword ptr [c + 4], edx
    }
    return c;
}
#elif defined(__GNUC__) || \
      defined(__INTEL_COMPILER) || defined(__ICC) || defined(_ECC) || defined(__ICL)
inline
cycle_type cycles()
{
    mars_boost::uint32_t lo, hi;

    __asm__ __volatile__ (
        "xorl %%eax, %%eax\n"
        "cpuid\n"
        ::: "%eax", "%ebx", "%ecx", "%edx"
    );
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi) );
    __asm__ __volatile__ (
        "xorl %%eax, %%eax\n"
        "cpuid\n"
        ::: "%eax", "%ebx", "%ecx", "%edx"
    );

    return ( cycle_type)hi << 32 | lo; 
}
#else
# error "this compiler is not supported"
#endif

struct cycle_overhead
{
    cycle_type operator()()
    {
        cycle_type start( cycles() );
        return cycles() - start;
    }
};

inline
cycle_type overhead_cycle()
{
    std::size_t iterations( 10);
    std::vector< cycle_type >  overhead( iterations, 0);
    for ( std::size_t i( 0); i < iterations; ++i)
        std::generate(
            overhead.begin(), overhead.end(),
            cycle_overhead() );
    BOOST_ASSERT( overhead.begin() != overhead.end() );
    return std::accumulate( overhead.begin(), overhead.end(), 0) / iterations;
}

#endif // CYCLE_I386_H
