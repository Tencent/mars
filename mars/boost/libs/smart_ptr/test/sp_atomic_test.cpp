#include <boost/config.hpp>

//  sp_atomic_test.cpp
//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt


#include <boost/core/lightweight_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/memory_order.hpp>

//

struct X
{
};

#define BOOST_TEST_SP_EQ( p, q ) BOOST_TEST( p == q && !( p < q ) && !( q < p ) )

int main()
{
    mars_boost::shared_ptr<X> px( new X );

    {
        mars_boost::shared_ptr<X> p2 = mars_boost::atomic_load( &px );
        BOOST_TEST_SP_EQ( p2, px );

        mars_boost::shared_ptr<X> px2( new X );
        mars_boost::atomic_store( &px, px2 );
        BOOST_TEST_SP_EQ( px, px2 );

        p2 = mars_boost::atomic_load( &px );
        BOOST_TEST_SP_EQ( p2, px );
        BOOST_TEST_SP_EQ( p2, px2 );

        mars_boost::shared_ptr<X> px3( new X );
        mars_boost::shared_ptr<X> p3 = mars_boost::atomic_exchange( &px, px3 );
        BOOST_TEST_SP_EQ( p3, px2 );
        BOOST_TEST_SP_EQ( px, px3 );

        mars_boost::shared_ptr<X> px4( new X );
        mars_boost::shared_ptr<X> cmp;

        bool r = mars_boost::atomic_compare_exchange( &px, &cmp, px4 );
        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( px, px3 );
        BOOST_TEST_SP_EQ( cmp, px3 );

        r = mars_boost::atomic_compare_exchange( &px, &cmp, px4 );
        BOOST_TEST( r );
        BOOST_TEST_SP_EQ( px, px4 );
    }

    //

    px.reset();

    {
        mars_boost::shared_ptr<X> p2 = mars_boost::atomic_load_explicit( &px, mars_boost::memory_order_acquire );
        BOOST_TEST_SP_EQ( p2, px );

        mars_boost::shared_ptr<X> px2( new X );
        mars_boost::atomic_store_explicit( &px, px2, mars_boost::memory_order_release );
        BOOST_TEST_SP_EQ( px, px2 );

        mars_boost::shared_ptr<X> p3 = mars_boost::atomic_exchange_explicit( &px, mars_boost::shared_ptr<X>(), mars_boost::memory_order_acq_rel );
        BOOST_TEST_SP_EQ( p3, px2 );
        BOOST_TEST_SP_EQ( px, p2 );

        mars_boost::shared_ptr<X> px4( new X );
        mars_boost::shared_ptr<X> cmp( px2 );

        bool r = mars_boost::atomic_compare_exchange_explicit( &px, &cmp, px4, mars_boost::memory_order_acquire, mars_boost::memory_order_relaxed );
        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( px, p2 );
        BOOST_TEST_SP_EQ( cmp, p2 );

        r = mars_boost::atomic_compare_exchange_explicit( &px, &cmp, px4, mars_boost::memory_order_release, mars_boost::memory_order_acquire );
        BOOST_TEST( r );
        BOOST_TEST_SP_EQ( px, px4 );
    }

    return mars_boost::report_errors();
}
