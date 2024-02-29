// Boost.Function library

//  Copyright (C) Douglas Gregor 2001-2005. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/function.hpp>

void test()
{
    mars_boost::function0<int> f1;
    mars_boost::function0<int> f2;

    if( f1 == f2 ) {}
}
