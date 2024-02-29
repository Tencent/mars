//
//  shared_array nullptr test
//
//  Copyright 2012, 2013 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstddef>
#include <memory>

#if !defined( BOOST_NO_CXX11_NULLPTR )

struct X
{
    static int instances;

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

int X::instances = 0;

int main()
{
    {
        mars_boost::shared_array<int> p( nullptr );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        BOOST_TEST( p == nullptr );
        BOOST_TEST( nullptr == p );
        BOOST_TEST( !( p != nullptr ) );
        BOOST_TEST( !( nullptr != p ) );
    }

    {
        mars_boost::shared_array<int> p( new int[ 1 ] );

        BOOST_TEST( p.get() != 0 );
        BOOST_TEST( p.use_count() == 1 );

        BOOST_TEST( p != nullptr );
        BOOST_TEST( nullptr != p );
        BOOST_TEST( !( p == nullptr ) );
        BOOST_TEST( !( nullptr == p ) );

        p = nullptr;

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        BOOST_TEST( p == nullptr );
        BOOST_TEST( nullptr == p );
        BOOST_TEST( !( p != nullptr ) );
        BOOST_TEST( !( nullptr != p ) );
    }

    {
        BOOST_TEST( X::instances == 0 );

        mars_boost::shared_array<X> p( new X[ 2 ] );
        BOOST_TEST( X::instances == 2 );

        BOOST_TEST( p.get() != 0 );
        BOOST_TEST( p.use_count() == 1 );

        BOOST_TEST( p != nullptr );
        BOOST_TEST( nullptr != p );
        BOOST_TEST( !( p == nullptr ) );
        BOOST_TEST( !( nullptr == p ) );

        p = nullptr;
        BOOST_TEST( X::instances == 0 );

        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.use_count() == 0 );

        BOOST_TEST( p == nullptr );
        BOOST_TEST( nullptr == p );
        BOOST_TEST( !( p != nullptr ) );
        BOOST_TEST( !( nullptr != p ) );
    }

    return mars_boost::report_errors();
}

#else

int main()
{
    return 0;
}

#endif
