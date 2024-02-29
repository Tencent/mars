//  make_local_shared_esft_test.cpp
//
//  Copyright 2007-2009, 2017 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/config.hpp>

#if defined( BOOST_NO_CXX11_RVALUE_REFERENCES ) || defined( BOOST_NO_CXX11_VARIADIC_TEMPLATES )

int main()
{
}

#else

#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class X: public mars_boost::enable_shared_from_this<X>
{
private:

    X( X const & );
    X & operator=( X const & );

public:

    static int instances;

    explicit X( int = 0, int = 0, int = 0, int = 0, int = 0, int = 0, int = 0, int = 0, int = 0 )
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
    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >();
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared_noinit< X >();
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3, 4 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3, 4, 5 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3, 4, 5, 6 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3, 4, 5, 6, 7 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3, 4, 5, 6, 7, 8 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr< X > px = mars_boost::make_local_shared< X >( 1, 2, 3, 4, 5, 6, 7, 8, 9 );
        BOOST_TEST( px.use_count() == 1 );
        BOOST_TEST( X::instances == 1 );

        try
        {
            mars_boost::shared_ptr< X > qx = px->shared_from_this();

            BOOST_TEST( px == qx );
            BOOST_TEST( !( px < qx ) && !( qx < px ) );

            px.reset();
            BOOST_TEST( X::instances == 1 );
        }
        catch( mars_boost::bad_weak_ptr const& )
        {
            BOOST_ERROR( "px->shared_from_this() failed" );
        }
    }

    BOOST_TEST( X::instances == 0 );

    return mars_boost::report_errors();
}

#endif
