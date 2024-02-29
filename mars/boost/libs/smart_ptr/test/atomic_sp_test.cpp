#include <boost/config.hpp>

//  atomic_sp_test.cpp
//
//  Copyright 2008, 2017 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt


#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/memory_order.hpp>

//

struct X
{
};

#define BOOST_TEST_SP_EQ( p, q ) BOOST_TEST( p == q && !( p < q ) && !( q < p ) )

int main()
{
    // default constructor

    {
        mars_boost::atomic_shared_ptr<X> apx;

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( p2.get(), (X*)0 );
        BOOST_TEST_EQ( p2.use_count(), 0 );
    }

    // shared_ptr constructor

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // shared_ptr assignment

    {
        mars_boost::shared_ptr<X> px0( new X );
        mars_boost::atomic_shared_ptr<X> apx( px0 );

        mars_boost::shared_ptr<X> px( new X );
        apx = px;

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // load, w/ mo

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> p2 = apx.load( mars_boost::memory_order_acquire );

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // operator shared_ptr

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> p2 = apx;

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // store

    {
        mars_boost::shared_ptr<X> px0( new X );
        mars_boost::atomic_shared_ptr<X> apx( px0 );

        mars_boost::shared_ptr<X> px( new X );
        apx.store( px );

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // store, w/ mo

    {
        mars_boost::shared_ptr<X> px0( new X );
        mars_boost::atomic_shared_ptr<X> apx( px0 );

        mars_boost::shared_ptr<X> px( new X );
        apx.store( px, mars_boost::memory_order_release );

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // exchange

    {
        mars_boost::shared_ptr<X> px0( new X );
        mars_boost::atomic_shared_ptr<X> apx( px0 );

        mars_boost::shared_ptr<X> px( new X );
        mars_boost::shared_ptr<X> p1 = apx.exchange( px );

        BOOST_TEST_EQ( px0, p1 );
        BOOST_TEST_EQ( px0.use_count(), 2 );
        BOOST_TEST_EQ( p1.use_count(), 2 );

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // exchange, w/ mo

    {
        mars_boost::shared_ptr<X> px0( new X );
        mars_boost::atomic_shared_ptr<X> apx( px0 );

        mars_boost::shared_ptr<X> px( new X );
        mars_boost::shared_ptr<X> p1 = apx.exchange( px, mars_boost::memory_order_acq_rel );

        BOOST_TEST_EQ( px0, p1 );
        BOOST_TEST_EQ( px0.use_count(), 2 );
        BOOST_TEST_EQ( p1.use_count(), 2 );

        mars_boost::shared_ptr<X> p2 = apx.load();

        BOOST_TEST_EQ( px, p2 );
        BOOST_TEST_EQ( px.use_count(), 3 );
        BOOST_TEST_EQ( p2.use_count(), 3 );
    }

    // compare_exchange_weak

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> px2( new X );
        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_weak( cmp, px2 );
        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_weak( cmp, px2 );
        BOOST_TEST( r );
        BOOST_TEST_SP_EQ( apx.load(), px2 );
    }

    // compare_exchange_weak, w/ mo

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> px2( new X );
        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_weak( cmp, px2, mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );
        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_weak( cmp, px2, mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );
        BOOST_TEST( r );
        BOOST_TEST_SP_EQ( apx.load(), px2 );
    }

    // compare_exchange_weak, rv

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_weak( cmp, mars_boost::shared_ptr<X>() );

        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_weak( cmp, mars_boost::shared_ptr<X>() );

        BOOST_TEST( r );
        BOOST_TEST( apx.load().get() == 0 );
        BOOST_TEST( apx.load().use_count() == 0 );
    }

    // compare_exchange_weak, rv, w/ mo

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_weak( cmp, mars_boost::shared_ptr<X>(), mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );

        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_weak( cmp, mars_boost::shared_ptr<X>(), mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );

        BOOST_TEST( r );
        BOOST_TEST( apx.load().get() == 0 );
        BOOST_TEST( apx.load().use_count() == 0 );
    }

    // compare_exchange_strong

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> px2( new X );
        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_strong( cmp, px2 );
        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_strong( cmp, px2 );
        BOOST_TEST( r );
        BOOST_TEST_SP_EQ( apx.load(), px2 );
    }

    // compare_exchange_strong, w/ mo

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> px2( new X );
        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_strong( cmp, px2, mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );
        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_strong( cmp, px2, mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );
        BOOST_TEST( r );
        BOOST_TEST_SP_EQ( apx.load(), px2 );
    }

    // compare_exchange_strong, rv

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_strong( cmp, mars_boost::shared_ptr<X>() );

        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_strong( cmp, mars_boost::shared_ptr<X>() );

        BOOST_TEST( r );
        BOOST_TEST( apx.load().get() == 0 );
        BOOST_TEST( apx.load().use_count() == 0 );
    }

    // compare_exchange_strong, rv, w/ mo

    {
        mars_boost::shared_ptr<X> px( new X );
        mars_boost::atomic_shared_ptr<X> apx( px );

        mars_boost::shared_ptr<X> cmp;

        bool r = apx.compare_exchange_strong( cmp, mars_boost::shared_ptr<X>(), mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );

        BOOST_TEST( !r );
        BOOST_TEST_SP_EQ( apx.load(), px );
        BOOST_TEST_SP_EQ( cmp, px );

        r = apx.compare_exchange_strong( cmp, mars_boost::shared_ptr<X>(), mars_boost::memory_order_seq_cst, mars_boost::memory_order_seq_cst );

        BOOST_TEST( r );
        BOOST_TEST( apx.load().get() == 0 );
        BOOST_TEST( apx.load().use_count() == 0 );
    }

    return mars_boost::report_errors();
}
