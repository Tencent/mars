//
//  pointer_cast_test2.cpp - a test for unique_ptr casts
//
//  Copyright 2016 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/config.hpp>

#if defined( BOOST_NO_CXX11_SMART_PTR )

int main()
{
    return 0;
}

#else

#include <boost/pointer_cast.hpp>
#include <boost/core/lightweight_test.hpp>
#include <memory>

struct B1
{
};

struct D1: B1
{
    ~D1()
    {
    }
};

static void test_static_cast()
{
    {
        std::unique_ptr<int> p1( new int );
        int * q1 = p1.get();

        std::unique_ptr<int> p2 = mars_boost::static_pointer_cast<int>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }

    {
        std::unique_ptr<int> p1( new int );
        int * q1 = p1.get();

        std::unique_ptr<int const> p2 = mars_boost::static_pointer_cast<int const>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }

    {
        std::unique_ptr<int[]> p1( new int[ 1 ] );
        int * q1 = p1.get();

        std::unique_ptr<int[]> p2 = mars_boost::static_pointer_cast<int[]>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }

    {
        std::unique_ptr<int[]> p1( new int[ 1 ] );
        int * q1 = p1.get();

        std::unique_ptr<int const[]> p2 = mars_boost::static_pointer_cast<int const[]>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }

    {
        std::unique_ptr<D1> p1( new D1 );
        D1 * q1 = p1.get();

        std::unique_ptr<B1> p2 = mars_boost::static_pointer_cast<B1>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );

        std::unique_ptr<D1> p3 = mars_boost::static_pointer_cast<D1>( std::move( p2 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p3.get(), q1 );
    }
}

static void test_const_cast()
{
    {
        std::unique_ptr<int> p1( new int );
        int * q1 = p1.get();

        std::unique_ptr<int const> p2 = mars_boost::const_pointer_cast<int const>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );

        std::unique_ptr<int> p3 = mars_boost::const_pointer_cast<int>( std::move( p2 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p3.get(), q1 );
    }

    {
        std::unique_ptr<int[]> p1( new int[ 1 ] );
        int * q1 = p1.get();

        std::unique_ptr<int const[]> p2 = mars_boost::const_pointer_cast<int const[]>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );

        std::unique_ptr<int[]> p3 = mars_boost::const_pointer_cast<int[]>( std::move( p2 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p3.get(), q1 );
    }
}

struct B2
{
    virtual ~B2()
    {
    }
};

struct C2
{
    virtual ~C2()
    {
    }
};

struct D2: B2, C2
{
};

static void test_dynamic_cast()
{
    {
        std::unique_ptr<D2> p1( new D2 );
        D2 * q1 = p1.get();

        std::unique_ptr<B2> p2 = mars_boost::dynamic_pointer_cast<B2>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }

    {
        std::unique_ptr<B2> p1( new D2 );
        B2 * q1 = p1.get();

        std::unique_ptr<D2> p2 = mars_boost::dynamic_pointer_cast<D2>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }

    {
        std::unique_ptr<B2> p1( new B2 );
        B2 * q1 = p1.get();

        std::unique_ptr<D2> p2 = mars_boost::dynamic_pointer_cast<D2>( std::move( p1 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p1.get(), q1 );
    }

    {
        D2 * q1 = new D2;
        std::unique_ptr<B2> p1( q1 );

        std::unique_ptr<C2> p2 = mars_boost::dynamic_pointer_cast<C2>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );
    }
}

static void test_reinterpret_cast()
{
    {
        std::unique_ptr<int> p1( new int );
        void * q1 = p1.get();

        std::unique_ptr<char> p2 = mars_boost::reinterpret_pointer_cast<char>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );

        p1 = mars_boost::reinterpret_pointer_cast<int>( std::move( p2 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p1.get(), q1 );
    }

    {
        std::unique_ptr<int> p1( new int );
        void * q1 = p1.get();

        std::unique_ptr<char[]> p2 = mars_boost::reinterpret_pointer_cast<char[]>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );

        p1 = mars_boost::reinterpret_pointer_cast<int>( std::move( p2 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p1.get(), q1 );
    }

    {
        std::unique_ptr<int[]> p1( new int[ 1 ] );
        void * q1 = p1.get();

        std::unique_ptr<char[]> p2 = mars_boost::reinterpret_pointer_cast<char[]>( std::move( p1 ) );

        BOOST_TEST( p1.get() == 0 );
        BOOST_TEST_EQ( p2.get(), q1 );

        p1 = mars_boost::reinterpret_pointer_cast<int[]>( std::move( p2 ) );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST_EQ( p1.get(), q1 );
    }
}

int main()
{
    test_static_cast();
    test_const_cast();
    test_dynamic_cast();
    test_reinterpret_cast();

    return mars_boost::report_errors();
}

#endif
