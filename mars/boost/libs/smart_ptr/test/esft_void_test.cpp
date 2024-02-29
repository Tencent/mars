//
//  esft_void_test.cpp
//
//  Copyright 2009 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//


#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

//

class X: public mars_boost::enable_shared_from_this<X>
{
};

int main()
{
    mars_boost::shared_ptr< void const volatile > pv( new X );
    mars_boost::shared_ptr< void > pv2 = mars_boost::const_pointer_cast< void >( pv );
    mars_boost::shared_ptr< X > px = mars_boost::static_pointer_cast< X >( pv2 );

    try
    {
        mars_boost::shared_ptr< X > qx = px->shared_from_this();

        BOOST_TEST( px == qx );
        BOOST_TEST( !( px < qx ) && !( qx < px ) );
    }
    catch( mars_boost::bad_weak_ptr const& )
    {
        BOOST_ERROR( "px->shared_from_this() failed" );
    }

    return mars_boost::report_errors();
}
