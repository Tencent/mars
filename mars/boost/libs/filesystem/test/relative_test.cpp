//  filesystem relative_test.cpp  ----------------------------------------------------  //

//  Copyright Beman Dawes 2015

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//  ----------------------------------------------------------------------------------  //
//
//  At least initially, development is easier if these tests are in a separate file.
//
//  ----------------------------------------------------------------------------------  //

#include <boost/config/warning_disable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/detail/lightweight_test_report.hpp>
#include <iostream>

using mars_boost::filesystem::path;
using std::cout;
using std::endl;

namespace {

void lexically_relative_test()
{
    cout << "lexically_relative_test..." << endl;

    BOOST_TEST(path("").lexically_relative("") == "");
    BOOST_TEST(path("").lexically_relative("/foo") == "");
    BOOST_TEST(path("/foo").lexically_relative("") == "");
    BOOST_TEST(path("/foo").lexically_relative("/foo") == ".");
    BOOST_TEST(path("").lexically_relative("foo") == "");
    BOOST_TEST(path("foo").lexically_relative("") == "");
    BOOST_TEST(path("foo").lexically_relative("foo") == ".");

    BOOST_TEST(path("a/b/c").lexically_relative("a") == "b/c");
    BOOST_TEST(path("a//b//c").lexically_relative("a") == "b/c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b") == "c");
    BOOST_TEST(path("a///b//c").lexically_relative("a//b") == "c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/c") == ".");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/c/x") == "..");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/c/x/y") == "../..");
    BOOST_TEST(path("a/b/c").lexically_relative("a/x") == "../b/c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/x") == "../c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/x/y") == "../../b/c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/x/y") == "../../c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/c/x/y/z") == "../../..");
    BOOST_TEST(path("a/b/c").lexically_relative("a/") == "b/c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/.") == "b/c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/./") == "b/c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/..") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/../") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/d/..") == "c");
    BOOST_TEST(path("a/b/c").lexically_relative("a/b/d/../") == "c");

    // paths unrelated except first element, and first element is root directory
    BOOST_TEST(path("/a/b/c").lexically_relative("/x") == "../a/b/c");
    BOOST_TEST(path("/a/b/c").lexically_relative("/x/y") == "../../a/b/c");
    BOOST_TEST(path("/a/b/c").lexically_relative("/x/y/z") == "../../../a/b/c");

    // paths unrelated
    BOOST_TEST(path("a/b/c").lexically_relative("x") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("x/y") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("x/y/z") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("/x") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("/x/y") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("/x/y/z") == "");
    BOOST_TEST(path("a/b/c").lexically_relative("/a/b/c") == "");

    // TODO: add some Windows-only test cases that probe presence or absence of
    // drive specifier-and root-directory

    //  Some tests from Jamie Allsop's paper
    BOOST_TEST(path("/a/d").lexically_relative("/a/b/c") == "../../d");
    BOOST_TEST(path("/a/b/c").lexically_relative("/a/d") == "../b/c");
#ifdef BOOST_WINDOWS_API
    BOOST_TEST(path("c:\\y").lexically_relative("c:\\x") == "../y");
#else
    BOOST_TEST(path("c:\\y").lexically_relative("c:\\x") == "");
#endif
    BOOST_TEST(path("d:\\y").lexically_relative("c:\\x") == "");

    //  From issue #1976
    BOOST_TEST(path("/foo/new").lexically_relative("/foo/bar") == "../new");
}

void lexically_proximate_test()
{
    cout << "lexically_proximate_test..." << endl;
    // paths unrelated
    BOOST_TEST(path("a/b/c").lexically_proximate("x") == "a/b/c");
}

} // unnamed namespace

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                     main                                             //
//                                                                                      //
//--------------------------------------------------------------------------------------//

int test_main(int, char*[])
{
// document state of critical macros
#ifdef BOOST_POSIX_API
    cout << "BOOST_POSIX_API" << endl;
#endif
#ifdef BOOST_WINDOWS_API
    cout << "BOOST_WINDOWS_API" << endl;
#endif

    lexically_relative_test();
    lexically_proximate_test();

    return ::mars_boost::report_errors();
}
