
// weak_ptr_alias_move_test.cpp
//
// Copyright 2007, 2019 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/core/lightweight_test.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/config.hpp>
#include <memory>
#include <cstddef>

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

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
        mars_boost::shared_ptr<float> p( new float );
        mars_boost::weak_ptr<float> p2( p );

        int m2 = 0;
        mars_boost::weak_ptr<int const volatile> p3( std::move( p2 ), &m2 );

        BOOST_TEST( p3.use_count() == p.use_count() );
        BOOST_TEST( share_ownership( p, p3 ) );
        BOOST_TEST( p3.lock().get() == &m2 );

        BOOST_TEST( p2.empty() );
    }

    {
        mars_boost::shared_ptr<incomplete> p;
        mars_boost::weak_ptr<incomplete> p2( p );

        int m2 = 0;
        mars_boost::weak_ptr<int const volatile> p3( std::move( p2 ), &m2 );

        BOOST_TEST( p3.use_count() == p.use_count() );
        BOOST_TEST( share_ownership( p, p3 ) );
        BOOST_TEST( p3.lock().get() == 0 );

        BOOST_TEST( p2.empty() );
    }

    {
        mars_boost::shared_ptr<X> p( new X( 5 ) );
        mars_boost::weak_ptr<X> p2( p );

        mars_boost::weak_ptr<int const volatile> p3( std::move( p2 ), &p2.lock()->v_ );

        BOOST_TEST( p3.use_count() == p.use_count() );
        BOOST_TEST( share_ownership( p, p3 ) );
        BOOST_TEST( p3.lock().get() == &p->v_ );

        BOOST_TEST( p2.empty() );

        p.reset();
        BOOST_TEST( p3.expired() );
    }

    return mars_boost::report_errors();
}

#else // defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

int main()
{
    return 0;
}

#endif
