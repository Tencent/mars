//  Boost Filesystem odr1_test.cpp  ----------------------------------------------------//

//  Copyright Beman Dawes 2014.

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <boost/filesystem.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace filesystem {
void tu2();
}
} // namespace mars_boost

int main()
{
    mars_boost::filesystem::tu2();
    return 0;
}
