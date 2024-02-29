//
//  A negative test for unique_ptr const_cast
//
//  Copyright 2016 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/pointer_cast.hpp>
#include <memory>

int main()
{
    std::unique_ptr<int> p1( new int );
    std::unique_ptr<int[]> p2 = mars_boost::const_pointer_cast<int[]>( std::move( p1 ) );
}
