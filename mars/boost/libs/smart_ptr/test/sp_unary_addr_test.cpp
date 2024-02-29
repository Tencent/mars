//
//  sp_unary_addr_test.cpp
//
//  Copyright (c) 2007 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <memory>

struct deleter
{
private:

    void operator& ();
    void operator& () const;

public:

    int data;

    deleter(): data( 17041 )
    {
    }

    void operator()( void * )
    {
    }
};

struct X
{
};

int main()
{
    X x;

    {
        mars_boost::shared_ptr<X> p( &x, deleter() );

        deleter * q = mars_boost::get_deleter<deleter>( p );

        BOOST_TEST( q != 0 );
        BOOST_TEST( q != 0 && q->data == 17041 );
    }

#if defined( BOOST_MSVC ) && BOOST_WORKAROUND( BOOST_MSVC, < 1300 )
#else

    {
        mars_boost::shared_ptr<X> p( &x, deleter(), std::allocator<X>() );

        deleter * q = mars_boost::get_deleter<deleter>( p );

        BOOST_TEST( q != 0 );
        BOOST_TEST( q != 0 && q->data == 17041 );
    }

#endif

    return mars_boost::report_errors();
}
