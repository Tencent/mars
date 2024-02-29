//  Copyright (c) 2020 Andrey Semashev
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  https://www.boost.org/LICENSE_1_0.txt)

#include <boost/filesystem.hpp>

int main()
{
    mars_boost::filesystem::path p(".");
    mars_boost::filesystem::is_directory(p);
}
