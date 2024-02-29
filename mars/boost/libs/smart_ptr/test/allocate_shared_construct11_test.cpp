// allocate_shared_construct11_test.cpp
//
// Test whether allocate_shared uses construct/destroy in C++11
//
// Copyright 2007-2009, 2014 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <cstddef>

#if !defined( BOOST_NO_CXX11_ALLOCATOR )

template< class T > class cxx11_allocator
{
public:

    typedef T value_type;

    cxx11_allocator()
    {
    }

    template< class Y > cxx11_allocator( cxx11_allocator<Y> const & )
    {
    }

    T * allocate( std::size_t n )
    {
        return static_cast< T* >( ::operator new( n * sizeof( T ) ) );
    }

    void deallocate( T * p, std::size_t n )
    {
        ::operator delete( p );
    }

    template<class... Args> void construct( T * p, Args&&... args )
    {
        ::new( static_cast< void* >( p ) ) T( std::forward<Args>( args )... );
    }

    void destroy( T * p )
    {
        p->~T();
    }
};

class X
{
private:

    X( X const & );
    X & operator=( X const & );

    void * operator new( std::size_t n )
    {
        BOOST_ERROR( "private X::new called" );
        return ::operator new( n );
    }

    void operator delete( void * p )
    {
        BOOST_ERROR( "private X::delete called" );
        ::operator delete( p );
    }

public:

    static int instances;

    int v;

protected:

    explicit X( int a1 = 0, int a2 = 0, int a3 = 0, int a4 = 0, int a5 = 0, int a6 = 0, int a7 = 0, int a8 = 0, int a9 = 0 ): v( a1+a2+a3+a4+a5+a6+a7+a8+a9 )
    {
        ++instances;
    }

    ~X()
    {
        --instances;
    }

    friend class cxx11_allocator<X>;
};

int X::instances = 0;

int main()
{
    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>() );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 0 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3, 4 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3+4 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3, 4, 5 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3+4+5 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3, 4, 5, 6 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3, 4, 5, 6, 7 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6+7 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3, 4, 5, 6, 7, 8 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6+7+8 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > pi = mars_boost::allocate_shared< X >( cxx11_allocator<void>(), 1, 2, 3, 4, 5, 6, 7, 8, 9 );
        mars_boost::weak_ptr<X> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6+7+8+9 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    return mars_boost::report_errors();
}

#else // !defined( BOOST_NO_CXX11_ALLOCATOR )

int main()
{
    return 0;
}

#endif
