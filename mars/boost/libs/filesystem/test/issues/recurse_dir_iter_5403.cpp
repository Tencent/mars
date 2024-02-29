//  Boost Filesystem recurse_dir_iter_test.cpp  ----------------------------------------//

//  Copyright Beman Dawes 2014.

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <boost/config/warning_disable.hpp>

//  See deprecated_test for tests of deprecated features
#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include <boost/filesystem/operations.hpp>

#include <boost/cerrno.hpp>
#include <boost/system/error_code.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>

namespace fs = mars_boost::filesystem;
using mars_boost::system::error_code;

#include <iostream>

using std::cout;
using std::endl;

namespace {
typedef int errno_t;
std::string platform(BOOST_PLATFORM);
bool report_throws = false;
bool cleanup = true;
bool skip_long_windows_tests = false;

} // unnamed namespace

//------------------------------------------------------------------------------------//
//                                                                                    //
//                                    main                                            //
//                                                                                    //
//------------------------------------------------------------------------------------//

int cpp_main(int argc, char* argv[])
{
    // document state of critical macros
#ifdef BOOST_POSIX_API
    cout << "BOOST_POSIX_API is defined\n";
#endif
#ifdef BOOST_WINDOWS_API
    cout << "BOOST_WINDOWS_API is defined\n";
#endif

    for (; argc > 1; --argc, ++argv)
    {
        //if (*argv[1]=='-' && *(argv[1]+1)=='t')
        //  report_throws = true;
        //else if (*argv[1]=='-' && *(argv[1]+1)=='x')
        //  cleanup = false;
        //else if (*argv[1]=='-' && *(argv[1]+1)=='w')
        //  skip_long_windows_tests = true;
    }

    // The choice of platform to test is made at runtime rather than compile-time
    // so that compile errors for all platforms will be detected even though
    // only the current platform is runtime tested.
#if defined(BOOST_POSIX_API)
    platform = "POSIX";
#elif defined(BOOST_WINDOWS_API)
    platform = "Windows";
#else
#error neither BOOST_POSIX_API nor BOOST_WINDOWS_API is defined. See boost/system/api_config.hpp
#endif
    cout << "API is " << platform << endl;
    cout << "initial_path() is " << fs::initial_path() << endl;
    fs::path ip = fs::initial_path();

    for (fs::path::const_iterator it = ip.begin(); it != ip.end(); ++it)
    {
        if (it != ip.begin())
            cout << ", ";
        cout << *it;
    }
    cout << endl;

    //  From the root, walk the directory tree looking for a permissions error

    fs::recursive_directory_iterator it("/");
    fs::recursive_directory_iterator end_it;

    //  The increment function has an invarient that it always makes progress,
    //  so even if an error occurs this loop will eventually terminate.

    while (it != end_it)
    {
        error_code ec;
        fs::path init_path = it->path();
        it.increment(ec);
        if (ec)
        {
            cout << "initial path: " << init_path << endl;
            cout << "error_code: " << ec.value() << " with msg: " << ec.message() << endl;
            if (it != end_it)
                cout << "post-increment path: " << it->path() << endl;
        }
    }

    cout << "returning from main()" << endl;
    return ::mars_boost::report_errors();
}
