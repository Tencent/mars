// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/smart_ptr.hpp>
#include <boost/config.hpp>
#include <boost/config/workaround.hpp>

int main()
{
    mars_boost::shared_ptr<int> p1( new int );
    mars_boost::shared_ptr<int[]> p2( new int[1] );

    mars_boost::make_shared<int>();

#if !BOOST_WORKAROUND(BOOST_MSVC, < 1500)

    mars_boost::make_shared<int[]>( 1 );

#endif
}
