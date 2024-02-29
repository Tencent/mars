//
//  A negative test for unique_ptr static_cast
//
//  Copyright 2016 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/pointer_cast.hpp>
#include <memory>

struct B
{
    virtual ~B()
    {
    }
};

struct D: B
{
};

int main()
{
    std::unique_ptr<D[]> p1( new D[ 1 ] );
    std::unique_ptr<B[]> p2 = mars_boost::static_pointer_cast<B[]>( std::move( p1 ) );
}
