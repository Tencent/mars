#include <boost/config.hpp>

//  shared_ptr_alloc3_test.cpp
//
//  Copyright (c) 2005, 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt


#include <boost/core/lightweight_test.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <cstddef>

//

struct D;

struct X
{
    static int instances;

    X(): deleted_( false )
    {
        ++instances;
    }

    ~X()
    {
        BOOST_TEST( deleted_ );
        --instances;
    }

private:

    friend struct D;

    bool deleted_;

    X( X const & );
    X & operator=( X const & );
};

int X::instances = 0;

struct D
{
    void operator()( X * px ) const
    {
        px->deleted_ = true;
        delete px;
    }
};

int main()
{
    BOOST_TEST( X::instances == 0 );

    mars_boost::shared_ptr<void> pv( new X, D(), std::allocator<X>() );

    BOOST_TEST( X::instances == 1 );

    pv.reset();

    BOOST_TEST( X::instances == 0 );

    pv.reset( new X, D(), std::allocator<void>() );

    BOOST_TEST( X::instances == 1 );

    pv.reset();

    BOOST_TEST( X::instances == 0 );

    return mars_boost::report_errors();
}
