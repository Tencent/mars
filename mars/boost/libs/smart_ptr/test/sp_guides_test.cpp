// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if defined(__cpp_deduction_guides)

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

int main()
{
    mars_boost::shared_ptr<int> p1( new int );
    mars_boost::weak_ptr<int> p2( p1 );
    mars_boost::shared_ptr p3( p2 );
}

#else

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE( "Skipping test because __cpp_deduction_guides is not defined" )

int main() {}

#endif
