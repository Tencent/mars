#include <boost/config.hpp>

//  wp_convertible_test.cpp
//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <boost/weak_ptr.hpp>

//

class incomplete;

struct X
{
};

struct Y
{
};

struct Z: public X
{
};

int f( mars_boost::weak_ptr<void const> )
{
    return 1;
}

int f( mars_boost::weak_ptr<int> )
{
    return 2;
}

int f( mars_boost::weak_ptr<incomplete> )
{
    return 3;
}

int g( mars_boost::weak_ptr<X> )
{
    return 4;
}

int g( mars_boost::weak_ptr<Y> )
{
    return 5;
}

int g( mars_boost::weak_ptr<incomplete> )
{
    return 6;
}

int main()
{
    BOOST_TEST( 1 == f( mars_boost::weak_ptr<double>() ) );
    BOOST_TEST( 1 == f( mars_boost::shared_ptr<double>() ) );
    BOOST_TEST( 4 == g( mars_boost::weak_ptr<Z>() ) );
    BOOST_TEST( 4 == g( mars_boost::shared_ptr<Z>() ) );

    return mars_boost::report_errors();
}
