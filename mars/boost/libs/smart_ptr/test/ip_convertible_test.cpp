#include <boost/config.hpp>

//  wp_convertible_test.cpp
//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <boost/intrusive_ptr.hpp>

//

struct W
{
};

void intrusive_ptr_add_ref( W* )
{
}

void intrusive_ptr_release( W* )
{
}

struct X: public virtual W
{
};

struct Y: public virtual W
{
};

struct Z: public X
{
};

int f( mars_boost::intrusive_ptr<X> )
{
    return 1;
}

int f( mars_boost::intrusive_ptr<Y> )
{
    return 2;
}

int main()
{
    BOOST_TEST( 1 == f( mars_boost::intrusive_ptr<Z>() ) );
    return mars_boost::report_errors();
}
