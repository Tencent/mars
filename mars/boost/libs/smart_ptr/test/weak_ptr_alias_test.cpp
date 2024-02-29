
// weak_ptr_alias_test.cpp
//
// Copyright 2007, 2019 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/core/lightweight_test.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <cstddef>

//

class incomplete;

struct X
{
    int v_;

    explicit X( int v ): v_( v )
    {
    }

    ~X()
    {
        v_ = 0;
    }
};

template<class P1, class P2> bool share_ownership( P1 const& p1, P2 const& p2 )
{
    return !p1.owner_before( p2 ) && !p2.owner_before( p1 );
}

int main()
{
    {
        mars_boost::shared_ptr<double> p( new double );

        float m = 0;
        mars_boost::weak_ptr<float> p2( p, &m );

        BOOST_TEST( p2.use_count() == p.use_count() );
        BOOST_TEST( share_ownership( p, p2 ) );
        BOOST_TEST( p2.lock().get() == &m );

        {
            int m2 = 0;
            mars_boost::weak_ptr<int> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<int const> p3( p, &m2 );

            BOOST_TEST( p3.use_count() == p.use_count() );
            BOOST_TEST( share_ownership( p, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<int volatile> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void> p3( p, &m2 );

            BOOST_TEST( p3.use_count() == p.use_count() );
            BOOST_TEST( share_ownership( p, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void const volatile> p3( p, &m2 );

            BOOST_TEST( p3.use_count() == p.use_count() );
            BOOST_TEST( share_ownership( p, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void const volatile> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == &m2 );
        }
    }

    {
        mars_boost::shared_ptr<incomplete> p;

        float m = 0;
        mars_boost::weak_ptr<float> p2( p, &m );

        BOOST_TEST( p2.use_count() == p.use_count() );
        BOOST_TEST( share_ownership( p, p2 ) );

        {
            int m2 = 0;
            mars_boost::weak_ptr<int> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<int const> p3( p, &m2 );

            BOOST_TEST( p3.use_count() == p.use_count() );
            BOOST_TEST( share_ownership( p, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<int volatile> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void> p3( p, &m2 );

            BOOST_TEST( p3.use_count() == p.use_count() );
            BOOST_TEST( share_ownership( p, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void const volatile> p3( p, &m2 );

            BOOST_TEST( p3.use_count() == p.use_count() );
            BOOST_TEST( share_ownership( p, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }

        {
            int m2 = 0;
            mars_boost::weak_ptr<void const volatile> p3( p2, &m2 );

            BOOST_TEST( p3.use_count() == p2.use_count() );
            BOOST_TEST( share_ownership( p2, p3 ) );
            BOOST_TEST( p3.lock().get() == 0 );
        }
    }

    {
        mars_boost::shared_ptr<X> p( new X( 5 ) );

        mars_boost::weak_ptr<int const> p2( p, &p->v_ );

        BOOST_TEST( p2.use_count() == p.use_count() );
        BOOST_TEST( share_ownership( p, p2 ) );
        BOOST_TEST( p2.lock().get() == &p->v_ );

        p.reset();
        BOOST_TEST( p2.expired() );
    }

    return mars_boost::report_errors();
}
