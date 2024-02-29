
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stdio.h>
#include <stdlib.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/utility.hpp>

#include <boost/context/detail/config.hpp>
#include <boost/context/detail/fcontext.hpp>

#define BOOST_CHECK(x) BOOST_TEST(x)
#define BOOST_CHECK_EQUAL(a, b) BOOST_TEST_EQ(a, b)

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

        void * limit = malloc( size);
        if ( ! limit) throw std::bad_alloc();

        return static_cast< char * >( limit) + size;
    }

    void deallocate( void * vp, std::size_t size) const
    {
        BOOST_ASSERT( vp);
        BOOST_ASSERT( minimum_stacksize() <= size);
        BOOST_ASSERT( maximum_stacksize() >= size);

        void * limit = static_cast< char * >( vp) - size;
        free( limit);
    }
};

typedef simple_stack_allocator<
            8 * 1024 * 1024, 64 * 1024, 8 * 1024
        >                                       stack_allocator;

namespace ctx = mars_boost::context::detail;

typedef simple_stack_allocator<
    8 * 1024 * 1024, // 8MB
    64 * 1024, // 64kB
    8 * 1024 // 8kB
>       stack_allocator;

int value1 = 0;
std::string value2;
double value3 = 0.;
void * value4 = 0;

void f1( ctx::transfer_t t) {
    ++value1;
    ctx::jump_fcontext( t.fctx, t.data);
}

void f3( ctx::transfer_t t_) {
    ++value1;
    ctx::transfer_t t = ctx::jump_fcontext( t_.fctx, 0);
    ++value1;
    ctx::jump_fcontext( t.fctx, t.data);
}

void f4( ctx::transfer_t t) {
    int i = 7;
    ctx::jump_fcontext( t.fctx, & i);
}

void f5( ctx::transfer_t t) {
    ctx::jump_fcontext( t.fctx, t.data);
}

void f6( ctx::transfer_t t_) {
    std::pair< int, int > data = * ( std::pair< int, int > * ) t_.data;
    int res = data.first + data.second;
    ctx::transfer_t t = ctx::jump_fcontext( t_.fctx, & res);
    data = * ( std::pair< int, int > *) t.data;
    res = data.first + data.second;
    ctx::jump_fcontext( t.fctx, & res);
}

void f7( ctx::transfer_t t) {
    try {
        throw std::runtime_error( * ( std::string *) t.data);
    } catch ( std::runtime_error const& e) {
        value2 = e.what();
    }
    ctx::jump_fcontext( t.fctx, t.data);
}

void f8( ctx::transfer_t t) {
    double d = * ( double *) t.data;
    d += 3.45;
    value3 = d;
    ctx::jump_fcontext( t.fctx, 0);
}

void f10( ctx::transfer_t t) {
    value1 = 3;
    ctx::jump_fcontext( t.fctx, 0);
}

void f9( ctx::transfer_t t) {
    std::cout << "f1: entered" << std::endl;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f10);
    ctx::jump_fcontext( ctx, 0);
    ctx::jump_fcontext( t.fctx, 0);
}

ctx::transfer_t f11( ctx::transfer_t t_) {
    value4 = t_.data;
    ctx::transfer_t t = { t_.fctx, t_.data };
    return t;
}

void f12( ctx::transfer_t t_) {
    ctx::transfer_t t = ctx::jump_fcontext( t_.fctx, t_.data);
    value1 = * ( int *) t.data;
    ctx::jump_fcontext( t.fctx, t.data);
}

void f13( ctx::transfer_t t) {
    {
        double n1 = 0;
        double n2 = 0;
        sscanf("3.14 7.13", "%lf %lf", & n1, & n2);
        BOOST_CHECK( n1 == 3.14);
        BOOST_CHECK( n2 == 7.13);
    }
    {
        int n1=0;
        int n2=0;
        sscanf("1 23", "%d %d", & n1, & n2);
        BOOST_CHECK( n1 == 1);
        BOOST_CHECK( n2 == 23);
    }
    {
        int n1=0;
        int n2=0;
        sscanf("1 jjj 23", "%d %*[j] %d", & n1, & n2);
        BOOST_CHECK( n1 == 1);
        BOOST_CHECK( n2 == 23);
    }
    ctx::jump_fcontext( t.fctx, 0);
}

void f14( ctx::transfer_t t) {
    {
        const char *fmt = "sqrt(2) = %f";
        char buf[19];
        snprintf( buf, sizeof( buf), fmt, std::sqrt( 2) );
        BOOST_CHECK( 0 < sizeof( buf) );
        BOOST_CHECK_EQUAL( std::string("sqrt(2) = 1.41"), std::string( buf, 14) );
    }
    {
        std::uint64_t n = 0xbcdef1234567890;
        const char *fmt = "0x%016llX";
        char buf[100];
        snprintf( buf, sizeof( buf), fmt, n);
        BOOST_CHECK_EQUAL( std::string("0x0BCDEF1234567890"), std::string( buf, 18) );
    }
    ctx::jump_fcontext( t.fctx, 0);
}

void test_setup() {
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f1);
    BOOST_CHECK( ctx);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_start() {
    value1 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f1);
    BOOST_CHECK( ctx);

    BOOST_CHECK_EQUAL( 0, value1);
    ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 1, value1);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_jump() {
    value1 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f3);
    BOOST_CHECK( ctx);
    BOOST_CHECK_EQUAL( 0, value1);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 1, value1);
    ctx::jump_fcontext( t.fctx, 0);
    BOOST_CHECK_EQUAL( 2, value1);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_result() {
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f4);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, 0);
    int result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( 7, result);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_arg() {
    stack_allocator alloc;
    int i = 7;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f5);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, & i);
    int result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( i, result);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_transfer() {
    stack_allocator alloc;
    std::pair< int, int > data = std::make_pair( 3, 7);
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f6);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, & data);
    int result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( 10, result);
    data = std::make_pair( 7, 7);
    t = ctx::jump_fcontext( t.fctx, & data);
    result = * ( int *) t.data;
    BOOST_CHECK_EQUAL( 14, result);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_exception() {
    stack_allocator alloc;
    std::string what("hello world");
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f7);
    BOOST_CHECK( ctx);
    ctx::jump_fcontext( ctx, & what);
    BOOST_CHECK_EQUAL( std::string( what), value2);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_fp() {
    stack_allocator alloc;
    double d = 7.13;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f8);
    BOOST_CHECK( ctx);
    ctx::jump_fcontext( ctx, & d);
    BOOST_CHECK_EQUAL( 10.58, value3);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_stacked() {
    value1 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize());
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f9);
    ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 3, value1);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_ontop() {
    value1 = 0;
    value4 = 0;
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f12);
    BOOST_CHECK( ctx);
    ctx::transfer_t t = ctx::jump_fcontext( ctx, 0);
    BOOST_CHECK_EQUAL( 0, value1);
    BOOST_CHECK( 0 == value4);
    int i = -3;
    t = ctx::ontop_fcontext( t.fctx, & i, f11);
    BOOST_CHECK_EQUAL( -3, value1);
    BOOST_CHECK_EQUAL( & i, value4);
    BOOST_CHECK_EQUAL( -3, * ( int *) t.data);
    BOOST_CHECK_EQUAL( & i, ( int *) t.data);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_sscanf() {
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f13);
    BOOST_CHECK( ctx);
    ctx::jump_fcontext( ctx, 0);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

void test_snprintf() {
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), f14);
    ctx::jump_fcontext( ctx, 0);
	alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

int main()
{
    test_setup();
    test_start();
    test_jump();
    test_result();
    test_arg();
    test_transfer();
    test_exception();
    test_fp();
    test_stacked();
    test_ontop();
    test_sscanf();
    test_snprintf();

    return mars_boost::report_errors();
}
