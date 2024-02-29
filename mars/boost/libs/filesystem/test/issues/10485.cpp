//  Copyright iamvfx@gmail.com 2014

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <stdio.h>
#include <boost/filesystem.hpp>

int main()
{
    mars_boost::filesystem::path dir("/");
    for (char c : dir.filename().string())
        printf("%c\n", c);
}
