
// Copyright (c) 2008 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

//#define USE_MUTEX
//#define USE_RWLOCK

#include <boost/config.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#if defined( USE_RWLOCK )
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#endif

#include <boost/detail/lightweight_mutex.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/detail/lightweight_thread.hpp>

#include <cstdio>
#include <ctime>

//

int const n = 1024 * 1024;

struct X
{
    int v_; // version

    unsigned a_;
    unsigned b_;

    X(): v_( 0 ), a_( 1 ), b_( 1 )
    {
    }

    int get() const
    {
        return a_ * 7 + b_ * 11;
    }

    void set()
    {
        int tmp = get();

        b_ = a_;
        a_ = tmp;

        ++v_;
    }
};

static mars_boost::shared_ptr<X> ps( new X );

static mars_boost::detail::lightweight_mutex lm;

#if defined( USE_RWLOCK )
static mars_boost::shared_mutex rw;
#endif

static int tr = 0;

void reader( int r )
{
    int k = 0;
    unsigned s = 0;

    for( int i = 0; i < n; ++k )
    {
#if defined( USE_MUTEX )

        mars_boost::detail::lightweight_mutex::scoped_lock lock( lm );

        s += ps->get();

        BOOST_TEST( ps->v_ >= i );
        i = ps->v_;

#elif defined( USE_RWLOCK )

        mars_boost::shared_lock<mars_boost::shared_mutex> lock( rw );

        s += ps->get();

        BOOST_TEST( ps->v_ >= i );
        i = ps->v_;

#else

        mars_boost::shared_ptr<X> p2 = mars_boost::atomic_load( &ps );

        s += p2->get();

        BOOST_TEST( p2->v_ >= i );
        i = p2->v_;

#endif
    }

    printf( "Reader %d: %9d iterations (%6.3fx), %u\n", r, k, (double)k / n, s );

    mars_boost::detail::lightweight_mutex::scoped_lock lock( lm );
    tr += k;
}

void writer()
{
    for( int i = 0; i < n; ++i )
    {
#if defined( USE_MUTEX )

        mars_boost::detail::lightweight_mutex::scoped_lock lock( lm );

        BOOST_TEST( ps->v_ == i );
        ps->set();

#elif defined( USE_RWLOCK )

        mars_boost::unique_lock<mars_boost::shared_mutex> lock( rw );

        BOOST_TEST( ps->v_ == i );
        ps->set();

#else

        mars_boost::shared_ptr<X> p2( new X( *ps ) );

        BOOST_TEST( p2->v_ == i );
        p2->set();

        mars_boost::atomic_store( &ps, p2 );

#endif
    }
}

#if defined( BOOST_HAS_PTHREADS )
  char const * thmodel = "POSIX";
#else
  char const * thmodel = "Windows";
#endif

int const mr = 8; // reader threads
int const mw = 1; // writer thread

#if defined( USE_MUTEX )
  char const * prim = "mutex";
#elif defined( USE_RWLOCK )
  char const * prim = "rwlock";
#else
  char const * prim = "atomics";
#endif

int main()
{
    using namespace std; // printf, clock_t, clock

    printf( "Using %s threads: %dR + %dW threads, %d iterations, %s\n\n", thmodel, mr, mw, n, prim );

    clock_t t = clock();

    mars_boost::detail::lw_thread_t a[ mr+mw ];

    for( int i = 0; i < mr; ++i )
    {
        mars_boost::detail::lw_thread_create( a[ i ], mars_boost::bind( reader, i ) );
    }

    for( int i = mr; i < mr+mw; ++i )
    {
        mars_boost::detail::lw_thread_create( a[ i ], writer );
    }

    for( int j = 0; j < mr+mw; ++j )
    {
        mars_boost::detail::lw_thread_join( a[ j ] );
    }

    t = clock() - t;

    double ts = static_cast<double>( t ) / CLOCKS_PER_SEC;
    printf( "%.3f seconds, %.3f reads per microsecond.\n", ts, tr / ts / 1e+6 );

    return mars_boost::report_errors();
}
