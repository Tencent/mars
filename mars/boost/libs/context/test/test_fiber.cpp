
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stdio.h>
#include <stdlib.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <boost/variant.hpp>

#include <boost/context/fiber.hpp>
#include <boost/context/detail/config.hpp>

#ifdef BOOST_WINDOWS
#include <windows.h>
#endif

#define BOOST_CHECK(x) BOOST_TEST(x)
#define BOOST_CHECK_EQUAL(a, b) BOOST_TEST_EQ(a, b)

#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable: 4702 4723 4996)
#endif

typedef mars_boost::variant<int,std::string> variant_t;

namespace ctx = mars_boost::context;

int value1 = 0;
std::string value2;
double value3 = 0.;

struct X {
    ctx::fiber foo( ctx::fiber && f, int i) {
        value1 = i;
        return std::move( f);
    }
};

struct Y {
    Y() {
        value1 = 3;
    }

    Y( Y const&) = delete;
    Y & operator=( Y const&) = delete;

    ~Y() {
        value1 = 7;
    }
};

class moveable {
public:
    bool    state;
    int     value;

    moveable() :
        state( false),
        value( -1) {
    }

    moveable( int v) :
        state( true),
        value( v) {
    }

    moveable( moveable && other) :
        state( other.state),
        value( other.value) {
        other.state = false;
        other.value = -1;
    }

    moveable & operator=( moveable && other) {
        if ( this == & other) return * this;
        state = other.state;
        value = other.value;
        other.state = false;
        other.value = -1;
        return * this;
    }

    moveable( moveable const& other) = delete;
    moveable & operator=( moveable const& other) = delete;

    void operator()() {
        value1 = value;
    }
};

struct my_exception : public std::runtime_error {
    ctx::fiber   f;
    my_exception( ctx::fiber && f_, char const* what) :
        std::runtime_error( what),
        f{ std::move( f_) } {
    }
};

#ifdef BOOST_MSVC
// Optimizations can remove the integer-divide-by-zero here.
#pragma optimize("", off)
void seh( bool & catched) {
    __try {
        int i = 1;
        i /= 0;
    } __except( EXCEPTION_EXECUTE_HANDLER) {
        catched = true;
    }
}
#pragma optimize("", on)
#endif

void test_move() {
    value1 = 0;
    int i = 1;
    BOOST_CHECK_EQUAL( 0, value1);
    ctx::fiber f1{
        [&i](ctx::fiber && f) {
            value1 = i;
            f = std::move( f).resume();
            value1 = i;
            return std::move( f);
        }};
    f1 = std::move( f1).resume();
    BOOST_CHECK_EQUAL( 1, value1);
    BOOST_CHECK( f1);
    ctx::fiber f2;
    BOOST_CHECK( ! f2);
    f2 = std::move( f1);
    BOOST_CHECK( ! f1);
    BOOST_CHECK( f2);
    i = 3;
    f2 = std::move( f2).resume();
    BOOST_CHECK_EQUAL( 3, value1);
    BOOST_CHECK( ! f1);
    BOOST_CHECK( ! f2);
}

void test_bind() {
    value1 = 0;
    X x;
    ctx::fiber f{ std::bind( & X::foo, x, std::placeholders::_1, 7) };
    f = std::move( f).resume();
    BOOST_CHECK_EQUAL( 7, value1);
}

void test_exception() {
    {
        const char * what = "hello world";
        ctx::fiber f{
            [&what](ctx::fiber && f) {
                try {
                    throw std::runtime_error( what);
                } catch ( std::runtime_error const& e) {
                    value2 = e.what();
                }
                return std::move( f);
            }};
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( std::string( what), value2);
        BOOST_CHECK( ! f);
    }
#ifdef BOOST_MSVC
    {
        bool catched = false;
        std::thread([&catched](){
                ctx::fiber f{ [&catched](ctx::fiber && f){
                            seh( catched);
                            return std::move( f);
                        }};
            BOOST_CHECK( f);
            f = std::move( f).resume();
        }).join();
        BOOST_CHECK( catched);
    }
#endif
}

void test_fp() {
    value3 = 0.;
    double d = 7.13;
    ctx::fiber f{
        [&d]( ctx::fiber && f) {
            d += 3.45;
            value3 = d;
            return std::move( f);
        }};
    f = std::move( f).resume();
    BOOST_CHECK_EQUAL( 10.58, value3);
    BOOST_CHECK( ! f);
}

void test_stacked() {
    value1 = 0;
    value3 = 0.;
    ctx::fiber f{
        [](ctx::fiber && f) {
            ctx::fiber f1{
                [](ctx::fiber && f) {
                    value1 = 3;
                    return std::move( f);
                }};
            f1 = std::move( f1).resume();
            value3 = 3.14;
            return std::move( f);
        }};
    f = std::move( f).resume();
    BOOST_CHECK_EQUAL( 3, value1);
    BOOST_CHECK_EQUAL( 3.14, value3);
    BOOST_CHECK( ! f);
}

void test_prealloc() {
    value1 = 0;
    ctx::default_stack alloc;
    ctx::stack_context sctx( alloc.allocate() );
    void * sp = static_cast< char * >( sctx.sp) - 10;
    std::size_t size = sctx.size - 10;
    int i = 7;
    ctx::fiber f{
        std::allocator_arg, ctx::preallocated( sp, size, sctx), alloc,
        [&i]( ctx::fiber && f) {
            value1 = i;
            return std::move( f);
        }};
    f = std::move( f).resume();
    BOOST_CHECK_EQUAL( 7, value1);
    BOOST_CHECK( ! f);
}

void test_ontop() {
    {
        int i = 3;
        ctx::fiber f{ [&i](ctx::fiber && f) {
                    for (;;) {
                        i *= 10;
                        f = std::move( f).resume();
                    }
                    return std::move( f);
                }};
        f = std::move( f).resume();
        // Pass fn by reference to see if the types are properly decayed.
        auto fn = [&i](ctx::fiber && f){
                   i -= 10;
                   return std::move( f);
        };
        f = std::move( f).resume_with(fn);
        BOOST_CHECK( f);
        BOOST_CHECK_EQUAL( i, 200);
    }
    {
        ctx::fiber f1;
        ctx::fiber f{ [&f1](ctx::fiber && f) {
                    f = std::move( f).resume();
                    BOOST_CHECK( ! f);
                    return std::move( f1);
                }};
        f = std::move( f).resume();
        f = std::move( f).resume_with(
               [&f1](ctx::fiber && f){
                   f1 = std::move( f);
                   return std::move( f);
               });
    }
}

void test_ontop_exception() {
    value1 = 0;
    value2 = "";
    ctx::fiber f{ [](ctx::fiber && f){
            for (;;) {
                value1 = 3;
                try {
                    f = std::move( f).resume();
                } catch ( my_exception & ex) {
                    value2 = ex.what();
                    return std::move( ex.f);
                }
            }
            return std::move( f);
    }};
    f = std::move( f).resume();
    BOOST_CHECK_EQUAL( 3, value1);
    const char * what = "hello world";
    f = std::move( f).resume_with(
       [what](ctx::fiber && f){
            throw my_exception( std::move( f), what);
            return std::move( f);
       });
    BOOST_CHECK_EQUAL( 3, value1);
    BOOST_CHECK_EQUAL( std::string( what), value2);
}

void test_termination1() {
    {
        value1 = 0;
        ctx::fiber f{
            [](ctx::fiber && f){
                Y y;
                f = std::move( f).resume();
                return std::move(f);
            }};
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( 3, value1);
    }
    BOOST_CHECK_EQUAL( 7, value1);
    {
        value1 = 0;
        BOOST_CHECK_EQUAL( 0, value1);
        ctx::fiber f{
            [](ctx::fiber && f) {
                value1 = 3;
                return std::move( f);
            }};
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK( ! f);
    }
    {
        value1 = 0;
        BOOST_CHECK_EQUAL( 0, value1);
        int i = 3;
        ctx::fiber f{
            [&i](ctx::fiber && f){
                value1 = i;
                f = std::move( f).resume();
                value1 = i;
                return std::move( f);
            }};
        f = std::move( f).resume();
        BOOST_CHECK( f);
        BOOST_CHECK_EQUAL( i, value1);
        BOOST_CHECK( f);
        i = 7;
        f = std::move( f).resume();
        BOOST_CHECK( ! f);
        BOOST_CHECK_EQUAL( i, value1);
    }
}

void test_termination2() {
    {
        value1 = 0;
        value3 = 0.0;
        ctx::fiber f{
            [](ctx::fiber && f){
                Y y;
                value1 = 3;
                value3 = 4.;
                f = std::move( f).resume();
                value1 = 7;
                value3 = 8.;
                f = std::move( f).resume();
                return std::move( f);
            }};
        BOOST_CHECK_EQUAL( 0, value1);
        BOOST_CHECK_EQUAL( 0., value3);
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 4., value3);
        f = std::move( f).resume();
    }
    BOOST_CHECK_EQUAL( 7, value1);
    BOOST_CHECK_EQUAL( 8., value3);
}

void test_sscanf() {
    ctx::fiber{
		[]( ctx::fiber && f) {
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
			return std::move( f);
	}}.resume();
}

void test_snprintf() {
    ctx::fiber{
		[]( ctx::fiber && f) {
            {
                const char *fmt = "sqrt(2) = %f";
                char buf[19];
                snprintf( buf, sizeof( buf), fmt, std::sqrt( 2) );
                BOOST_CHECK( 0 < sizeof( buf) );
                BOOST_ASSERT( std::string("sqrt(2) = 1.41") == std::string( buf, 14) );
            }
            {
                std::uint64_t n = 0xbcdef1234567890;
                const char *fmt = "0x%016llX";
                char buf[100];
                snprintf( buf, sizeof( buf), fmt, n);
                BOOST_ASSERT( std::string("0x0BCDEF1234567890") == std::string( buf, 18) );
            }
			return std::move( f);
	}}.resume();
}

#ifdef BOOST_WINDOWS
void test_bug12215() {
        ctx::fiber{
            [](ctx::fiber && f) {
                char buffer[MAX_PATH];
                GetModuleFileName( nullptr, buffer, MAX_PATH);
                return std::move( f);
            }}.resume();
}
#endif

void test_goodcatch() {
    value1 = 0;
    value3 = 0.0;
    {
        ctx::fiber f{
            []( ctx::fiber && f) {
                Y y;
                value3 = 2.;
                f = std::move( f).resume();
                try {
                    value3 = 3.;
                    f = std::move( f).resume();
                } catch ( mars_boost::context::detail::forced_unwind const&) {
                    value3 = 4.;
                    throw;
                } catch (...) {
                    value3 = 5.;
                }
                value3 = 6.;
                return std::move( f);
            }};
        BOOST_CHECK_EQUAL( 0, value1);
        BOOST_CHECK_EQUAL( 0., value3);
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 2., value3);
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 3., value3);
    }
    BOOST_CHECK_EQUAL( 7, value1);
    BOOST_CHECK_EQUAL( 4., value3);
}

void test_badcatch() {
#if 0
    value1 = 0;
    value3 = 0.;
    {
        ctx::fiber f{
            []( ctx::fiber && f) {
                Y y;
                try {
                    value3 = 3.;
                    f = std::move( f).resume();
                } catch (...) {
                    value3 = 5.;
                }
                return std::move( f);
            }};
        BOOST_CHECK_EQUAL( 0, value1);
        BOOST_CHECK_EQUAL( 0., value3);
        f = std::move( f).resume();
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 3., value3);
        // the destruction of ctx here will cause a forced_unwind to be thrown that is not caught
        // in fn19.  That will trigger the "not caught" assertion in ~forced_unwind.  Getting that
        // assertion to propogate bak here cleanly is non-trivial, and there seems to not be a good
        // way to hook directly into the assertion when it happens on an alternate stack.
        std::move( f);
    }
    BOOST_CHECK_EQUAL( 7, value1);
    BOOST_CHECK_EQUAL( 4., value3);
#endif
}

int main()
{
    test_move();
    test_bind();
    test_exception();
    test_fp();
    test_stacked();
    test_prealloc();
    test_ontop();
    test_ontop_exception();
    test_termination1();
    test_termination2();
    test_sscanf();
    test_snprintf();
#ifdef BOOST_WINDOWS
    test_bug12215();
#endif
    test_goodcatch();
    test_badcatch();

    return mars_boost::report_errors();
}

#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif
