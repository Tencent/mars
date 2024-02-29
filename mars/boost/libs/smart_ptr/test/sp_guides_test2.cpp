// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if defined(__cpp_deduction_guides)

#include <boost/shared_ptr.hpp>
#include <memory>

int main()
{
    mars_boost::shared_ptr p2( std::unique_ptr<int>( new int ) );
}

#else

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE( "Skipping test because __cpp_deduction_guides is not defined" )

int main() {}

#endif
