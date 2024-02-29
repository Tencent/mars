//
//  shared_from_raw_test4 - based on esft_void_test.cpp
//
//  Copyright 2009, 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//


#include <boost/smart_ptr/enable_shared_from_raw.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

//

class X: public mars_boost::enable_shared_from_raw
{
};

int main()
{
    mars_boost::shared_ptr< void const volatile > pv( new X );
    mars_boost::shared_ptr< void > pv2 = mars_boost::const_pointer_cast< void >( pv );
    mars_boost::shared_ptr< X > px = mars_boost::static_pointer_cast< X >( pv2 );

    try
    {
        mars_boost::shared_ptr< X > qx = mars_boost::shared_from_raw( px.get() );

        BOOST_TEST( px == qx );
        BOOST_TEST( !( px < qx ) && !( qx < px ) );
    }
    catch( mars_boost::bad_weak_ptr const& )
    {
        BOOST_ERROR( "shared_from_this( px.get() ) failed" );
    }

    mars_boost::shared_ptr< X const volatile > px2( px );

    try
    {
        mars_boost::shared_ptr< X const volatile > qx2 = mars_boost::shared_from_raw( px2.get() );

        BOOST_TEST( px2 == qx2 );
        BOOST_TEST( !( px2 < qx2 ) && !( qx2 < px2 ) );
    }
    catch( mars_boost::bad_weak_ptr const& )
    {
        BOOST_ERROR( "shared_from_this( px2.get() ) failed" );
    }

    return mars_boost::report_errors();
}
