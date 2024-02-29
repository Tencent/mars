//
// Explicit instantiations are reported to exist in user code
//
// Copyright (c) 2014 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_ptr.hpp>

template class mars_boost::shared_ptr< int >;

struct X
{
};

template class mars_boost::shared_ptr< X >;

int main()
{
}
