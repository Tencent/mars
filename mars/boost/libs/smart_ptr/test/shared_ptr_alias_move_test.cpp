#include <boost/config.hpp>

//  shared_ptr_alias_move_test.cpp
//
//  Copyright (c) 2007 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/core/lightweight_test.hpp>
#include <boost/shared_ptr.hpp>
#include <utility>

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

class incomplete;

struct X
{
    static long instances;

    int v_;

    explicit X( int v ): v_( v )
    {
        ++instances;
    }

    ~X()
    {
        v_ = 0;
        --instances;
    }

private:
    X( X const & );
    X & operator=( X const & );
};

long X::instances = 0;

int main()
{
    BOOST_TEST( X::instances == 0 );

    {
        int m = 0;
        mars_boost::shared_ptr< int > p;
        mars_boost::shared_ptr< int > p2( std::move( p ), &m );

        BOOST_TEST( p2.get() == &m );
        BOOST_TEST( p2? true: false );
        BOOST_TEST( !!p2 );
        BOOST_TEST( p2.use_count() == 0 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        p2.reset( std::move( p ), 0 );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p2? false: true );
        BOOST_TEST( !p2 );
        BOOST_TEST( p2.use_count() == 0 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );
    }

    {
        int m = 0;
        mars_boost::shared_ptr< int > p( new int );
        mars_boost::shared_ptr< int const > p2( std::move( p ), &m );

        BOOST_TEST( p2.get() == &m );
        BOOST_TEST( p2? true: false );
        BOOST_TEST( !!p2 );
        BOOST_TEST( p2.use_count() == 1 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        mars_boost::shared_ptr< int volatile > p3;
        p2.reset( std::move( p3 ), 0 );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p2? false: true );
        BOOST_TEST( !p2 );
        BOOST_TEST( p2.use_count() == 0 );

        BOOST_TEST( p3.get() == 0 );
        BOOST_TEST( p3.use_count() == 0 );

        mars_boost::shared_ptr< int const volatile > p4( new int );
        p2.reset( std::move( p4 ), &m );

        BOOST_TEST( p2.get() == &m );
        BOOST_TEST( p2.use_count() == 1 );

        BOOST_TEST( p4.get() == 0 );
        BOOST_TEST( p4.use_count() == 0 );
    }

    {
        mars_boost::shared_ptr< int > p( new int );
        mars_boost::shared_ptr< void const > p2( std::move( p ), 0 );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p2? false: true );
        BOOST_TEST( !p2 );
        BOOST_TEST( p2.use_count() == 1 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        int m = 0;
        mars_boost::shared_ptr< void volatile > p3;

        p2.reset( std::move( p3 ), &m );

        BOOST_TEST( p2.get() == &m );
        BOOST_TEST( p2? true: false );
        BOOST_TEST( !!p2 );
        BOOST_TEST( p2.use_count() == 0 );

        BOOST_TEST( p3.get() == 0 );
        BOOST_TEST( p3.use_count() == 0 );

        mars_boost::shared_ptr< void const volatile > p4( new int );
        p2.reset( std::move( p4 ), 0 );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p2.use_count() == 1 );

        BOOST_TEST( p4.get() == 0 );
        BOOST_TEST( p4.use_count() == 0 );
    }

    {
        mars_boost::shared_ptr< incomplete > p;
        mars_boost::shared_ptr< incomplete > p2( std::move( p ), 0 );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p2? false: true );
        BOOST_TEST( !p2 );
        BOOST_TEST( p2.use_count() == 0 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        p2.reset( std::move( p ), 0 );

        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p2? false: true );
        BOOST_TEST( !p2 );
        BOOST_TEST( p2.use_count() == 0 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );
    }

    {
        mars_boost::shared_ptr< X > p( new X( 5 ) ), q( p );
        mars_boost::shared_ptr< int const > p2( std::move( q ), &q->v_ );

        BOOST_TEST( p2.get() == &p->v_ );
        BOOST_TEST( p2? true: false );
        BOOST_TEST( !!p2 );
        BOOST_TEST( p2.use_count() == p.use_count() );
        BOOST_TEST( !( p < p2 ) && !( p2 < p ) );

        BOOST_TEST( q.get() == 0 );
        BOOST_TEST( q.use_count() == 0 );

        p.reset();
        BOOST_TEST( *p2 == 5 );

        mars_boost::shared_ptr< X const > p3( new X( 8 ) ), q3( p3 );
        p2.reset( std::move( q3 ), &q3->v_ );

        BOOST_TEST( p2.get() == &p3->v_ );
        BOOST_TEST( p2? true: false );
        BOOST_TEST( !!p2 );
        BOOST_TEST( p2.use_count() == p3.use_count() );
        BOOST_TEST( !( p3 < p2 ) && !( p2 < p3 ) );

        BOOST_TEST( q3.get() == 0 );
        BOOST_TEST( q3.use_count() == 0 );

        p3.reset();
        BOOST_TEST( *p2 == 8 );
    }

    {
        mars_boost::shared_ptr< X > p( new X( 5 ) );
        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( p.unique() );
        BOOST_TEST( p->v_ == 5 );

        mars_boost::shared_ptr< X > p2( std::move( p ), p.get() );
        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p2.unique() );
        BOOST_TEST( p2->v_ == 5 );

        mars_boost::shared_ptr< int const > p3( std::move( p2 ), &p2->v_ );
        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( p2.get() == 0 );
        BOOST_TEST( p3.unique() );
        BOOST_TEST( *p3 == 5 );

        p3.reset();
        BOOST_TEST( X::instances == 0 );
    }

    {
        mars_boost::shared_ptr< X > p( new X( 5 ) );
        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( p.unique() );
        BOOST_TEST( p->v_ == 5 );

        {
            mars_boost::shared_ptr< X > p2(p);
            BOOST_TEST( X::instances == 1 );
            BOOST_TEST( p.get() == p2.get() );
            BOOST_TEST( p.use_count() == 2 );
            BOOST_TEST( p2.use_count() == 2 );

            mars_boost::shared_ptr< int const > p3( std::move( p2 ), &p2->v_ );
            BOOST_TEST( X::instances == 1 );
            BOOST_TEST( p.use_count() == 2 );
            BOOST_TEST( p2.use_count() == 0 );
            BOOST_TEST( p2.get() == 0 );
            BOOST_TEST( p3.use_count() == 2 );
            BOOST_TEST( p3.get() == &p->v_ );
        }

        BOOST_TEST( X::instances == 1 );
        BOOST_TEST( p.unique() );
        BOOST_TEST( p->v_ == 5 );
    }

    return mars_boost::report_errors();
}

#else // defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

int main()
{
    return 0;
}

#endif
