
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <boost/context/fiber.hpp>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#include "../clock.hpp"
#include "../cycle.hpp"

mars_boost::uint64_t jobs = 1000000;

namespace ctx = mars_boost::context;

static ctx::fiber foo( ctx::fiber && f) {
    while ( true) {
        f = std::move( f).resume();
    }
    return ctx::fiber{};
}

duration_type measure_time() {
    // cache warum-up
    ctx::fiber f{ foo };
    f = std::move( f).resume();

    time_point_type start( clock_type::now() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        f = std::move( f).resume();
    }
    duration_type total = clock_type::now() - start;
    total -= overhead_clock(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

#ifdef BOOST_CONTEXT_CYCLE
cycle_type measure_cycles() {
    // cache warum-up
    ctx::fixedsize_stack alloc;
    ctx::fiber f{ std::allocator_arg, alloc, foo };
    f = std::move( f).resume();

    cycle_type start( cycles() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        f = std::move( f).resume();
    }
    cycle_type total = cycles() - start;
    total -= overhead_cycle(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}
#endif

int main( int argc, char * argv[]) {
    try {
        mars_boost::program_options::options_description desc("allowed options");
        desc.add_options()
            ("help", "help message")
            ("jobs,j", mars_boost::program_options::value< mars_boost::uint64_t >( & jobs), "jobs to run");

        mars_boost::program_options::variables_map vm;
        mars_boost::program_options::store(
                mars_boost::program_options::parse_command_line(
                    argc,
                    argv,
                    desc),
                vm);
        mars_boost::program_options::notify( vm);

        if ( vm.count("help") ) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        mars_boost::uint64_t res = measure_time().count();
        std::cout << "fiber: average of " << res << " nano seconds" << std::endl;
#ifdef BOOST_CONTEXT_CYCLE
        res = measure_cycles();
        std::cout << "fiber: average of " << res << " cpu cycles" << std::endl;
#endif

        return EXIT_SUCCESS;
    } catch ( std::exception const& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unhandled exception" << std::endl;
    }
    return EXIT_FAILURE;
}
