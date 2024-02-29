//
//  weak_from_raw_test3.cpp
//
//  Test that weak_from_raw and shared_from_raw
//  return consistent values from a constructor
//
//  Copyright (c) 2015 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/smart_ptr/enable_shared_from_raw.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

class X: public mars_boost::enable_shared_from_raw
{
public:

    X()
    {
        mars_boost::weak_ptr<X> p1 = mars_boost::weak_from_raw( this );
        BOOST_TEST( !p1.expired() );

        mars_boost::weak_ptr<X> p2 = mars_boost::weak_from_raw( this );
        BOOST_TEST( !p2.expired() );
        BOOST_TEST( !( p1 < p2 ) && !( p2 < p1 ) );

        mars_boost::weak_ptr<X> p3 = mars_boost::shared_from_raw( this );
        BOOST_TEST( !( p1 < p3 ) && !( p3 < p1 ) );

        mars_boost::weak_ptr<X> p4 = mars_boost::weak_from_raw( this );
        BOOST_TEST( !p4.expired() );
        BOOST_TEST( !( p3 < p4 ) && !( p4 < p3 ) );
        BOOST_TEST( !( p1 < p4 ) && !( p4 < p1 ) );
    }
};

int main()
{
    mars_boost::shared_ptr< X > px( new X );
    return mars_boost::report_errors();
}
