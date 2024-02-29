//
// local_sp_test.cpp
//
// Copyright 2002, 2003, 2017 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

struct X
{
    static long instances;

    X()
    {
        ++instances;
    }

    ~X()
    {
        --instances;
    }

private:

    X( X const & );
    X & operator=( X const & );
};

long X::instances = 0;

class incomplete;

// default constructor

static void default_constructor()
{
    {
        mars_boost::local_shared_ptr<int> p;

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

    {
        mars_boost::local_shared_ptr<void> p;

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

    {
        mars_boost::local_shared_ptr<incomplete> p;

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::local_shared_ptr<X> p;

        BOOST_TEST( X::instances == 0 );

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }
}

// nullptr_constructor

static void nullptr_constructor()
{
#if !defined( BOOST_NO_CXX11_NULLPTR )

    {
        mars_boost::local_shared_ptr<int> p( nullptr );

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

    {
        mars_boost::local_shared_ptr<void> p( nullptr );

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

    {
        mars_boost::local_shared_ptr<incomplete> p( nullptr );

        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::local_shared_ptr<X> p( nullptr );

        BOOST_TEST( X::instances == 0 );


        BOOST_TEST_EQ( p.get(), static_cast<void*>(0) );
        BOOST_TEST_EQ( p.local_use_count(), 0 );
    }

#endif
}

// pointer constructor

template<class T, class U> static void pc0_test_()
{
    mars_boost::local_shared_ptr<T> p( static_cast<U*>( 0 ) );

    BOOST_TEST( p? false: true );
    BOOST_TEST( !p );
    BOOST_TEST( p.get() == 0 );
    BOOST_TEST( p.local_use_count() == 1 );
}

template<class T> static void pc0_test()
{
    pc0_test_<T, T>();
    pc0_test_<T const, T const>();
    pc0_test_<T volatile, T volatile>();
    pc0_test_<T const volatile, T const volatile>();

    pc0_test_<T const, T>();
    pc0_test_<T volatile, T>();
    pc0_test_<T const volatile, T>();

    pc0_test_<void, T>();
    pc0_test_<void const, T>();
    pc0_test_<void volatile, T>();
    pc0_test_<void const volatile, T>();
}

template<class T, class U> static void pc1_test_()
{
    mars_boost::local_shared_ptr<T> p( new U() );

    BOOST_TEST( p? true: false );
    BOOST_TEST( !!p );
    BOOST_TEST( p.get() != 0 );
    BOOST_TEST( p.local_use_count() == 1 );
}

template<class T> static void pc1_test()
{
    pc1_test_<T, T>();
    pc1_test_<T const, T const>();
    pc1_test_<T volatile, T volatile>();
    pc1_test_<T const volatile, T const volatile>();

    pc1_test_<T const, T>();
    pc1_test_<T volatile, T>();
    pc1_test_<T const volatile, T>();

    pc1_test_<void, T>();
    pc1_test_<void const, T>();
    pc1_test_<void volatile, T>();
    pc1_test_<void const volatile, T>();
}

static void pointer_constructor()
{
    pc0_test<int>();
    pc0_test<X>();

    pc1_test<int>();

    BOOST_TEST( X::instances == 0 );

    pc1_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// deleter constructor

int m = 0;

void deleter2( int * p )
{
    BOOST_TEST( p == &m );
    ++*p;
}

template<class T> static void deleter2_test_()
{
    {
        m = 0;
        mars_boost::local_shared_ptr<T> p( &m, deleter2 );

        BOOST_TEST( p? true: false );
        BOOST_TEST( !!p );
        BOOST_TEST( p.get() == &m );
        BOOST_TEST( p.local_use_count() == 1 );
    }

    BOOST_TEST( m == 1 );
}

static void deleter_constructor()
{
    deleter2_test_<int>();
    deleter2_test_<int const>();
    deleter2_test_<int volatile>();
    deleter2_test_<int const volatile>();

    deleter2_test_<void>();
    deleter2_test_<void const>();
    deleter2_test_<void volatile>();
    deleter2_test_<void const volatile>();
}

// nullptr_deleter_constructor

#if !defined( BOOST_NO_CXX11_NULLPTR )

void deleter3( mars_boost::detail::sp_nullptr_t )
{
    ++m;
}

template<class T> static void deleter3_test_()
{
    {
        m = 0;
        mars_boost::local_shared_ptr<T> p( nullptr, deleter3 );

        BOOST_TEST( !p );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.local_use_count() == 1 );
    }

    BOOST_TEST( m == 1 );
}

static void nullptr_deleter_constructor()
{
    deleter3_test_<int>();
    deleter3_test_<int const>();
    deleter3_test_<int volatile>();
    deleter3_test_<int const volatile>();

    deleter3_test_<void>();
    deleter3_test_<void const>();
    deleter3_test_<void volatile>();
    deleter3_test_<void const volatile>();
}

#else

static void nullptr_deleter_constructor()
{
}

#endif

// allocator constructor

template<class T> static void allocator_test_()
{
    {
        m = 0;
        mars_boost::local_shared_ptr<T> p( &m, deleter2, std::allocator<void>() );

        BOOST_TEST( p? true: false );
        BOOST_TEST( !!p );
        BOOST_TEST( p.get() == &m );
        BOOST_TEST( p.local_use_count() == 1 );
    }

    BOOST_TEST( m == 1 );
}

static void allocator_constructor()
{
    allocator_test_<int>();
    allocator_test_<int const>();
    allocator_test_<int volatile>();
    allocator_test_<int const volatile>();

    allocator_test_<void>();
    allocator_test_<void const>();
    allocator_test_<void volatile>();
    allocator_test_<void const volatile>();
}

// nullptr_allocator_constructor

#if !defined( BOOST_NO_CXX11_NULLPTR )

template<class T> static void allocator3_test_()
{
    {
        m = 0;
        mars_boost::local_shared_ptr<T> p( nullptr, deleter3, std::allocator<void>() );

        BOOST_TEST( !p );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.local_use_count() == 1 );
    }

    BOOST_TEST( m == 1 );
}

static void nullptr_allocator_constructor()
{
    allocator3_test_<int>();
    allocator3_test_<int const>();
    allocator3_test_<int volatile>();
    allocator3_test_<int const volatile>();

    allocator3_test_<void>();
    allocator3_test_<void const>();
    allocator3_test_<void volatile>();
    allocator3_test_<void const volatile>();
}

#else

static void nullptr_allocator_constructor()
{
}

#endif

// copy constructor

template<class T> static void empty_copy_test()
{
    mars_boost::local_shared_ptr<T> p1;

    BOOST_TEST_EQ( p1.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p1.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T> p2( p1 );

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( p1 );

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( p1 );

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( p3 );

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_copy( mars_boost::local_shared_ptr<U> const & p1 )
{
    long k = p1.local_use_count();

    {
        mars_boost::local_shared_ptr<T> p2( p1 );

        BOOST_TEST( p2.get() == p1.get() );
        BOOST_TEST( p2.local_use_count() == p1.local_use_count() );
        BOOST_TEST( p2.local_use_count() == k + 1 );
    }

    BOOST_TEST( p1.local_use_count() == k );
}

template<class T> static void null_copy_test()
{
    mars_boost::local_shared_ptr<T> p1( static_cast<T*>(0) );

    test_nonempty_copy<T>( p1 );
    test_nonempty_copy<T const>( p1 );
    test_nonempty_copy<T volatile>( p1 );
    test_nonempty_copy<T const volatile>( p1 );
    test_nonempty_copy<void>( p1 );
    test_nonempty_copy<void const>( p1 );
    test_nonempty_copy<void volatile>( p1 );
    test_nonempty_copy<void const volatile>( p1 );
}

template<class T> static void new_copy_test()
{
    mars_boost::local_shared_ptr<T> p1( new T() );

    test_nonempty_copy<T>( p1 );
    test_nonempty_copy<T const>( p1 );
    test_nonempty_copy<T volatile>( p1 );
    test_nonempty_copy<T const volatile>( p1 );
    test_nonempty_copy<void>( p1 );
    test_nonempty_copy<void const>( p1 );
    test_nonempty_copy<void volatile>( p1 );
    test_nonempty_copy<void const volatile>( p1 );
}

static void copy_constructor()
{
    empty_copy_test<int>();
    empty_copy_test<incomplete>();
    empty_copy_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_copy_test<int>();
    null_copy_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_copy_test<int>();
    new_copy_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// move constructor

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

template<class T> static void empty_move_test()
{
    mars_boost::local_shared_ptr<T> p2(( mars_boost::local_shared_ptr<T>() ));

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3(( mars_boost::local_shared_ptr<T>() ));

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4(( mars_boost::local_shared_ptr<T>() ));

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( std::move(p3) );

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_move( mars_boost::local_shared_ptr<U> && p1 )
{
    U* q = p1.get();
    long k = p1.local_use_count();

    mars_boost::local_shared_ptr<T> p2( std::move(p1) );

    BOOST_TEST( p2.get() == q );
    BOOST_TEST( p2.local_use_count() == k );

    BOOST_TEST( p1.get() == 0 );
    BOOST_TEST( p1.local_use_count() == 0 );
}

template<class T> static void null_move_test()
{
    test_nonempty_move<T>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<T const>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<T volatile>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<T const volatile>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<void>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<void const>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<void volatile>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move<void const volatile>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
}

template<class T> static void new_move_test()
{
    test_nonempty_move<T>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<T const>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<T volatile>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<T const volatile>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<void>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<void const>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<void volatile>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move<void const volatile>( mars_boost::local_shared_ptr<T>( new T() ) );
}

static void move_constructor()
{
    empty_move_test<int>();
    empty_move_test<incomplete>();
    empty_move_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_move_test<int>();
    null_move_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_move_test<int>();
    new_move_test<X>();

    BOOST_TEST( X::instances == 0 );
}

#else

static void move_constructor()
{
}

#endif

// aliasing constructor

template<class T, class U> void test_aliasing_( mars_boost::local_shared_ptr<T> const & p1, U * p2 )
{
    mars_boost::local_shared_ptr<U> p3( p1, p2 );

    BOOST_TEST( p3.get() == p2 );
    BOOST_TEST( p3.local_use_count() == p1.local_use_count() );
    BOOST_TEST( !p3.owner_before( p1 ) && !p1.owner_before( p3 ) );
}

template<class T, class U> void test_01_aliasing_()
{
    U u;
    mars_boost::local_shared_ptr<T> p1;

    test_aliasing_( p1, &u );
}

template<class T, class U> void test_01_aliasing()
{
    test_01_aliasing_<T, U>();
    test_01_aliasing_<T const, U>();
    test_01_aliasing_<T volatile, U>();
    test_01_aliasing_<T const volatile, U>();

    test_01_aliasing_<T, U volatile>();
    test_01_aliasing_<T const, U volatile>();
    test_01_aliasing_<T volatile, U volatile>();
    test_01_aliasing_<T const volatile, U volatile>();
}

template<class T, class U> void test_10_aliasing_()
{
    mars_boost::local_shared_ptr<T> p1( new T() );
    test_aliasing_( p1, static_cast<U*>(0) );
}

template<class T, class U> void test_10_aliasing()
{
    test_10_aliasing_<T, U>();
    test_10_aliasing_<T const, U>();
    test_10_aliasing_<T volatile, U>();
    test_10_aliasing_<T const volatile, U>();

    test_10_aliasing_<T, U const>();
    test_10_aliasing_<T const, U const>();
    test_10_aliasing_<T volatile, U const>();
    test_10_aliasing_<T const volatile, U const>();

    test_10_aliasing_<T, U volatile>();
    test_10_aliasing_<T const, U volatile>();
    test_10_aliasing_<T volatile, U volatile>();
    test_10_aliasing_<T const volatile, U volatile>();

    test_10_aliasing_<T, U const volatile>();
    test_10_aliasing_<T const, U const volatile>();
    test_10_aliasing_<T volatile, U const volatile>();
    test_10_aliasing_<T const volatile, U const volatile>();
}

template<class T, class U> void test_11_aliasing_()
{
    U u;
    mars_boost::local_shared_ptr<T> p1( new T() );

    test_aliasing_( p1, &u );
}

template<class T, class U> void test_11_aliasing()
{
    test_11_aliasing_<T, U>();
    test_11_aliasing_<T const, U>();
    test_11_aliasing_<T volatile, U>();
    test_11_aliasing_<T const volatile, U>();

    test_11_aliasing_<T, U volatile>();
    test_11_aliasing_<T const, U volatile>();
    test_11_aliasing_<T volatile, U volatile>();
    test_11_aliasing_<T const volatile, U volatile>();
}

static void aliasing_constructor()
{
    test_01_aliasing<int, int>();
    test_10_aliasing<int, int>();
    test_11_aliasing<int, int>();

    test_01_aliasing<void, int>();

    test_10_aliasing<int, void>();

    test_10_aliasing<int, incomplete>();

    test_01_aliasing<X, X>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing<X, X>();
    BOOST_TEST( X::instances == 0 );

    test_11_aliasing<X, X>();
    BOOST_TEST( X::instances == 0 );

    test_01_aliasing<int, X>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing<int, X>();
    BOOST_TEST( X::instances == 0 );

    test_11_aliasing<int, X>();
    BOOST_TEST( X::instances == 0 );

    test_01_aliasing<X, int>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing<X, int>();
    BOOST_TEST( X::instances == 0 );

    test_11_aliasing<X, int>();
    BOOST_TEST( X::instances == 0 );

    test_01_aliasing<void, X>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing<X, void>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing<X, incomplete>();
    BOOST_TEST( X::instances == 0 );
}

// shared_ptr copy constructor

template<class T> static void empty_shared_ptr_copy_test()
{
    mars_boost::shared_ptr<T> p1;

    mars_boost::local_shared_ptr<T> p2( p1 );

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( p1 );

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( p1 );

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( p3 );

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_shared_ptr_copy( mars_boost::shared_ptr<U> const & p1 )
{
    long k = p1.use_count();

    {
        mars_boost::local_shared_ptr<T> p2( p1 );

        BOOST_TEST( p2.get() == p1.get() );
        BOOST_TEST( p2.local_use_count() == 1 );

        BOOST_TEST( p1.use_count() == k + 1 );
    }

    BOOST_TEST( p1.use_count() == k );
}

template<class T> static void null_shared_ptr_copy_test()
{
    mars_boost::shared_ptr<T> p1( static_cast<T*>(0) );

    test_nonempty_shared_ptr_copy<T>( p1 );
    test_nonempty_shared_ptr_copy<T const>( p1 );
    test_nonempty_shared_ptr_copy<T volatile>( p1 );
    test_nonempty_shared_ptr_copy<T const volatile>( p1 );
    test_nonempty_shared_ptr_copy<void>( p1 );
    test_nonempty_shared_ptr_copy<void const>( p1 );
    test_nonempty_shared_ptr_copy<void volatile>( p1 );
    test_nonempty_shared_ptr_copy<void const volatile>( p1 );
}

template<class T> static void new_shared_ptr_copy_test()
{
    mars_boost::shared_ptr<T> p1( new T() );

    test_nonempty_shared_ptr_copy<T>( p1 );
    test_nonempty_shared_ptr_copy<T const>( p1 );
    test_nonempty_shared_ptr_copy<T volatile>( p1 );
    test_nonempty_shared_ptr_copy<T const volatile>( p1 );
    test_nonempty_shared_ptr_copy<void>( p1 );
    test_nonempty_shared_ptr_copy<void const>( p1 );
    test_nonempty_shared_ptr_copy<void volatile>( p1 );
    test_nonempty_shared_ptr_copy<void const volatile>( p1 );
}

static void shared_ptr_copy_constructor()
{
    empty_shared_ptr_copy_test<int>();
    empty_shared_ptr_copy_test<incomplete>();
    empty_shared_ptr_copy_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_shared_ptr_copy_test<int>();
    null_shared_ptr_copy_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_shared_ptr_copy_test<int>();
    new_shared_ptr_copy_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// shared_ptr_move constructor

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

template<class T> static void empty_shared_ptr_move_test()
{
    mars_boost::local_shared_ptr<T> p2(( mars_boost::shared_ptr<T>() ));

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3(( mars_boost::shared_ptr<T>() ));

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4(( mars_boost::shared_ptr<T>() ));

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( std::move(p3) );

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_shared_ptr_move( mars_boost::shared_ptr<U> && p1 )
{
    U* q = p1.get();

    mars_boost::local_shared_ptr<T> p2( std::move(p1) );

    BOOST_TEST( p2.get() == q );
    BOOST_TEST( p2.local_use_count() == 1 );

    BOOST_TEST( p1.get() == 0 );
    BOOST_TEST( p1.use_count() == 0 );
}

template<class T> static void null_shared_ptr_move_test()
{
    test_nonempty_shared_ptr_move<T>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<T const>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<T volatile>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<T const volatile>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<void>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<void const>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<void volatile>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move<void const volatile>( mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
}

template<class T> static void new_shared_ptr_move_test()
{
    test_nonempty_shared_ptr_move<T>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<T const>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<T volatile>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<T const volatile>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<void>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<void const>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<void volatile>( mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move<void const volatile>( mars_boost::shared_ptr<T>( new T() ) );
}

static void shared_ptr_move_constructor()
{
    empty_shared_ptr_move_test<int>();
    empty_shared_ptr_move_test<incomplete>();
    empty_shared_ptr_move_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_shared_ptr_move_test<int>();
    null_shared_ptr_move_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_shared_ptr_move_test<int>();
    new_shared_ptr_move_test<X>();

    BOOST_TEST( X::instances == 0 );
}

#else

static void shared_ptr_move_constructor()
{
}

#endif

// unique_ptr_constructor

#if !defined( BOOST_NO_CXX11_SMART_PTR ) && !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

template<class T, class U> static void test_null_unique_ptr( std::unique_ptr<U> && p1 )
{
    mars_boost::local_shared_ptr<T> p2( std::move( p1 ) );

    BOOST_TEST( p1.get() == 0 );

    BOOST_TEST( p2.get() == 0 );
    BOOST_TEST( p2.local_use_count() == 0 );
}

template<class T> static void null_unique_ptr_test()
{
    test_null_unique_ptr<T>( std::unique_ptr<T>() );

    test_null_unique_ptr<T const>( std::unique_ptr<T>() );
    test_null_unique_ptr<T volatile>( std::unique_ptr<T>() );
    test_null_unique_ptr<T const volatile>( std::unique_ptr<T>() );

    test_null_unique_ptr<T const>( std::unique_ptr<T const>() );
    test_null_unique_ptr<T volatile>( std::unique_ptr<T volatile>() );
    test_null_unique_ptr<T const volatile>( std::unique_ptr<T const volatile>() );

    test_null_unique_ptr<void>( std::unique_ptr<T>() );

    test_null_unique_ptr<void const>( std::unique_ptr<T>() );
    test_null_unique_ptr<void volatile>( std::unique_ptr<T>() );
    test_null_unique_ptr<void const volatile>( std::unique_ptr<T>() );

    test_null_unique_ptr<void const>( std::unique_ptr<T const>() );
    test_null_unique_ptr<void volatile>( std::unique_ptr<T volatile>() );
    test_null_unique_ptr<void const volatile>( std::unique_ptr<T const volatile>() );
}

template<class T, class U, class D> static void test_nonempty_unique_ptr( std::unique_ptr<U, D> && p1 )
{
    U * q = p1.get();

    mars_boost::local_shared_ptr<T> p2( std::move(p1) );

    BOOST_TEST( p2.get() == q );
    BOOST_TEST( p2.local_use_count() == 1 );

    BOOST_TEST( p1.get() == 0 );
}

template<class T> static void new_unique_ptr_test()
{
    test_nonempty_unique_ptr<T>( std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr<T const>( std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr<T volatile>( std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr<T const volatile>( std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr<T const>( std::unique_ptr<T const>( new T() ) );
    test_nonempty_unique_ptr<T volatile>( std::unique_ptr<T volatile>( new T() ) );
    test_nonempty_unique_ptr<T const volatile>( std::unique_ptr<T const volatile>( new T() ) );

    test_nonempty_unique_ptr<void>( std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr<void const>( std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr<void volatile>( std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr<void const volatile>( std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr<void const>( std::unique_ptr<T const>( new T() ) );
    test_nonempty_unique_ptr<void volatile>( std::unique_ptr<T volatile>( new T() ) );
    test_nonempty_unique_ptr<void const volatile>( std::unique_ptr<T const volatile>( new T() ) );
}

template<class T> static void test_deleter_unique_ptr()
{
    m = 0;

    test_nonempty_unique_ptr<T>( std::unique_ptr<int, void(*)(int*)>( &m, deleter2 ) );

    BOOST_TEST( m == 1 );
}

template<class T> static void deleter_unique_ptr_test()
{
    test_deleter_unique_ptr<T>();
    test_deleter_unique_ptr<T const>();
    test_deleter_unique_ptr<T volatile>();
    test_deleter_unique_ptr<T const volatile>();
}

static void unique_ptr_constructor()
{
    null_unique_ptr_test<int>();
    null_unique_ptr_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_unique_ptr_test<int>();
    new_unique_ptr_test<X>();

    BOOST_TEST( X::instances == 0 );

    deleter_unique_ptr_test<int>();
    deleter_unique_ptr_test<void>();
}

#else

static void unique_ptr_constructor()
{
}

#endif

// copy assignment

template<class T> static void empty_copy_assign_test()
{
    mars_boost::local_shared_ptr<T> p1;

    BOOST_TEST_EQ( p1.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p1.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T> p2;

    p2 = p1;

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3;

    p3 = p1;

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4;

    p4 = p1;

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5;

    p5 = p3;

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T> static void empty_copy_assign_test_()
{
    mars_boost::local_shared_ptr<T> p1;

    BOOST_TEST_EQ( p1.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p1.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T> p2( static_cast<T*>(0) );

    p2 = p1;

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( static_cast<T const*>(0) );

    p3 = p1;

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( static_cast<T*>(0) );

    p4 = p1;

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( static_cast<T const*>(0) );

    p5 = p3;

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_copy_assign( mars_boost::local_shared_ptr<T> p2, mars_boost::local_shared_ptr<U> const & p1 )
{
    long k = p1.local_use_count();

    p2 = p1;

    BOOST_TEST( p2.get() == p1.get() );
    BOOST_TEST( p2.local_use_count() == p1.local_use_count() );
    BOOST_TEST( p2.local_use_count() == k + 1 );

    p2.reset();

    BOOST_TEST( p1.local_use_count() == k );
}

template<class T> static void null_copy_assign_test()
{
    mars_boost::local_shared_ptr<T> p1( static_cast<T*>(0) );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T volatile>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const volatile>(), p1 );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), p1 );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void const>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void volatile>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void const volatile>(), p1 );
}

template<class T> static void new_copy_assign_test()
{
    mars_boost::local_shared_ptr<T> p1( new T() );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T volatile>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const volatile>(), p1 );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), p1 );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T>( new T() ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const>( new T const() ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ), p1 );

    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void const>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void volatile>(), p1 );
    test_nonempty_copy_assign( mars_boost::local_shared_ptr<void const volatile>(), p1 );
}

static void copy_assignment()
{
    empty_copy_assign_test<incomplete>();
    empty_copy_assign_test<int>();
    empty_copy_assign_test_<int>();
    empty_copy_assign_test<X>();
    empty_copy_assign_test_<X>();

    BOOST_TEST( X::instances == 0 );

    null_copy_assign_test<int>();
    null_copy_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_copy_assign_test<int>();
    new_copy_assign_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// move assignment

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

template<class T> static void empty_move_assign_test()
{
    mars_boost::local_shared_ptr<T> p2;

    p2 = mars_boost::local_shared_ptr<T>();

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3;

    p3 = mars_boost::local_shared_ptr<T>();

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4;

    p4 = mars_boost::local_shared_ptr<T>();

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5;

    p5 = std::move( p3 );

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T> static void empty_move_assign_test_()
{
    mars_boost::local_shared_ptr<T> p2( static_cast<T*>(0) );

    p2 = mars_boost::local_shared_ptr<T>();

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( static_cast<T const*>(0) );

    p3 = mars_boost::local_shared_ptr<T>();

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( static_cast<T*>(0) );

    p4 = mars_boost::local_shared_ptr<T>();

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( static_cast<T const*>(0) );

    p5 = std::move( p3 );

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_move_assign( mars_boost::local_shared_ptr<T> p2, mars_boost::local_shared_ptr<U> && p1 )
{
    U* q = p1.get();
    long k = p1.local_use_count();

    p2 = std::move( p1 );

    BOOST_TEST( p2.get() == q );
    BOOST_TEST( p2.local_use_count() == k );

    BOOST_TEST( p1.get() == 0 );
    BOOST_TEST( p1.local_use_count() == 0 );
}

template<class T> static void null_move_assign_test()
{
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T volatile>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const volatile>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );

    test_nonempty_move_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );

    test_nonempty_move_assign( mars_boost::local_shared_ptr<void>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<void const>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<void volatile>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<void const volatile>(), mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
}

template<class T> static void new_move_assign_test()
{
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T>(), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const>(), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T volatile>(), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const volatile>(), mars_boost::local_shared_ptr<T>( new T() ) );

    test_nonempty_move_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), mars_boost::local_shared_ptr<T>( new T() ) );

    test_nonempty_move_assign( mars_boost::local_shared_ptr<T>( new T() ), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const>( new T const() ), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ), mars_boost::local_shared_ptr<T>( new T() ) );

    test_nonempty_move_assign( mars_boost::local_shared_ptr<void>(), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<void const>(), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<void volatile>(), mars_boost::local_shared_ptr<T>( new T() ) );
    test_nonempty_move_assign( mars_boost::local_shared_ptr<void const volatile>(), mars_boost::local_shared_ptr<T>( new T() ) );
}

static void move_assignment()
{
    empty_move_assign_test<incomplete>();
    empty_move_assign_test<int>();
    empty_move_assign_test_<int>();
    empty_move_assign_test<X>();
    empty_move_assign_test_<X>();

    BOOST_TEST( X::instances == 0 );

    null_move_assign_test<int>();
    null_move_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_move_assign_test<int>();
    new_move_assign_test<X>();

    BOOST_TEST( X::instances == 0 );
}

#else

static void move_assignment()
{
}

#endif

// nullptr assignment

#if !defined( BOOST_NO_CXX11_NULLPTR )

template<class T> static void test_nullptr_assign( mars_boost::local_shared_ptr<T> p1 )
{
    p1 = nullptr;

    BOOST_TEST( p1.get() == 0 );
    BOOST_TEST( p1.local_use_count() == 0 );
}

template<class T> static void empty_nullptr_assign_test()
{
    test_nullptr_assign( mars_boost::local_shared_ptr<T>() );
    test_nullptr_assign( mars_boost::local_shared_ptr<T const>() );
    test_nullptr_assign( mars_boost::local_shared_ptr<T volatile>() );
    test_nullptr_assign( mars_boost::local_shared_ptr<T const volatile>() );
}

template<class T> static void null_nullptr_assign_test()
{
    test_nullptr_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_nullptr_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ) );
    test_nullptr_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ) );
    test_nullptr_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ) );
}

template<class T> static void new_nullptr_assign_test()
{
    test_nullptr_assign( mars_boost::local_shared_ptr<T>( new T() ) );
    test_nullptr_assign( mars_boost::local_shared_ptr<T const>( new T const() ) );
    test_nullptr_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ) );
    test_nullptr_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ) );
}

static void nullptr_assignment()
{
    empty_nullptr_assign_test<incomplete>();
    empty_nullptr_assign_test<void>();
    empty_nullptr_assign_test<int>();
    empty_nullptr_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_nullptr_assign_test<int>();
    null_nullptr_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_nullptr_assign_test<int>();
    new_nullptr_assign_test<X>();

    BOOST_TEST( X::instances == 0 );
}

#else

static void nullptr_assignment()
{
}

#endif

// default_reset

template<class T> static void test_default_reset( mars_boost::local_shared_ptr<T> p1 )
{
    p1.reset();

    BOOST_TEST( p1.get() == 0 );
    BOOST_TEST( p1.local_use_count() == 0 );
}

template<class T> static void empty_default_reset_test()
{
    test_default_reset( mars_boost::local_shared_ptr<T>() );
    test_default_reset( mars_boost::local_shared_ptr<T const>() );
    test_default_reset( mars_boost::local_shared_ptr<T volatile>() );
    test_default_reset( mars_boost::local_shared_ptr<T const volatile>() );
}

template<class T> static void null_default_reset_test()
{
    test_default_reset( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_default_reset( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ) );
    test_default_reset( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ) );
    test_default_reset( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ) );
}

template<class T> static void new_default_reset_test()
{
    test_default_reset( mars_boost::local_shared_ptr<T>( new T() ) );
    test_default_reset( mars_boost::local_shared_ptr<T const>( new T const() ) );
    test_default_reset( mars_boost::local_shared_ptr<T volatile>( new T volatile() ) );
    test_default_reset( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ) );
}

static void default_reset()
{
    empty_default_reset_test<incomplete>();
    empty_default_reset_test<void>();
    empty_default_reset_test<int>();
    empty_default_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_default_reset_test<int>();
    null_default_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_default_reset_test<int>();
    new_default_reset_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// shared_ptr copy assignment

template<class T> static void empty_shared_ptr_copy_assign_test()
{
    mars_boost::shared_ptr<T> sp1;

    BOOST_TEST_EQ( sp1.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( sp1.use_count(), 0 );

    mars_boost::local_shared_ptr<T> p2;

    p2 = sp1;

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3;

    p3 = sp1;

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4;

    p4 = sp1;

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::shared_ptr<T const> sp2( sp1 );
    mars_boost::local_shared_ptr<void const> p5;

    p5 = sp2;

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T> static void empty_shared_ptr_copy_assign_test_()
{
    mars_boost::shared_ptr<T> sp1;

    BOOST_TEST_EQ( sp1.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( sp1.use_count(), 0 );

    mars_boost::local_shared_ptr<T> p2( static_cast<T*>(0) );

    p2 = sp1;

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( static_cast<T const*>(0) );

    p3 = sp1;

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( static_cast<T*>(0) );

    p4 = sp1;

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::shared_ptr<T const> sp2( sp1 );
    mars_boost::local_shared_ptr<void const> p5( static_cast<T const*>(0) );

    p5 = sp2;

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T> p2, mars_boost::shared_ptr<U> const & p1 )
{
    long k = p1.use_count();

    p2 = p1;

    BOOST_TEST( p2.get() == p1.get() );
    BOOST_TEST( p2.local_use_count() == 1 );
    BOOST_TEST( p1.use_count() == k + 1 );

    p2.reset();

    BOOST_TEST( p1.use_count() == k );
}

template<class T> static void null_shared_ptr_copy_assign_test()
{
    mars_boost::shared_ptr<T> p1( static_cast<T*>(0) );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T volatile>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const volatile>(), p1 );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), p1 );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void const>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void volatile>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void const volatile>(), p1 );
}

template<class T> static void new_shared_ptr_copy_assign_test()
{
    mars_boost::shared_ptr<T> p1( new T() );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T volatile>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const volatile>(), p1 );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), p1 );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T>( new T() ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const>( new T const() ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ), p1 );

    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void const>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void volatile>(), p1 );
    test_nonempty_shared_ptr_copy_assign( mars_boost::local_shared_ptr<void const volatile>(), p1 );
}

static void shared_ptr_copy_assignment()
{
    empty_shared_ptr_copy_assign_test<incomplete>();
    empty_shared_ptr_copy_assign_test<int>();
    empty_shared_ptr_copy_assign_test_<int>();
    empty_shared_ptr_copy_assign_test<X>();
    empty_shared_ptr_copy_assign_test_<X>();

    BOOST_TEST( X::instances == 0 );

    null_shared_ptr_copy_assign_test<int>();
    null_shared_ptr_copy_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_shared_ptr_copy_assign_test<int>();
    new_shared_ptr_copy_assign_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// shared_ptr_move assignment

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

template<class T> static void empty_shared_ptr_move_assign_test()
{
    mars_boost::local_shared_ptr<T> p2;

    p2 = mars_boost::shared_ptr<T>();

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3;

    p3 = mars_boost::shared_ptr<T>();

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4;

    p4 = mars_boost::shared_ptr<T>();

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5;

    p5 = mars_boost::shared_ptr<T const>();

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T> static void empty_shared_ptr_move_assign_test_()
{
    mars_boost::local_shared_ptr<T> p2( static_cast<T*>(0) );

    p2 = mars_boost::shared_ptr<T>();

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( static_cast<T const*>(0) );

    p3 = mars_boost::shared_ptr<T>();

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( static_cast<T*>(0) );

    p4 = mars_boost::shared_ptr<T>();

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( static_cast<T const*>(0) );

    p5 = mars_boost::shared_ptr<T const>();

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U> static void test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T> p2, mars_boost::shared_ptr<U> && p1 )
{
    U* q = p1.get();
    long k = p1.use_count();

    p2 = std::move( p1 );

    BOOST_TEST_EQ( p2.get(), q );
    BOOST_TEST_EQ( p2.local_use_count(), 1 );

    BOOST_TEST( p1.get() == 0 );
    BOOST_TEST( p1.use_count() == 0 );

    mars_boost::shared_ptr<T> p3( p2 );

    BOOST_TEST_EQ( p3.get(), q );
    BOOST_TEST_EQ( p3.use_count(), k + 1 );
}

template<class T> static void null_shared_ptr_move_assign_test()
{
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T volatile>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const volatile>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );

    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );

    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void const>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void volatile>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void const volatile>(), mars_boost::shared_ptr<T>( static_cast<T*>(0) ) );
}

template<class T> static void new_shared_ptr_move_assign_test()
{
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T>(), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const>(), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T volatile>(), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const volatile>(), mars_boost::shared_ptr<T>( new T() ) );

    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), mars_boost::shared_ptr<T>( new T() ) );

    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T>( new T() ), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const>( new T const() ), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ), mars_boost::shared_ptr<T>( new T() ) );

    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void>(), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void const>(), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void volatile>(), mars_boost::shared_ptr<T>( new T() ) );
    test_nonempty_shared_ptr_move_assign( mars_boost::local_shared_ptr<void const volatile>(), mars_boost::shared_ptr<T>( new T() ) );
}

static void shared_ptr_move_assignment()
{
    empty_shared_ptr_move_assign_test<incomplete>();
    empty_shared_ptr_move_assign_test<int>();
    empty_shared_ptr_move_assign_test_<int>();
    empty_shared_ptr_move_assign_test<X>();
    empty_shared_ptr_move_assign_test_<X>();

    BOOST_TEST( X::instances == 0 );

    null_shared_ptr_move_assign_test<int>();
    null_shared_ptr_move_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_shared_ptr_move_assign_test<int>();
    new_shared_ptr_move_assign_test<X>();

    BOOST_TEST( X::instances == 0 );
}

#else

static void shared_ptr_move_assignment()
{
}

#endif

// unique_ptr assignment

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES ) && !defined( BOOST_NO_CXX11_SMART_PTR )

template<class T> static void empty_unique_ptr_assign_test()
{
    mars_boost::local_shared_ptr<T> p2;

    p2 = std::unique_ptr<T>();

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3;

    p3 = std::unique_ptr<T>();

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4;

    p4 = std::unique_ptr<T>();

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5;

    p5 = std::unique_ptr<T const>();

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T> static void empty_unique_ptr_assign_test_()
{
    mars_boost::local_shared_ptr<T> p2( static_cast<T*>(0) );

    p2 = std::unique_ptr<T>();

    BOOST_TEST_EQ( p2.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p2.local_use_count(), 0 );

    mars_boost::local_shared_ptr<T const> p3( static_cast<T const*>(0) );

    p3 = std::unique_ptr<T>();

    BOOST_TEST_EQ( p3.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p3.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void> p4( static_cast<T*>(0) );

    p4 = std::unique_ptr<T>();

    BOOST_TEST_EQ( p4.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p4.local_use_count(), 0 );

    mars_boost::local_shared_ptr<void const> p5( static_cast<T const*>(0) );

    p5 = std::unique_ptr<T const>();

    BOOST_TEST_EQ( p5.get(), static_cast<void*>(0) );
    BOOST_TEST_EQ( p5.local_use_count(), 0 );
}

template<class T, class U, class D> static void test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T> p2, std::unique_ptr<U, D> && p1 )
{
    U* q = p1.get();

    p2 = std::move( p1 );

    BOOST_TEST_EQ( p2.get(), q );
    BOOST_TEST_EQ( p2.local_use_count(), 1 );

    BOOST_TEST( p1.get() == 0 );
}

template<class T> static void new_unique_ptr_assign_test()
{
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T>(), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const>(), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T volatile>(), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const volatile>(), std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T>( new T() ), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const>( new T const() ), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ), std::unique_ptr<T>( new T() ) );

    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void>(), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void const>(), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void volatile>(), std::unique_ptr<T>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void const volatile>(), std::unique_ptr<T>( new T() ) );
}

template<class T> static void del_unique_ptr_assign_test()
{
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T volatile>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const volatile>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );

    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const>( static_cast<T const*>(0) ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T volatile>( static_cast<T volatile*>(0) ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const volatile>( static_cast<T const volatile*>(0) ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );

    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T>( new T() ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const>( new T const() ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T volatile>( new T volatile() ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<T const volatile>( new T const volatile() ), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );

    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void const>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void volatile>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
    test_nonempty_unique_ptr_assign( mars_boost::local_shared_ptr<void const volatile>(), std::unique_ptr<T, mars_boost::checked_deleter<T>>( new T() ) );
}

static void unique_ptr_assignment()
{
    empty_unique_ptr_assign_test<int>();
    empty_unique_ptr_assign_test_<int>();
    empty_unique_ptr_assign_test<X>();
    empty_unique_ptr_assign_test_<X>();

    BOOST_TEST( X::instances == 0 );

    new_unique_ptr_assign_test<int>();
    new_unique_ptr_assign_test<X>();

    BOOST_TEST( X::instances == 0 );

    del_unique_ptr_assign_test<int>();
    del_unique_ptr_assign_test<X>();

    BOOST_TEST( X::instances == 0 );
}

#else

static void unique_ptr_assignment()
{
}

#endif

// pointer reset

template<class T, class U> static void test_pointer_reset( mars_boost::local_shared_ptr<U> p2 )
{
    T * q = new T();

    p2.reset( q );

    BOOST_TEST_EQ( p2.get(), q );
    BOOST_TEST_EQ( p2.local_use_count(), 1 );
}

template<class T> static void empty_pointer_reset_test()
{
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T>() );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T const>() );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T volatile>() );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T const volatile>() );

    test_pointer_reset<T>( mars_boost::local_shared_ptr<void>() );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void const>() );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void volatile>() );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void const volatile>() );
}

template<class T> static void null_pointer_reset_test()
{
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T const>( static_cast<T*>(0) ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T volatile>( static_cast<T*>(0) ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T const volatile>( static_cast<T*>(0) ) );

    test_pointer_reset<T>( mars_boost::local_shared_ptr<void>( static_cast<T*>(0) ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void const>( static_cast<T*>(0) ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void volatile>( static_cast<T*>(0) ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void const volatile>( static_cast<T*>(0) ) );
}

template<class T> static void new_pointer_reset_test()
{
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T const>( new T() ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T volatile>( new T() ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<T const volatile>( new T() ) );

    test_pointer_reset<T>( mars_boost::local_shared_ptr<void>( new T() ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void const>( new T() ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void volatile>( new T() ) );
    test_pointer_reset<T>( mars_boost::local_shared_ptr<void const volatile>( new T() ) );
}

static void pointer_reset()
{
    empty_pointer_reset_test<int>();
    empty_pointer_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_pointer_reset_test<int>();
    null_pointer_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_pointer_reset_test<int>();
    new_pointer_reset_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// deleter reset

template<class T> class deleter
{
private:

    bool * called_;

public:

    explicit deleter( bool * called ): called_( called ) {}
    void operator()( T * p ) { *called_ = true; delete p; }
};

template<class T, class U> static void test_deleter_reset( mars_boost::local_shared_ptr<U> p2 )
{
    T * q = new T();

    bool called = false;

    p2.reset( q, deleter<T>( &called ) );

    BOOST_TEST_EQ( p2.get(), q );
    BOOST_TEST_EQ( p2.local_use_count(), 1 );

    mars_boost::shared_ptr<U> p3( p2 );

    BOOST_TEST( mars_boost::get_deleter< deleter<T> >( p3 ) != 0 );

    p3.reset();
    BOOST_TEST( !called );

    p2.reset();
    BOOST_TEST( called );
}

template<class T> static void empty_deleter_reset_test()
{
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T>() );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T const>() );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T volatile>() );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T const volatile>() );

    test_deleter_reset<T>( mars_boost::local_shared_ptr<void>() );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void const>() );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void volatile>() );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void const volatile>() );
}

template<class T> static void null_deleter_reset_test()
{
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T const>( static_cast<T*>(0) ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T volatile>( static_cast<T*>(0) ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T const volatile>( static_cast<T*>(0) ) );

    test_deleter_reset<T>( mars_boost::local_shared_ptr<void>( static_cast<T*>(0) ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void const>( static_cast<T*>(0) ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void volatile>( static_cast<T*>(0) ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void const volatile>( static_cast<T*>(0) ) );
}

template<class T> static void new_deleter_reset_test()
{
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T const>( new T() ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T volatile>( new T() ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<T const volatile>( new T() ) );

    test_deleter_reset<T>( mars_boost::local_shared_ptr<void>( new T() ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void const>( new T() ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void volatile>( new T() ) );
    test_deleter_reset<T>( mars_boost::local_shared_ptr<void const volatile>( new T() ) );
}

static void deleter_reset()
{
    empty_deleter_reset_test<int>();
    empty_deleter_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_deleter_reset_test<int>();
    null_deleter_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_deleter_reset_test<int>();
    new_deleter_reset_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// allocator reset

template<class T, class U> static void test_allocator_reset( mars_boost::local_shared_ptr<U> p2 )
{
    T * q = new T();

    bool called = false;

    p2.reset( q, deleter<T>( &called ), std::allocator<T>() );

    BOOST_TEST_EQ( p2.get(), q );
    BOOST_TEST_EQ( p2.local_use_count(), 1 );

    mars_boost::shared_ptr<U> p3( p2 );

    BOOST_TEST( mars_boost::get_deleter< deleter<T> >( p3 ) != 0 );

    p3.reset();
    BOOST_TEST( !called );

    p2.reset();
    BOOST_TEST( called );
}

template<class T> static void empty_allocator_reset_test()
{
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T>() );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T const>() );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T volatile>() );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T const volatile>() );

    test_allocator_reset<T>( mars_boost::local_shared_ptr<void>() );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void const>() );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void volatile>() );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void const volatile>() );
}

template<class T> static void null_allocator_reset_test()
{
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T>( static_cast<T*>(0) ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T const>( static_cast<T*>(0) ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T volatile>( static_cast<T*>(0) ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T const volatile>( static_cast<T*>(0) ) );

    test_allocator_reset<T>( mars_boost::local_shared_ptr<void>( static_cast<T*>(0) ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void const>( static_cast<T*>(0) ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void volatile>( static_cast<T*>(0) ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void const volatile>( static_cast<T*>(0) ) );
}

template<class T> static void new_allocator_reset_test()
{
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T>( new T() ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T const>( new T() ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T volatile>( new T() ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<T const volatile>( new T() ) );

    test_allocator_reset<T>( mars_boost::local_shared_ptr<void>( new T() ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void const>( new T() ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void volatile>( new T() ) );
    test_allocator_reset<T>( mars_boost::local_shared_ptr<void const volatile>( new T() ) );
}

static void allocator_reset()
{
    empty_allocator_reset_test<int>();
    empty_allocator_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    null_allocator_reset_test<int>();
    null_allocator_reset_test<X>();

    BOOST_TEST( X::instances == 0 );

    new_allocator_reset_test<int>();
    new_allocator_reset_test<X>();

    BOOST_TEST( X::instances == 0 );
}

// aliasing reset

struct null_deleter
{
    void operator()( void const volatile* ) {}
};

template<class T, class U> void test_aliasing_reset_( mars_boost::local_shared_ptr<T> const & p1, U * p2 )
{
    mars_boost::local_shared_ptr<U> p3( static_cast<U*>(0), null_deleter() );

    p3.reset( p1, p2 );

    BOOST_TEST( p3.get() == p2 );
    BOOST_TEST( p3.local_use_count() == p1.local_use_count() );
    BOOST_TEST( !p3.owner_before( p1 ) && !p1.owner_before( p3 ) );
}

template<class T, class U> void test_01_aliasing_reset_()
{
    U u;
    mars_boost::local_shared_ptr<T> p1;

    test_aliasing_reset_( p1, &u );
}

template<class T, class U> void test_01_aliasing_reset()
{
    test_01_aliasing_reset_<T, U>();
    test_01_aliasing_reset_<T const, U>();
    test_01_aliasing_reset_<T volatile, U>();
    test_01_aliasing_reset_<T const volatile, U>();

    test_01_aliasing_reset_<T, U volatile>();
    test_01_aliasing_reset_<T const, U volatile>();
    test_01_aliasing_reset_<T volatile, U volatile>();
    test_01_aliasing_reset_<T const volatile, U volatile>();
}

template<class T, class U> void test_10_aliasing_reset_()
{
    mars_boost::local_shared_ptr<T> p1( new T() );
    test_aliasing_reset_( p1, static_cast<U*>(0) );
}

template<class T, class U> void test_10_aliasing_reset()
{
    test_10_aliasing_reset_<T, U>();
    test_10_aliasing_reset_<T const, U>();
    test_10_aliasing_reset_<T volatile, U>();
    test_10_aliasing_reset_<T const volatile, U>();

    test_10_aliasing_reset_<T, U const>();
    test_10_aliasing_reset_<T const, U const>();
    test_10_aliasing_reset_<T volatile, U const>();
    test_10_aliasing_reset_<T const volatile, U const>();

    test_10_aliasing_reset_<T, U volatile>();
    test_10_aliasing_reset_<T const, U volatile>();
    test_10_aliasing_reset_<T volatile, U volatile>();
    test_10_aliasing_reset_<T const volatile, U volatile>();

    test_10_aliasing_reset_<T, U const volatile>();
    test_10_aliasing_reset_<T const, U const volatile>();
    test_10_aliasing_reset_<T volatile, U const volatile>();
    test_10_aliasing_reset_<T const volatile, U const volatile>();
}

template<class T, class U> void test_11_aliasing_reset_()
{
    U u;
    mars_boost::local_shared_ptr<T> p1( new T() );

    test_aliasing_reset_( p1, &u );
}

template<class T, class U> void test_11_aliasing_reset()
{
    test_11_aliasing_reset_<T, U>();
    test_11_aliasing_reset_<T const, U>();
    test_11_aliasing_reset_<T volatile, U>();
    test_11_aliasing_reset_<T const volatile, U>();

    test_11_aliasing_reset_<T, U volatile>();
    test_11_aliasing_reset_<T const, U volatile>();
    test_11_aliasing_reset_<T volatile, U volatile>();
    test_11_aliasing_reset_<T const volatile, U volatile>();
}

static void aliasing_reset()
{
    test_01_aliasing_reset<int, int>();
    test_10_aliasing_reset<int, int>();
    test_11_aliasing_reset<int, int>();

    test_01_aliasing_reset<void, int>();

    test_10_aliasing_reset<int, void>();

    test_10_aliasing_reset<int, incomplete>();

    test_01_aliasing_reset<X, X>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing_reset<X, X>();
    BOOST_TEST( X::instances == 0 );

    test_11_aliasing_reset<X, X>();
    BOOST_TEST( X::instances == 0 );

    test_01_aliasing_reset<int, X>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing_reset<int, X>();
    BOOST_TEST( X::instances == 0 );

    test_11_aliasing_reset<int, X>();
    BOOST_TEST( X::instances == 0 );

    test_01_aliasing_reset<X, int>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing_reset<X, int>();
    BOOST_TEST( X::instances == 0 );

    test_11_aliasing_reset<X, int>();
    BOOST_TEST( X::instances == 0 );

    test_01_aliasing_reset<void, X>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing_reset<X, void>();
    BOOST_TEST( X::instances == 0 );

    test_10_aliasing_reset<X, incomplete>();
    BOOST_TEST( X::instances == 0 );
}

// element access

template<class T> static void empty_element_access_()
{
    mars_boost::local_shared_ptr<T> p1;

    BOOST_TEST_EQ( p1.operator->(), static_cast<T*>(0) );
    BOOST_TEST_EQ( p1.get(), static_cast<T*>(0) );
    BOOST_TEST( p1? false: true );
    BOOST_TEST( !p1 );
    BOOST_TEST_EQ( p1.local_use_count(), 0 );
}

template<class T> static void empty_element_access()
{
    empty_element_access_<T>();
    empty_element_access_<T const>();
    empty_element_access_<T volatile>();
    empty_element_access_<T const volatile>();
}

template<class T> static void new_element_access_()
{
    {
        T * p0 = new T();
        mars_boost::local_shared_ptr<T> p1( p0 );

        BOOST_TEST_EQ( p1.operator->(), p0 );
        BOOST_TEST_EQ( p1.get(), p0 );
        BOOST_TEST_EQ( &*p1, p0 );
        BOOST_TEST( p1? true: false );
        BOOST_TEST_NOT( !p1 );
        BOOST_TEST_EQ( p1.local_use_count(), 1 );
    }

    {
        T * p0 = new T[3]();
        mars_boost::local_shared_ptr<T[]> p1( p0 );

        BOOST_TEST_EQ( p1.get(), p0 );

        BOOST_TEST_EQ( &p1[0], &p0[0] );
        BOOST_TEST_EQ( &p1[1], &p0[1] );
        BOOST_TEST_EQ( &p1[2], &p0[2] );

        BOOST_TEST( p1? true: false );
        BOOST_TEST_NOT( !p1 );
        BOOST_TEST_EQ( p1.local_use_count(), 1 );
    }

    {
        T * p0 = new T[3]();
        mars_boost::local_shared_ptr<T[3]> p1( p0 );

        BOOST_TEST_EQ( p1.get(), p0 );

        BOOST_TEST_EQ( &p1[0], &p0[0] );
        BOOST_TEST_EQ( &p1[1], &p0[1] );
        BOOST_TEST_EQ( &p1[2], &p0[2] );

        BOOST_TEST( p1? true: false );
        BOOST_TEST_NOT( !p1 );
        BOOST_TEST_EQ( p1.local_use_count(), 1 );
    }
}

template<class T> static void new_element_access()
{
    new_element_access_<T>();
    new_element_access_<T const>();
    new_element_access_<T volatile>();
    new_element_access_<T const volatile>();
}

static void element_access()
{
    empty_element_access<int>();
    empty_element_access<X>();

    BOOST_TEST( X::instances == 0 );

    empty_element_access<incomplete>();
    empty_element_access<void>();

    new_element_access<int>();
    new_element_access<X>();

    BOOST_TEST( X::instances == 0 );
}

// shared_ptr conversion

template<class T, class U> static void empty_shared_ptr_conversion_()
{
    mars_boost::local_shared_ptr<T> p1;
    mars_boost::shared_ptr<U> p2( p1 );

    BOOST_TEST_EQ( p2.get(), static_cast<U*>(0) );
    BOOST_TEST_EQ( p2.use_count(), 0 );
}

template<class T> static void empty_shared_ptr_conversion()
{
    empty_shared_ptr_conversion_<T, T>();
    empty_shared_ptr_conversion_<T, T const>();
    empty_shared_ptr_conversion_<T, T volatile>();
    empty_shared_ptr_conversion_<T, T const volatile>();

    empty_shared_ptr_conversion_<T const, T const>();
    empty_shared_ptr_conversion_<T volatile, T volatile>();
    empty_shared_ptr_conversion_<T const volatile, T const volatile>();

    empty_shared_ptr_conversion_<T, void>();
    empty_shared_ptr_conversion_<T, void const>();
    empty_shared_ptr_conversion_<T, void volatile>();
    empty_shared_ptr_conversion_<T, void const volatile>();

    empty_shared_ptr_conversion_<T const, void const>();
    empty_shared_ptr_conversion_<T volatile, void volatile>();
    empty_shared_ptr_conversion_<T const volatile, void const volatile>();
}

template<class T, class U> static void new_shared_ptr_conversion_()
{
    mars_boost::local_shared_ptr<T> p1( new T() );
    mars_boost::shared_ptr<U> p2( p1 );

    BOOST_TEST_EQ( p2.get(), p1.get() );
    BOOST_TEST_EQ( p2.use_count(), 2 );

    mars_boost::shared_ptr<U> p3( p1 );

    BOOST_TEST_EQ( p3.get(), p1.get() );
    BOOST_TEST_EQ( p3.use_count(), 3 );
    BOOST_TEST( !(p2 < p3) && !(p3 < p2) );

    BOOST_TEST_EQ( p1.local_use_count(), 1 );

    p1.reset();

    BOOST_TEST_EQ( p2.use_count(), 2 );
    BOOST_TEST_EQ( p3.use_count(), 2 );
}

template<class T> static void new_shared_ptr_conversion()
{
    new_shared_ptr_conversion_<T, T>();
    new_shared_ptr_conversion_<T, T const>();
    new_shared_ptr_conversion_<T, T volatile>();
    new_shared_ptr_conversion_<T, T const volatile>();

    new_shared_ptr_conversion_<T const, T const>();
    new_shared_ptr_conversion_<T volatile, T volatile>();
    new_shared_ptr_conversion_<T const volatile, T const volatile>();

    new_shared_ptr_conversion_<T, void>();
    new_shared_ptr_conversion_<T, void const>();
    new_shared_ptr_conversion_<T, void volatile>();
    new_shared_ptr_conversion_<T, void const volatile>();

    new_shared_ptr_conversion_<T const, void const>();
    new_shared_ptr_conversion_<T volatile, void volatile>();
    new_shared_ptr_conversion_<T const volatile, void const volatile>();
}

static void shared_ptr_conversion()
{
    empty_shared_ptr_conversion<void>();
    empty_shared_ptr_conversion<incomplete>();
    empty_shared_ptr_conversion<int>();
    empty_shared_ptr_conversion<X>();

    BOOST_TEST( X::instances == 0 );

    new_shared_ptr_conversion<int>();
    new_shared_ptr_conversion<X>();

    BOOST_TEST( X::instances == 0 );
}

// weak_ptr conversion

template<class T, class U> static void empty_weak_ptr_conversion_()
{
    mars_boost::local_shared_ptr<T> p1;
    mars_boost::weak_ptr<U> p2( p1 );

    BOOST_TEST_EQ( p2.lock().get(), static_cast<U*>(0) );
    BOOST_TEST_EQ( p2.use_count(), 0 );
}

template<class T> static void empty_weak_ptr_conversion()
{
    empty_weak_ptr_conversion_<T, T>();
    empty_weak_ptr_conversion_<T, T const>();
    empty_weak_ptr_conversion_<T, T volatile>();
    empty_weak_ptr_conversion_<T, T const volatile>();

    empty_weak_ptr_conversion_<T const, T const>();
    empty_weak_ptr_conversion_<T volatile, T volatile>();
    empty_weak_ptr_conversion_<T const volatile, T const volatile>();

    empty_weak_ptr_conversion_<T, void>();
    empty_weak_ptr_conversion_<T, void const>();
    empty_weak_ptr_conversion_<T, void volatile>();
    empty_weak_ptr_conversion_<T, void const volatile>();

    empty_weak_ptr_conversion_<T const, void const>();
    empty_weak_ptr_conversion_<T volatile, void volatile>();
    empty_weak_ptr_conversion_<T const volatile, void const volatile>();
}

template<class T, class U> static void new_weak_ptr_conversion_()
{
    mars_boost::local_shared_ptr<T> p1( new T() );
    mars_boost::weak_ptr<U> p2( p1 );

    BOOST_TEST_EQ( p2.lock().get(), p1.get() );
    BOOST_TEST_EQ( p2.use_count(), 1 );

    mars_boost::weak_ptr<U> p3( p1 );

    BOOST_TEST_EQ( p3.lock().get(), p1.get() );
    BOOST_TEST_EQ( p3.use_count(), 1 );
    BOOST_TEST( !(p2 < p3) && !(p3 < p2) );

    BOOST_TEST_EQ( p1.local_use_count(), 1 );

    p1.reset();

    BOOST_TEST_EQ( p2.use_count(), 0 );
    BOOST_TEST_EQ( p3.use_count(), 0 );
}

template<class T> static void new_weak_ptr_conversion()
{
    new_weak_ptr_conversion_<T, T>();
    new_weak_ptr_conversion_<T, T const>();
    new_weak_ptr_conversion_<T, T volatile>();
    new_weak_ptr_conversion_<T, T const volatile>();

    new_weak_ptr_conversion_<T const, T const>();
    new_weak_ptr_conversion_<T volatile, T volatile>();
    new_weak_ptr_conversion_<T const volatile, T const volatile>();

    new_weak_ptr_conversion_<T, void>();
    new_weak_ptr_conversion_<T, void const>();
    new_weak_ptr_conversion_<T, void volatile>();
    new_weak_ptr_conversion_<T, void const volatile>();

    new_weak_ptr_conversion_<T const, void const>();
    new_weak_ptr_conversion_<T volatile, void volatile>();
    new_weak_ptr_conversion_<T const volatile, void const volatile>();
}

static void weak_ptr_conversion()
{
    empty_weak_ptr_conversion<void>();
    empty_weak_ptr_conversion<incomplete>();
    empty_weak_ptr_conversion<int>();
    empty_weak_ptr_conversion<X>();

    BOOST_TEST( X::instances == 0 );

    new_weak_ptr_conversion<int>();
    new_weak_ptr_conversion<X>();

    BOOST_TEST( X::instances == 0 );
}

// main

int main()
{
    default_constructor();
    nullptr_constructor();
    pointer_constructor();
    deleter_constructor();
    copy_constructor();
    move_constructor();
    aliasing_constructor();
    nullptr_deleter_constructor();
    allocator_constructor();
    nullptr_allocator_constructor();
    shared_ptr_copy_constructor();
    shared_ptr_move_constructor();
    unique_ptr_constructor();

    copy_assignment();
    move_assignment();
    nullptr_assignment();
    shared_ptr_copy_assignment();
    shared_ptr_move_assignment();
    unique_ptr_assignment();

    default_reset();
    pointer_reset();
    deleter_reset();
    allocator_reset();
    aliasing_reset();

    element_access();
    shared_ptr_conversion();
    weak_ptr_conversion();
    // swap_test();
    // owner_before_test();
    // equal_test();
    // operator< ?
    // casts
    // get_pointer
    // operator<<
    // hash

    return mars_boost::report_errors();
}
