
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

#include <boost/context/continuation.hpp>
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
    ctx::continuation foo( ctx::continuation && c, int i) {
        value1 = i;
        return std::move( c);
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
    ctx::continuation   c;
    my_exception( ctx::continuation && c_, char const* what) :
        std::runtime_error( what),
        c{ std::move( c_) } {
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
    ctx::continuation c1 = ctx::callcc(
        [&i](ctx::continuation && c) {
            value1 = i;
            c = c.resume();
            value1 = i;
            return std::move( c);
        });
    BOOST_CHECK_EQUAL( 1, value1);
    BOOST_CHECK( c1);
    ctx::continuation c2;
    BOOST_CHECK( ! c2);
    c2 = std::move( c1);
    BOOST_CHECK( ! c1);
    BOOST_CHECK( c2);
    i = 3;
    c2.resume();
    BOOST_CHECK_EQUAL( 3, value1);
    BOOST_CHECK( ! c1);
    BOOST_CHECK( ! c2);
}

void test_bind() {
    value1 = 0;
    X x;
    ctx::continuation c = ctx::callcc( std::bind( & X::foo, x, std::placeholders::_1, 7) );
    BOOST_CHECK_EQUAL( 7, value1);
}

void test_exception() {
    {
        const char * what = "hello world";
        ctx::continuation c = ctx::callcc(
            [&what](ctx::continuation && c) {
                try {
                    throw std::runtime_error( what);
                } catch ( std::runtime_error const& e) {
                    value2 = e.what();
                }
                return std::move( c);
            });
        BOOST_CHECK_EQUAL( std::string( what), value2);
        BOOST_CHECK( ! c);
    }
#ifdef BOOST_MSVC
    {
        bool catched = false;
        std::thread([&catched](){
                ctx::continuation c = ctx::callcc([&catched](ctx::continuation && c){
                            c = c.resume();
                            seh( catched);
                            return std::move( c);
                        });
            BOOST_CHECK( c );
            c.resume();
        }).join();
        BOOST_CHECK( catched);
    }
#endif
}

void test_fp() {
    value3 = 0.;
    double d = 7.13;
    ctx::continuation c = ctx::callcc(
        [&d]( ctx::continuation && c) {
            d += 3.45;
            value3 = d;
            return std::move( c);
        });
    BOOST_CHECK_EQUAL( 10.58, value3);
    BOOST_CHECK( ! c);
}

void test_stacked() {
    value1 = 0;
    value3 = 0.;
    ctx::continuation c = ctx::callcc(
        [](ctx::continuation && c) {
            ctx::continuation c1 = ctx::callcc(
                [](ctx::continuation && c) {
                    value1 = 3;
                    return std::move( c);
                });
            value3 = 3.14;
            return std::move( c);
        });
    BOOST_CHECK_EQUAL( 3, value1);
    BOOST_CHECK_EQUAL( 3.14, value3);
    BOOST_CHECK( ! c );
}

void test_prealloc() {
    value1 = 0;
    ctx::default_stack alloc;
    ctx::stack_context sctx( alloc.allocate() );
    void * sp = static_cast< char * >( sctx.sp) - 10;
    std::size_t size = sctx.size - 10;
    int i = 7;
    ctx::continuation c = ctx::callcc(
        std::allocator_arg, ctx::preallocated( sp, size, sctx), alloc,
        [&i]( ctx::continuation && c) {
            value1 = i;
            return std::move( c);
        });
    BOOST_CHECK_EQUAL( 7, value1);
    BOOST_CHECK( ! c);
}

void test_ontop() {
    {
        int i = 3;
        ctx::continuation c = ctx::callcc([&i](ctx::continuation && c) {
                    for (;;) {
                        i *= 10;
                        c = c.resume();
                    }
                    return std::move( c);
                });
        c = c.resume_with(
               [&i](ctx::continuation && c){
                   i -= 10;
                   return std::move( c);
               });
        BOOST_CHECK( c);
        BOOST_CHECK_EQUAL( i, 200);
    }
    {
        ctx::continuation c1;
        ctx::continuation c = ctx::callcc([&c1](ctx::continuation && c) {
                    c = c.resume();
                    BOOST_CHECK( ! c);
                    return std::move( c1);
                });
        c = c.resume_with(
               [&c1](ctx::continuation && c){
                   c1 = std::move( c);
                   return std::move( c);
               });
    }
}

void test_ontop_exception() {
    value1 = 0;
    value2 = "";
    ctx::continuation c = ctx::callcc([](ctx::continuation && c){
            for (;;) {
                value1 = 3;
                try {
                    c = c.resume();
                } catch ( my_exception & ex) {
                    value2 = ex.what();
                    return std::move( ex.c); 
                }
            }
            return std::move( c);
    });
    c = c.resume();
    BOOST_CHECK_EQUAL( 3, value1);
    const char * what = "hello world";
    c.resume_with(
       [what](ctx::continuation && c){
            throw my_exception( std::move( c), what);
            return std::move( c);
       });
    BOOST_CHECK_EQUAL( 3, value1);
    BOOST_CHECK_EQUAL( std::string( what), value2);
}

void test_termination1() {
    {
        value1 = 0;
        ctx::continuation c = ctx::callcc(
            [](ctx::continuation && c){
                Y y;
                return c.resume();
            });
        BOOST_CHECK_EQUAL( 3, value1);
    }
    BOOST_CHECK_EQUAL( 7, value1);
    {
        value1 = 0;
        BOOST_CHECK_EQUAL( 0, value1);
        ctx::continuation c = ctx::callcc(
            [](ctx::continuation && c) {
                value1 = 3;
                return std::move( c);
            });
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK( ! c );
    }
    {
        value1 = 0;
        BOOST_CHECK_EQUAL( 0, value1);
        int i = 3;
        ctx::continuation c = ctx::callcc(
            [&i](ctx::continuation && c){
                value1 = i;
                c = c.resume();
                value1 = i;
                return std::move( c);
            });
        BOOST_CHECK( c);
        BOOST_CHECK_EQUAL( i, value1);
        BOOST_CHECK( c);
        i = 7;
        c = c.resume();
        BOOST_CHECK( ! c);
        BOOST_CHECK_EQUAL( i, value1);
    }
}

void test_termination2() {
    {
        value1 = 0;
        value3 = 0.0;
        ctx::continuation c = ctx::callcc(
            [](ctx::continuation && c){
                Y y;
                value1 = 3;
                value3 = 4.;
                c = c.resume();
                value1 = 7;
                value3 = 8.;
                c = c.resume();
                return std::move( c);
            });
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 4., value3);
        c = c.resume();
    }
    BOOST_CHECK_EQUAL( 7, value1);
    BOOST_CHECK_EQUAL( 8., value3);
}

void test_sscanf() {
    ctx::continuation c = ctx::callcc(
		[]( ctx::continuation && c) {
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
			return std::move( c);
	});
}

void test_snprintf() {
    ctx::continuation c = ctx::callcc(
		[]( ctx::continuation && c) {
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
			return std::move( c);
	});
}

#ifdef BOOST_WINDOWS
void test_bug12215() {
        ctx::continuation c = ctx::callcc(
            [](ctx::continuation && c) {
                char buffer[MAX_PATH];
                GetModuleFileName( nullptr, buffer, MAX_PATH);
                return std::move( c);
            });
}
#endif

void test_goodcatch() {
    value1 = 0;
    value3 = 0.0;
    {
        ctx::continuation c = ctx::callcc(
            [](ctx::continuation && c) {
                Y y;
                value3 = 2.;
                c = c.resume();
                try {
                    value3 = 3.;
                    c = c.resume();
                } catch ( mars_boost::context::detail::forced_unwind const&) {
                    value3 = 4.;
                    throw;
                } catch (...) {
                    value3 = 5.;
                }
                value3 = 6.;
                return std::move( c);
            });
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 2., value3);
        c = c.resume();
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
        ctx::continuation c = ctx::callcc(
            [](ctx::continuation && c) {
                Y y;
                try {
                    value3 = 3.;
                    c = c.resume();
                } catch (...) {
                    value3 = 5.;
                }
                return std::move( c);
            });
        BOOST_CHECK_EQUAL( 3, value1);
        BOOST_CHECK_EQUAL( 3., value3);
        // the destruction of ctx here will cause a forced_unwind to be thrown that is not caught
        // in fn19.  That will trigger the "not caught" assertion in ~forced_unwind.  Getting that 
        // assertion to propogate bak here cleanly is non-trivial, and there seems to not be a good
        // way to hook directly into the assertion when it happens on an alternate stack.
        std::move( c);
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
