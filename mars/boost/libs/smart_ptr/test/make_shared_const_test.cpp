// make_shared_const_test.cpp
//
// Copyright 2007-2009, 2018 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <cstddef>

class X
{
private:

    X( X const & );
    X & operator=( X const & );

    void * operator new( std::size_t n )
    {
        // lack of this definition causes link errors on Comeau C++
        BOOST_ERROR( "private X::new called" );
        return ::operator new( n );
    }

    void operator delete( void * p )
    {
        // lack of this definition causes link errors on MSVC
        BOOST_ERROR( "private X::delete called" );
        ::operator delete( p );
    }

public:

    static int instances;

    int v;

    explicit X( int a1 = 0, int a2 = 0, int a3 = 0, int a4 = 0, int a5 = 0, int a6 = 0, int a7 = 0, int a8 = 0, int a9 = 0 ): v( a1+a2+a3+a4+a5+a6+a7+a8+a9 )
    {
        ++instances;
    }

    ~X()
    {
        --instances;
    }
};

int X::instances = 0;

int main()
{
    {
        mars_boost::shared_ptr<int const> pi = mars_boost::make_shared<int const>();

        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );

        BOOST_TEST( *pi == 0 );
    }

    {
        mars_boost::shared_ptr<int const> pi = mars_boost::make_shared<int const>( 5 );

        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );

        BOOST_TEST( *pi == 5 );
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>();
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 0 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared_noinit<X const>();
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 0 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3, 4 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3+4 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3, 4, 5 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3+4+5 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3, 4, 5, 6 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3, 4, 5, 6, 7 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6+7 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3, 4, 5, 6, 7, 8 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6+7+8 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr<X const> pi = mars_boost::make_shared<X const>( 1, 2, 3, 4, 5, 6, 7, 8, 9 );
        mars_boost::weak_ptr<X const> wp( pi );

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( pi.get() != 0 );
        BOOST_TEST( pi.use_count() == 1 );
        BOOST_TEST( pi->v == 1+2+3+4+5+6+7+8+9 );

        pi.reset();

        BOOST_TEST( X::instances == 0 );
    }

    return mars_boost::report_errors();
}
