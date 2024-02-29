//
//  weak_from_raw_test4.cpp
//
//  As weak_from_raw_test2.cpp, but uses weak_from_raw
//  in the constructor
//
//  Copyright (c) 2014, 2015 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/smart_ptr/enable_shared_from_raw.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

class X;

static mars_boost::weak_ptr< X > r_;

void register_( mars_boost::weak_ptr< X > const & r )
{
    r_ = r;
}

void check_( mars_boost::weak_ptr< X > const & r )
{
    BOOST_TEST( !( r < r_ ) && !( r_ < r ) );
}

void unregister_( mars_boost::weak_ptr< X > const & r )
{
    BOOST_TEST( !( r < r_ ) && !( r_ < r ) );
    r_.reset();
}

class X: public mars_boost::enable_shared_from_raw
{
public:

    X()
    {
        register_( mars_boost::weak_from_raw( this ) );
    }

    ~X()
    {
        unregister_( mars_boost::weak_from_raw( this ) );
    }

    void check()
    {
        check_( mars_boost::weak_from_raw( this ) );
    }
};

int main()
{
    {
        mars_boost::shared_ptr< X > px( new X );
        px->check();
    }

    return mars_boost::report_errors();
}
