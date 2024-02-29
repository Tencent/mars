//
//  shared_from_raw_test3 - based on esft_second_ptr_test.cpp
//
//  This test has been extracted from a real
//  scenario that occurs in Boost.Python
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

void null_deleter( void const* )
{
}

int main()
{
    mars_boost::shared_ptr<X> px( new X );

    {
        mars_boost::shared_ptr<X> px2( px.get(), null_deleter );
        BOOST_TEST( px == px2 );
    }

    try
    {
        mars_boost::shared_ptr< X > qx = mars_boost::shared_from_raw( px.get() );

        BOOST_TEST( px == qx );
        BOOST_TEST( !( px < qx ) && !( qx < px ) );
    }
    catch( mars_boost::bad_weak_ptr const& )
    {
        BOOST_ERROR( "shared_from_raw( px.get() ) failed" );
    }

    return mars_boost::report_errors();
}
