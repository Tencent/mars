//
//  shared_ptr_rv_test.cpp
//
//  Copyright (c) 2007 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

struct X
{
    static long instances;

    X()
    {
        ++instances;
    }

    ~X()
    {
        --instances;
    }

    static mars_boost::shared_ptr<X> create()
    {
        return mars_boost::shared_ptr<X>( new X );
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
        mars_boost::shared_ptr<X> p( X::create() );
        BOOST_TEST( X::instances == 1 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );

        p.reset();
        BOOST_TEST( X::instances == 0 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr<X const> p( X::create() );
        BOOST_TEST( X::instances == 1 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );

        p.reset();
        BOOST_TEST( X::instances == 0 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr<void> p( X::create() );
        BOOST_TEST( X::instances == 1 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );

        p.reset();
        BOOST_TEST( X::instances == 0 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );
    }

    BOOST_TEST( X::instances == 0 );

    {
        mars_boost::shared_ptr<void const> p( X::create() );
        BOOST_TEST( X::instances == 1 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );

        p.reset();
        BOOST_TEST( X::instances == 0 );

        p = X::create();
        BOOST_TEST( X::instances == 1 );
    }

    BOOST_TEST( X::instances == 0 );

    return mars_boost::report_errors();
}
