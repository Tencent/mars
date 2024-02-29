//  filesystem tut4.cpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2009

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>

using std::cout;
using namespace mars_boost::filesystem;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: tut4 path\n";
        return 1;
    }

    path p(argv[1]);

    try
    {
        if (exists(p))
        {
            if (is_regular_file(p))
            {
                cout << p << " size is " << file_size(p) << '\n';
            }
            else if (is_directory(p))
            {
                cout << p << " is a directory containing:\n";

                std::vector< path > v;

                for (auto&& x : directory_iterator(p))
                    v.push_back(x.path());

                std::sort(v.begin(), v.end());

                for (auto&& x : v)
                    cout << "    " << x.filename() << '\n';
            }
            else
                cout << p << " exists, but is not a regular file or directory\n";
        }
        else
            cout << p << " does not exist\n";
    }
    catch (filesystem_error& ex)
    {
        cout << ex.what() << '\n';
    }

    return 0;
}
