
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <boost/context/detail/fcontext.hpp>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#include "../clock.hpp"
#include "../cycle.hpp"

template< std::size_t Max, std::size_t Default, std::size_t Min >
class simple_stack_allocator
{
public:
    static std::size_t maximum_stacksize()
    { return Max; }

    static std::size_t default_stacksize()
    { return Default; }

    static std::size_t minimum_stacksize()
    { return Min; }

    void * allocate( std::size_t size) const
    {
        BOOST_ASSERT( minimum_stacksize() <= size);
        BOOST_ASSERT( maximum_stacksize() >= size);

        void * limit = std::malloc( size);
        if ( ! limit) throw std::bad_alloc();

        return static_cast< char * >( limit) + size;
    }

    void deallocate( void * vp, std::size_t size) const
    {
        BOOST_ASSERT( vp);
        BOOST_ASSERT( minimum_stacksize() <= size);
        BOOST_ASSERT( maximum_stacksize() >= size);

        void * limit = static_cast< char * >( vp) - size;
        std::free( limit);
    }
};

typedef simple_stack_allocator<
            8 * 1024 * 1024, 64 * 1024, 8 * 1024
        >                                       stack_allocator;

mars_boost::uint64_t jobs = 1000000;

static void foo( mars_boost::context::detail::transfer_t t_) {
    mars_boost::context::detail::transfer_t t = t_;
    while ( true) {
        t = mars_boost::context::detail::jump_fcontext( t.fctx, 0);
    }
}

duration_type measure_time_fc() {
    stack_allocator stack_alloc;
    mars_boost::context::detail::fcontext_t ctx = mars_boost::context::detail::make_fcontext(
            stack_alloc.allocate( stack_allocator::default_stacksize() ),
            stack_allocator::default_stacksize(),
            foo);

    // cache warum-up
    mars_boost::context::detail::transfer_t t = mars_boost::context::detail::jump_fcontext( ctx, 0);

    time_point_type start( clock_type::now() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        t = mars_boost::context::detail::jump_fcontext( t.fctx, 0);
    }
    duration_type total = clock_type::now() - start;
    total -= overhead_clock(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

#ifdef BOOST_CONTEXT_CYCLE
cycle_type measure_cycles_fc() {
    stack_allocator stack_alloc;
    mars_boost::context::detail::fcontext_t ctx = mars_boost::context::detail::make_fcontext(
            stack_alloc.allocate( stack_allocator::default_stacksize() ),
            stack_allocator::default_stacksize(),
            foo);

    // cache warum-up
    mars_boost::context::detail::transfer_t t = mars_boost::context::detail::jump_fcontext( ctx, 0);

    cycle_type start( cycles() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        t = mars_boost::context::detail::jump_fcontext( t.fctx, 0);
    }
    cycle_type total = cycles() - start;
    total -= overhead_cycle(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}
#endif

int main( int argc, char * argv[])
{
    try
    {
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

        mars_boost::uint64_t res = measure_time_fc().count();
        std::cout << "fcontext_t: average of " << res << " nano seconds" << std::endl;
#ifdef BOOST_CONTEXT_CYCLE
        res = measure_cycles_fc();
        std::cout << "fcontext_t: average of " << res << " cpu cycles" << std::endl;
#endif

        return EXIT_SUCCESS;
    }
    catch ( std::exception const& e)
    { std::cerr << "exception: " << e.what() << std::endl; }
    catch (...)
    { std::cerr << "unhandled exception" << std::endl; }
    return EXIT_FAILURE;
}
