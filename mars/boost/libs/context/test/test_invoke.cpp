
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/core/lightweight_test.hpp>

#if defined(BOOST_NO_CXX17_STD_INVOKE)
#include <boost/context/detail/invoke.hpp>
#include <boost/context/detail/config.hpp>

#define BOOST_CHECK_EQUAL(a, b) BOOST_TEST_EQ(a, b)

namespace ctx = mars_boost::context;

struct callable {
    int k{ 0 };

    callable() = default;

    callable( int k_) :
        k{ k_ } {
    }

    int foo( int i, int j) const {
        return i + j + k;
    }

    int operator()( int i, int j) const {
        return foo( i, j);
    }
};

struct movable {
    int k{ 0 };

    movable() = default;

    movable( int k_) :
        k{ k_ } {
    }

    movable( movable const&) = delete;
    movable & operator=( movable const&) = delete;

    movable( movable && other) :
        k{ other.k } {
        other.k = -1;
    }

    movable & operator=( movable && other) {
        if ( this == & other) return * this;
        k = other.k;
        other.k = -1;
        return * this;
    }

    int foo( int i, int j) const {
        return i + j + k;
    }

    int operator()( int i, int j) const {
        return foo( i, j);
    }
};

int fn1( int i, int j) {
    return i + j;
}

int * fn2( int * ip) {
    return ip;
}

int * fn3( int & ir) {
    return & ir;
}

int & fn4( int & ir) {
    return ir;
}

template< typename T >
int fn5( int i, T && t_) {
    T t = std::forward< T >( t_);
    return i + t.k;
}

void test1() {
    int result = ctx::detail::invoke( fn1, 1, 2);
    BOOST_CHECK_EQUAL( result, 3);
}

void test2() {
    {
        int i = 3;
        int * ip = & i;
        int * result = ctx::detail::invoke( fn2, ip);
        BOOST_CHECK_EQUAL( result, ip);
        BOOST_CHECK_EQUAL( * result, i);
    }
    {
        int i = 3;
        int * result = ctx::detail::invoke( fn2, & i);
        BOOST_CHECK_EQUAL( result, & i);
        BOOST_CHECK_EQUAL( * result, i);
    }
}

void test3() {
    {
        int i = 3;
        int & ir = i;
        int * result = ctx::detail::invoke( fn3, ir);
        BOOST_CHECK_EQUAL( result, & ir);
        BOOST_CHECK_EQUAL( * result, i);
    }
    {
        int i = 3;
        int * result = ctx::detail::invoke( fn3, i);
        BOOST_CHECK_EQUAL( result, & i);
        BOOST_CHECK_EQUAL( * result, i);
    }
}

void test4() {
    {
        int i = 3;
        int & ir = i;
        int & result = ctx::detail::invoke( fn4, ir);
        BOOST_CHECK_EQUAL( result, ir);
        BOOST_CHECK_EQUAL( & result, & ir);
        BOOST_CHECK_EQUAL( result, i);
    }
    {
        int i = 3;
        int & result = ctx::detail::invoke( fn4, i);
        BOOST_CHECK_EQUAL( & result, & i);
        BOOST_CHECK_EQUAL( result, i);
    }
}

void test5() {
    {
        callable c( 5);
        int result = ctx::detail::invoke( fn5< callable >, 1, std::move( c) );
        BOOST_CHECK_EQUAL( result, 6);
        BOOST_CHECK_EQUAL( c.k, 5);
    }
    {
        movable m( 5);
        int result = ctx::detail::invoke( fn5< movable >, 1, std::move( m) );
        BOOST_CHECK_EQUAL( result, 6);
        BOOST_CHECK_EQUAL( m.k, -1);
    }
}

void test6() {
    {
        callable c;
        int result = ctx::detail::invoke( c, 1, 2);
        BOOST_CHECK_EQUAL( result, 3);
        BOOST_CHECK_EQUAL( c.k, 0);
    }
    {
        callable c;
        int result = ctx::detail::invoke( & callable::foo, c, 1, 2);
        BOOST_CHECK_EQUAL( result, 3);
        BOOST_CHECK_EQUAL( c.k, 0);
    }
}

void test7() {
    {
        int result = ctx::detail::invoke( movable{}, 1, 2);
        BOOST_CHECK_EQUAL( result, 3);
    }
    {
        int result = ctx::detail::invoke( & movable::foo, movable{}, 1, 2);
        BOOST_CHECK_EQUAL( result, 3);
    }
}

template< typename R, typename Fn, typename ... Args >
R apply( Fn && fn, Args && ... args) {
    return ctx::detail::invoke(
            std::forward< Fn >( fn),
            std::forward< Args >( args) ... );
}

void test8() {
    {
        int result = apply< int >( fn1, 1, 2);
        BOOST_CHECK_EQUAL( result, 3);
    }
    {
        int i = 3;
        int & ir = i;
        int * result = apply< int * >( fn3, ir);
        BOOST_CHECK_EQUAL( result, & ir);
        BOOST_CHECK_EQUAL( * result, i);
    }
    {
        movable m( 5);
        int result = apply< int >( fn5< movable >, 1, std::move( m) );
        BOOST_CHECK_EQUAL( result, 6);
        BOOST_CHECK_EQUAL( m.k, -1);
    }
}
#else
void dummy() {}
#endif

int main()
{
#if defined(BOOST_NO_CXX17_STD_INVOKE)
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
#else
    dummy();
#endif

    return mars_boost::report_errors();
}
