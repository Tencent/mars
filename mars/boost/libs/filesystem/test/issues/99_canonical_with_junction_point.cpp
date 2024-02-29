//  Boost operations_test.cpp  ---------------------------------------------------------//

//  Copyright Alexander Grund 2020

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <iostream>

#if defined(BOOST_FILESYSTEM_HAS_MKLINK)

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstdlib>
#include <vector>

namespace fs = mars_boost::filesystem;

struct TmpDir
{
    fs::path path;
    TmpDir(const fs::path& base) :
        path(fs::absolute(base) / fs::unique_path())
    {
        fs::create_directories(path);
    }
    ~TmpDir()
    {
        mars_boost::system::error_code ec;
        fs::remove_all(path, ec);
    }
};

// Test fs::canonical for various path in a Windows directory junction point
// This failed before due to broken handling of absolute paths and ignored ReparseTag
int main()
{

    const fs::path cwd = fs::current_path();
    const TmpDir tmp(cwd);
    const fs::path junction = tmp.path / "junction";
    const fs::path real = tmp.path / "real";
    const fs::path subDir = "sub";
    fs::create_directories(real / subDir);
    fs::current_path(tmp.path);
    BOOST_TEST(std::system("mklink /J junction real") == 0);
    BOOST_TEST(fs::exists(junction));

    // Due to a bug there was a dependency on the current path so try the below for all:
    std::vector< fs::path > paths;
    paths.push_back(cwd);
    paths.push_back(junction);
    paths.push_back(real);
    paths.push_back(junction / subDir);
    paths.push_back(real / subDir);
    for (std::vector< fs::path >::iterator it = paths.begin(); it != paths.end(); ++it)
    {
        std::cout << "Testing in " << *it << std::endl;
        fs::current_path(*it);

        // Used by canonical, must work too
        BOOST_TEST(fs::read_symlink(junction) == real);

        BOOST_TEST(fs::canonical(junction) == real);
        BOOST_TEST(fs::canonical(junction / subDir) == real / subDir);
    }

    // Restore the original current directory so that temp directory can be removed
    fs::current_path(cwd);

    return mars_boost::report_errors();
}

#else // defined(BOOST_FILESYSTEM_HAS_MKLINK)

int main()
{
    std::cout << "Skipping test as the target system does not support mklink." << std::endl;
    return 0;
}

#endif // defined(BOOST_FILESYSTEM_HAS_MKLINK)
