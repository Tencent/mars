//  path_test program  -----------------------------------------------------------------//

//  Copyright Beman Dawes 2002, 2008
//  Copyright Vladimir Prus 2002

//  Use, modification, and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/filesystem

//  basic_path's stem(), extension(), and replace_extension() tests are based
//  on basename(), extension(), and change_extension() tests from the original
//  convenience_test.cpp by Vladimir Prus.

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                     Caution                                          //
//                                                                                      //
//  The class path relational operators (==, !=, <, etc.) on Windows treat slash and    //
//  backslash as equal. Thus any tests on Windows where the difference between slash    //
//  and backslash is significant should compare strings rather than paths.              //
//                                                                                      //
//  BOOST_TEST(path == path)                     // '\\' and '/' are equal              //
//  BOOST_TEST(path == convertable to string)    // '\\' and '/' are equal              //
//  PATH_TEST_EQ(path, path)                     // '\\' and '/' are equal              //
//                                                                                      //
//  BOOST_TEST(path.string() == path.string())   // '\\' and '/' are not equal          //
//  BOOST_TEST(path.string() ==                                                         //
//               convertable to string)          // '\\' and '/' are not equal          //
//  PATH_TEST_EQ(path.string(),                                                         //
//                convertable to string)         // '\\' and '/' are not equal          //
//                                                                                      //
//  The last of these is often what is needed, so the PATH_TEST_EQ macro is provided.   //
//  It converts its first argument to a path, and then performs a .string() on it,      //
//  eliminating much boilerplate .string() or even path(...).string() code.             //
//                                                                                      //
//  PATH_TEST_EQ(path, convertable to string)    // '\\' and '/' are not equal          //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#include <boost/config/warning_disable.hpp>

//  See deprecated_test for tests of deprecated features
#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include <boost/config.hpp>
#include <boost/next_prior.hpp>
#include <boost/utility/string_view.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
#include <string_view>
#endif
#include <boost/core/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>

namespace fs = mars_boost::filesystem;
using mars_boost::filesystem::path;
using mars_boost::next;
using mars_boost::prior;

#ifdef BOOST_WINDOWS_API
#define BOOST_DIR_SEP "\\"
#else
#define BOOST_DIR_SEP "/"
#endif

#if BOOST_FILESYSTEM_VERSION == 3
#define BOOST_FILESYSTEM_V3_TRAILING_DOT "."
#else
#define BOOST_FILESYSTEM_V3_TRAILING_DOT ""
#endif

#define PATH_TEST_EQ(a, b) check(a, b, __FILE__, __LINE__)

namespace {

class derived_from_path :
    public fs::path
{
public:
    derived_from_path() {}
    derived_from_path(derived_from_path const& that) : fs::path(static_cast< fs::path const& >(that)) {}
    template< typename T >
    derived_from_path(T const& that) : fs::path(that) {}

    derived_from_path& operator= (derived_from_path const& that)
    {
        *static_cast< fs::path* >(this) = that;
        return *this;
    }
    template< typename T >
    derived_from_path& operator= (T const& that)
    {
        *static_cast< fs::path* >(this) = that;
        return *this;
    }
};

class convertible_to_path
{
private:
    fs::path m_path;

public:
    convertible_to_path() {}
    convertible_to_path(convertible_to_path const& that) : m_path(that.m_path) {}
    template< typename T >
    convertible_to_path(T const& that) : m_path(that) {}

    convertible_to_path& operator= (convertible_to_path const& that)
    {
        m_path = that.m_path;
        return *this;
    }
    template< typename T >
    convertible_to_path& operator= (T const& that)
    {
        m_path = that;
        return *this;
    }

    operator fs::path() const { return m_path; }
};

template< typename Char >
class basic_custom_string
{
public:
    typedef std::basic_string< Char > string_type;
    typedef typename string_type::size_type size_type;
    typedef typename string_type::difference_type difference_type;
    typedef typename string_type::value_type value_type;
    typedef typename string_type::reference reference;
    typedef typename string_type::const_reference const_reference;
    typedef typename string_type::pointer pointer;
    typedef typename string_type::const_pointer const_pointer;
    typedef typename string_type::iterator iterator;
    typedef typename string_type::const_iterator const_iterator;

private:
    string_type m_str;

public:
    basic_custom_string() {}
    explicit basic_custom_string(const_pointer str) : m_str(str) {}
    explicit basic_custom_string(string_type const& str) : m_str(str) {}
    template< typename OtherChar >
    explicit basic_custom_string(const OtherChar* str)
    {
        // Do a simple character code conversion; only valid for ASCII characters
        while (*str != static_cast< OtherChar >(0))
        {
            m_str.push_back(static_cast< value_type >(*str));
            ++str;
        }
    }

    bool empty() const { return m_str.empty(); }
    size_type size() const { return m_str.size(); }

    const_pointer data() const { return m_str.data(); }
    const_pointer c_str() const { return m_str.c_str(); }

    iterator begin() { return m_str.begin(); }
    const_iterator begin() const { return m_str.begin(); }
    iterator end() { return m_str.end(); }
    const_iterator end() const { return m_str.end(); }

    operator string_type() const { return m_str; }
};

typedef basic_custom_string< char > custom_string;
typedef basic_custom_string< wchar_t > wcustom_string;
typedef basic_custom_string< fs::path::value_type > pcustom_string;

#if defined(__clang__)
#pragma clang diagnostic push
// unused function 'to_string'
#pragma clang diagnostic ignored "-Wunused-function"
#endif // defined(__clang__)

inline std::string const& to_string(std::string const& s)
{
    return s;
}

inline std::string to_string(fs::path const& p)
{
    return p.string();
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif // defined(__clang__)

std::string platform(BOOST_PLATFORM);

void check(const fs::path& source, const std::string& expected, const char* file, int line)
{
    if (source.string() == expected)
        return;

    std::cout << file
              << '(' << line << "): source: \"" << source.string()
              << "\" != expected: \"" << expected
              << "\"" << std::endl;

    ++::mars_boost::detail::test_errors();
}

path p1("fe/fi/fo/fum");
path p2(p1);
path p3;
path p4("foobar");
path p5;

//  exception_tests  -----------------------------------------------------------------//

void exception_tests()
{
    std::cout << "exception_tests..." << std::endl;
    const std::string str_1("string-1");
    mars_boost::system::error_code ec(12345, mars_boost::system::system_category());
    try
    {
        throw fs::filesystem_error(str_1, ec);
    }
    catch (const fs::filesystem_error& ex)
    {
        //std::cout << ex.what() << "*" << std::endl;
        //BOOST_TEST(std::strcmp(ex.what(),
        //  "string-1: Unknown error") == 0);
        BOOST_TEST(ex.code() == ec);
    }

    try
    {
        throw fs::filesystem_error(str_1, "p1", "p2", ec);
    }
    catch (const fs::filesystem_error& ex)
    {
        //std::cout << ex.what() << "*" << std::endl;
        //BOOST_TEST(std::strcmp(ex.what(),
        //  "string-1: Unknown error: \"p1\", \"p2\"") == 0);
        BOOST_TEST(ex.code() == ec);
        BOOST_TEST(ex.path1() == "p1");
        BOOST_TEST(ex.path2() == "p2");
    }
}

//  overload_tests  ------------------------------------------------------------------//

// These verify various overloads don't cause compiler errors
// They pre-date operations_unit_test.cpp

void overload_tests()
{
    std::cout << "overload_tests..." << std::endl;

    fs::exists(p1);
    fs::exists("foo");
    fs::exists(std::string("foo"));

    fs::exists(p1 / path("foo"));
    fs::exists(p1 / "foo");
    fs::exists(p1 / std::string("foo"));

    fs::exists("foo" / p1);
    fs::exists(std::string("foo") / p1);

    p4 /= path("foo");
    p4 /= "foo";
    p4 /= std::string("foo");
}

//  iterator_tests  ------------------------------------------------------------------//

void iterator_tests()
{
    std::cout << "iterator_tests..." << std::endl;

    path itr_ck = "";
    path::const_iterator itr = itr_ck.begin();
    BOOST_TEST(itr == itr_ck.end());

    itr_ck = "/";
    itr = itr_ck.begin();
    BOOST_TEST(itr->string() == "/");
    BOOST_TEST(++itr == itr_ck.end());
    BOOST_TEST((--itr)->string() == "/");

    itr_ck = "foo";
    BOOST_TEST(*itr_ck.begin() == std::string("foo"));
    BOOST_TEST(mars_boost::next(itr_ck.begin()) == itr_ck.end());
    BOOST_TEST(*mars_boost::prior(itr_ck.end()) == std::string("foo"));
    BOOST_TEST(mars_boost::prior(itr_ck.end()) == itr_ck.begin());

    itr_ck = path("/foo");
    BOOST_TEST((itr_ck.begin())->string() == "/");
    BOOST_TEST(*mars_boost::next(itr_ck.begin()) == std::string("foo"));
    BOOST_TEST(mars_boost::next(mars_boost::next(itr_ck.begin())) == itr_ck.end());
    BOOST_TEST(mars_boost::next(itr_ck.begin()) == mars_boost::prior(itr_ck.end()));
    BOOST_TEST(*mars_boost::prior(itr_ck.end()) == std::string("foo"));
    BOOST_TEST(*mars_boost::prior(mars_boost::prior(itr_ck.end())) == std::string("/"));
    BOOST_TEST(mars_boost::prior(mars_boost::prior(itr_ck.end())) == itr_ck.begin());

    itr_ck = "/foo/bar";
    itr = itr_ck.begin();
    path::const_iterator itr_begin = itr;
    BOOST_TEST(itr->string() == "/");
    BOOST_TEST(*++itr == std::string("foo"));
    BOOST_TEST(*++itr == std::string("bar"));
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, "bar");
    PATH_TEST_EQ(*--itr, "foo");
    PATH_TEST_EQ(*--itr, "/");
    BOOST_TEST(itr == itr_begin);

    itr_ck = "../f"; // previously failed due to short name bug
    itr_begin = itr = itr_ck.begin();
    PATH_TEST_EQ(itr->string(), "..");
    PATH_TEST_EQ(*++itr, "f");
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, "f");
    PATH_TEST_EQ(*--itr, "..");
    BOOST_TEST(itr == itr_begin);

    // POSIX says treat "/foo/bar/" as "/foo/bar/."
    itr_ck = "/foo/bar/";
    itr_begin = itr = itr_ck.begin();
    PATH_TEST_EQ(itr->string(), "/");
    PATH_TEST_EQ(*++itr, "foo");
    BOOST_TEST(itr != itr_ck.end());
    PATH_TEST_EQ(*++itr, "bar");
    BOOST_TEST(itr != itr_ck.end());
    PATH_TEST_EQ(*++itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    BOOST_TEST(itr != itr_ck.end()); // verify the . isn't also seen as end()
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(*--itr, "bar");
    PATH_TEST_EQ(*--itr, "foo");
    PATH_TEST_EQ(*--itr, "/");
    BOOST_TEST(itr == itr_begin);

    // POSIX says treat "/f/b/" as "/f/b/."
    itr_ck = "/f/b/";
    itr_begin = itr = itr_ck.begin();
    PATH_TEST_EQ(itr->string(), "/");
    PATH_TEST_EQ(*++itr, "f");
    PATH_TEST_EQ(*++itr, "b");
    PATH_TEST_EQ(*++itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    BOOST_TEST(itr != itr_ck.end()); // verify the . isn't also seen as end()
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(*--itr, "b");
    PATH_TEST_EQ(*--itr, "f");
    PATH_TEST_EQ(*--itr, "/");
    BOOST_TEST(itr == itr_begin);

    // POSIX says treat "a/b/" as "a/b/."
    // Although similar to the prior test case, this failed the ". isn't end" test due to
    // a bug while the prior case did not fail.
    itr_ck = "a/b/";
    itr_begin = itr = itr_ck.begin();
    PATH_TEST_EQ(*itr, "a");
    PATH_TEST_EQ(*++itr, "b");
    PATH_TEST_EQ(*++itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    BOOST_TEST(itr != itr_ck.end()); // verify the . isn't also seen as end()
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(*--itr, "b");
    PATH_TEST_EQ(*--itr, "a");
    BOOST_TEST(itr == itr_begin);

    itr_ck = "//net";
    itr_begin = itr = itr_ck.begin();
    // two leading slashes are permitted by POSIX (as implementation defined),
    // while for Windows it is always well defined (as a network name)
    PATH_TEST_EQ(itr->string(), "//net");
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, "//net");
    BOOST_TEST(itr == itr_begin);

    itr_ck = "//net/";
    itr_begin = itr = itr_ck.begin();
    PATH_TEST_EQ(itr->string(), "//net");
    PATH_TEST_EQ(*++itr, "/");
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, "/");
    PATH_TEST_EQ(*--itr, "//net");
    BOOST_TEST(itr == itr_begin);

    itr_ck = "//foo///bar///";
    itr_begin = itr = itr_ck.begin();
    PATH_TEST_EQ(itr->string(), "//foo");
    PATH_TEST_EQ(*++itr, "/");
    PATH_TEST_EQ(*++itr, "bar");
    PATH_TEST_EQ(*++itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(*--itr, "bar");
    PATH_TEST_EQ(*--itr, "/");
    PATH_TEST_EQ(*--itr, "//foo");
    BOOST_TEST(itr == itr_begin);

    itr_ck = "///foo///bar///";
    itr_begin = itr = itr_ck.begin();
    // three or more leading slashes are to be treated as a single slash
    PATH_TEST_EQ(itr->string(), "/");
    PATH_TEST_EQ(*++itr, "foo");
    PATH_TEST_EQ(*++itr, "bar");
    PATH_TEST_EQ(*++itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    BOOST_TEST(++itr == itr_ck.end());
    PATH_TEST_EQ(*--itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(*--itr, "bar");
    PATH_TEST_EQ(*--itr, "foo");
    PATH_TEST_EQ(*--itr, "/");
    BOOST_TEST(itr == itr_begin);

    if (platform == "Windows")
    {
        itr_ck = "c:/";
        itr_begin = itr = itr_ck.begin();
        PATH_TEST_EQ(itr->string(), "c:");
        PATH_TEST_EQ(*++itr, std::string("/"));
        BOOST_TEST(++itr == itr_ck.end());
        PATH_TEST_EQ(*--itr, "/");
        PATH_TEST_EQ(*--itr, "c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "c:\\";
        itr_begin = itr = itr_ck.begin();
        PATH_TEST_EQ(itr->string(), "c:");
        PATH_TEST_EQ(*++itr, "/"); // test that iteration returns generic format
        BOOST_TEST(++itr == itr_ck.end());
        PATH_TEST_EQ(*--itr, "/"); // test that iteration returns generic format
        PATH_TEST_EQ(*--itr, "c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "c:/foo";
        itr_begin = itr = itr_ck.begin();
        PATH_TEST_EQ(*itr, "c:");
        PATH_TEST_EQ(*++itr, "/");
        PATH_TEST_EQ(*++itr, "foo");
        BOOST_TEST(++itr == itr_ck.end());
        PATH_TEST_EQ(*--itr, "foo");
        PATH_TEST_EQ(*--itr, "/");
        PATH_TEST_EQ(*--itr, "c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "c:\\foo";
        itr_begin = itr = itr_ck.begin();
        BOOST_TEST_EQ(*itr, "c:");
        BOOST_TEST_EQ(*++itr, "\\");
        BOOST_TEST_EQ(*++itr, "foo");
        BOOST_TEST(++itr == itr_ck.end());
        BOOST_TEST_EQ(*--itr, "foo");
        BOOST_TEST_EQ(*--itr, "\\");
        BOOST_TEST_EQ(*--itr, "c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "\\\\?\\c:\\foo";
        itr_begin = itr = itr_ck.begin();
        BOOST_TEST_EQ(*itr, "\\\\?\\c:");
        BOOST_TEST_EQ(*++itr, "\\");
        BOOST_TEST_EQ(*++itr, "foo");
        BOOST_TEST(++itr == itr_ck.end());
        BOOST_TEST_EQ(*--itr, "foo");
        BOOST_TEST_EQ(*--itr, "\\");
        BOOST_TEST_EQ(*--itr, "\\\\?\\c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "\\\\.\\c:\\foo";
        itr_begin = itr = itr_ck.begin();
        BOOST_TEST_EQ(*itr, "\\\\.\\c:");
        BOOST_TEST_EQ(*++itr, "\\");
        BOOST_TEST_EQ(*++itr, "foo");
        BOOST_TEST(++itr == itr_ck.end());
        BOOST_TEST_EQ(*--itr, "foo");
        BOOST_TEST_EQ(*--itr, "\\");
        BOOST_TEST_EQ(*--itr, "\\\\.\\c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "\\??\\c:\\foo";
        itr_begin = itr = itr_ck.begin();
        BOOST_TEST_EQ(*itr, "\\??\\c:");
        BOOST_TEST_EQ(*++itr, "\\");
        BOOST_TEST_EQ(*++itr, "foo");
        BOOST_TEST(++itr == itr_ck.end());
        BOOST_TEST_EQ(*--itr, "foo");
        BOOST_TEST_EQ(*--itr, "\\");
        BOOST_TEST_EQ(*--itr, "\\??\\c:");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "\\\\\\foo\\\\\\bar\\\\\\";
        itr_begin = itr = itr_ck.begin();
        // three or more leading slashes are to be treated as a single slash
        PATH_TEST_EQ(itr->string(), "/");
        PATH_TEST_EQ(*++itr, "foo");
        PATH_TEST_EQ(*++itr, "bar");
        PATH_TEST_EQ(*++itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
        BOOST_TEST(++itr == itr_ck.end());
        PATH_TEST_EQ(*--itr, BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(*--itr, "bar");
        PATH_TEST_EQ(*--itr, "foo");
        PATH_TEST_EQ(*--itr, "/");
        BOOST_TEST(itr == itr_begin);

        itr_ck = "c:foo";
        itr_begin = itr = itr_ck.begin();
        BOOST_TEST(*itr == std::string("c:"));
        BOOST_TEST(*++itr == std::string("foo"));
        BOOST_TEST(++itr == itr_ck.end());
        BOOST_TEST(*--itr == std::string("foo"));
        BOOST_TEST(*--itr == std::string("c:"));
        BOOST_TEST(itr == itr_begin);

        itr_ck = "c:foo/";
        itr_begin = itr = itr_ck.begin();
        BOOST_TEST(*itr == std::string("c:"));
        BOOST_TEST(*++itr == std::string("foo"));
        BOOST_TEST(*++itr == std::string(BOOST_FILESYSTEM_V3_TRAILING_DOT));
        BOOST_TEST(++itr == itr_ck.end());
        BOOST_TEST(*--itr == std::string(BOOST_FILESYSTEM_V3_TRAILING_DOT));
        BOOST_TEST(*--itr == std::string("foo"));
        BOOST_TEST(*--itr == std::string("c:"));
        BOOST_TEST(itr == itr_begin);

        itr_ck = path("c:");
        BOOST_TEST(*itr_ck.begin() == std::string("c:"));
        BOOST_TEST(next(itr_ck.begin()) == itr_ck.end());
        BOOST_TEST(prior(itr_ck.end()) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("c:"));

        itr_ck = path("c:/");
        BOOST_TEST(*itr_ck.begin() == std::string("c:"));
        BOOST_TEST(*next(itr_ck.begin()) == std::string("/"));
        BOOST_TEST(next(next(itr_ck.begin())) == itr_ck.end());
        BOOST_TEST(prior(prior(itr_ck.end())) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("/"));
        BOOST_TEST(*prior(prior(itr_ck.end())) == std::string("c:"));

        itr_ck = path("c:foo");
        BOOST_TEST(*itr_ck.begin() == std::string("c:"));
        BOOST_TEST(*next(itr_ck.begin()) == std::string("foo"));
        BOOST_TEST(next(next(itr_ck.begin())) == itr_ck.end());
        BOOST_TEST(prior(prior(itr_ck.end())) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("foo"));
        BOOST_TEST(*prior(prior(itr_ck.end())) == std::string("c:"));

        itr_ck = path("c:/foo");
        BOOST_TEST(*itr_ck.begin() == std::string("c:"));
        BOOST_TEST(*next(itr_ck.begin()) == std::string("/"));
        BOOST_TEST(*next(next(itr_ck.begin())) == std::string("foo"));
        BOOST_TEST(next(next(next(itr_ck.begin()))) == itr_ck.end());
        BOOST_TEST(prior(prior(prior(itr_ck.end()))) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("foo"));
        BOOST_TEST(*prior(prior(itr_ck.end())) == std::string("/"));
        BOOST_TEST(*prior(prior(prior(itr_ck.end()))) == std::string("c:"));

        itr_ck = path("//net");
        BOOST_TEST(*itr_ck.begin() == std::string("//net"));
        BOOST_TEST(next(itr_ck.begin()) == itr_ck.end());
        BOOST_TEST(prior(itr_ck.end()) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("//net"));

        itr_ck = path("//net/");
        PATH_TEST_EQ(itr_ck.begin()->string(), "//net");
        PATH_TEST_EQ(next(itr_ck.begin())->string(), "/");
        BOOST_TEST(next(next(itr_ck.begin())) == itr_ck.end());
        BOOST_TEST(prior(prior(itr_ck.end())) == itr_ck.begin());
        PATH_TEST_EQ(prior(itr_ck.end())->string(), "/");
        PATH_TEST_EQ(prior(prior(itr_ck.end()))->string(), "//net");

        itr_ck = path("//net/foo");
        BOOST_TEST(*itr_ck.begin() == std::string("//net"));
        BOOST_TEST(*next(itr_ck.begin()) == std::string("/"));
        BOOST_TEST(*next(next(itr_ck.begin())) == std::string("foo"));
        BOOST_TEST(next(next(next(itr_ck.begin()))) == itr_ck.end());
        BOOST_TEST(prior(prior(prior(itr_ck.end()))) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("foo"));
        BOOST_TEST(*prior(prior(itr_ck.end())) == std::string("/"));
        BOOST_TEST(*prior(prior(prior(itr_ck.end()))) == std::string("//net"));

        itr_ck = path("prn:");
        BOOST_TEST(*itr_ck.begin() == std::string("prn:"));
        BOOST_TEST(next(itr_ck.begin()) == itr_ck.end());
        BOOST_TEST(prior(itr_ck.end()) == itr_ck.begin());
        BOOST_TEST(*prior(itr_ck.end()) == std::string("prn:"));
    }
    else
    {
        itr_ck = "///";
        itr = itr_ck.begin();
        PATH_TEST_EQ(itr->string(), "/");
        BOOST_TEST(++itr == itr_ck.end());
    }
}

//  non_member_tests  ----------------------------------------------------------------//

void non_member_tests()
{
    std::cout << "non_member_tests..." << std::endl;

    // test non-member functions, particularly operator overloads

    path e, e2;
    std::string es, es2;
    char ecs[] = "";
    char ecs2[] = "";

    char acs[] = "a";
    std::string as(acs);
    path a(as);

    char acs2[] = "a";
    std::string as2(acs2);
    path a2(as2);

    char bcs[] = "b";
    std::string bs(bcs);
    path b(bs);

    // swap
    a.swap(b);
    BOOST_TEST(a.string() == "b");
    BOOST_TEST(b.string() == "a");
    fs::swap(a, b);
    BOOST_TEST(a.string() == "a");
    BOOST_TEST(b.string() == "b");

    // probe operator /
    PATH_TEST_EQ(path("") / ".", ".");
    PATH_TEST_EQ(path("") / "..", "..");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(path("/") / "/", "//");
    PATH_TEST_EQ(path("/") / "/foo", "//foo");
    PATH_TEST_EQ(path("/foo") / "/bar", "/foo/bar");
#else
    PATH_TEST_EQ(path("/") / "/", "/");
    PATH_TEST_EQ(path("/") / "/foo", "/foo");
    PATH_TEST_EQ(path("/foo") / "/bar", "/bar");
#endif

    if (platform == "Windows")
    {
        BOOST_TEST(path("foo\\bar") == "foo/bar");
        BOOST_TEST((b / a).native() == path("b\\a").native());
        BOOST_TEST((bs / a).native() == path("b\\a").native());
        BOOST_TEST((bcs / a).native() == path("b\\a").native());
        BOOST_TEST((b / as).native() == path("b\\a").native());
        BOOST_TEST((b / acs).native() == path("b\\a").native());
        PATH_TEST_EQ(path("a") / "b", "a\\b");
        PATH_TEST_EQ(path("foo") / path("bar"), "foo\\bar");                // path arg
        PATH_TEST_EQ(path("foo") / "bar", "foo\\bar");                      // const char* arg
        PATH_TEST_EQ(path("foo") / path("woo/bar").filename(), "foo\\bar"); // const std::string & arg
        PATH_TEST_EQ("foo" / path("bar"), "foo\\bar");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("..") / "", "..");
#else
        PATH_TEST_EQ(path("..") / "", "..\\");
#endif
        PATH_TEST_EQ(path("..") / "..", "..\\..");
        PATH_TEST_EQ(path("/") / "..", "/..");
        PATH_TEST_EQ(path("/..") / "..", "/..\\..");
        PATH_TEST_EQ(path("..") / "foo", "..\\foo");
        PATH_TEST_EQ(path("foo") / "..", "foo\\..");
        PATH_TEST_EQ(path("..") / "f", "..\\f");
        PATH_TEST_EQ(path("/..") / "f", "/..\\f");
        PATH_TEST_EQ(path("f") / "..", "f\\..");
        PATH_TEST_EQ(path("foo") / ".." / "..", "foo\\..\\..");
        PATH_TEST_EQ(path("foo") / ".." / ".." / "..", "foo\\..\\..\\..");
        PATH_TEST_EQ(path("f") / ".." / "b", "f\\..\\b");
        PATH_TEST_EQ(path("foo") / ".." / "bar", "foo\\..\\bar");
        PATH_TEST_EQ(path("foo") / "bar" / "..", "foo\\bar\\..");
        PATH_TEST_EQ(path("foo") / "bar" / ".." / "..", "foo\\bar\\..\\..");
        PATH_TEST_EQ(path("foo") / "bar" / ".." / "blah", "foo\\bar\\..\\blah");
        PATH_TEST_EQ(path("f") / "b" / "..", "f\\b\\..");
        PATH_TEST_EQ(path("f") / "b" / ".." / "a", "f\\b\\..\\a");
        PATH_TEST_EQ(path("foo") / "bar" / "blah" / ".." / "..", "foo\\bar\\blah\\..\\..");
        PATH_TEST_EQ(path("foo") / "bar" / "blah" / ".." / ".." / "bletch", "foo\\bar\\blah\\..\\..\\bletch");

        PATH_TEST_EQ(path(".") / "foo", ".\\foo");
        PATH_TEST_EQ(path(".") / "..", ".\\..");
        PATH_TEST_EQ(path("foo") / ".", "foo\\.");
        PATH_TEST_EQ(path("..") / ".", "..\\.");
        PATH_TEST_EQ(path(".") / ".", ".\\.");
        PATH_TEST_EQ(path(".") / "." / ".", ".\\.\\.");
        PATH_TEST_EQ(path(".") / "foo" / ".", ".\\foo\\.");
        PATH_TEST_EQ(path("foo") / "." / "bar", "foo\\.\\bar");
        PATH_TEST_EQ(path("foo") / "." / ".", "foo\\.\\.");
        PATH_TEST_EQ(path("foo") / "." / "..", "foo\\.\\..");
        PATH_TEST_EQ(path(".") / "." / "..", ".\\.\\..");
        PATH_TEST_EQ(path(".") / ".." / ".", ".\\..\\.");
        PATH_TEST_EQ(path("..") / "." / ".", "..\\.\\.");

#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("\\\\net1\\foo") / "\\\\net2\\bar", "\\\\net1\\foo\\\\net2\\bar");
        PATH_TEST_EQ(path("\\\\net1\\foo") / "\\bar", "\\\\net1\\foo\\bar");
        PATH_TEST_EQ(path("c:\\foo") / "d:\\bar", "c:\\foo\\d:\\bar");
        PATH_TEST_EQ(path("c:\\foo") / "\\bar", "c:\\foo\\bar");
        PATH_TEST_EQ(path("c:foo") / "\\bar", "c:foo\\bar");
#else
        PATH_TEST_EQ(path("\\\\net1\\foo") / "\\\\net2\\bar", "\\\\net2\\bar");
        PATH_TEST_EQ(path("\\\\net1\\foo") / "\\bar", "\\\\net1\\bar");
        PATH_TEST_EQ(path("c:\\foo") / "d:\\bar", "d:\\bar");
        PATH_TEST_EQ(path("c:\\foo") / "\\bar", "c:\\bar");
        PATH_TEST_EQ(path("c:foo") / "\\bar", "c:\\bar");
#endif
        PATH_TEST_EQ(path("c:foo") / "bar", "c:foo\\bar");
    }
    else // POSIX
    {
        PATH_TEST_EQ(b / a, "b/a");
        PATH_TEST_EQ(bs / a, "b/a");
        PATH_TEST_EQ(bcs / a, "b/a");
        PATH_TEST_EQ(b / as, "b/a");
        PATH_TEST_EQ(b / acs, "b/a");
        PATH_TEST_EQ(path("a") / "b", "a/b");
        PATH_TEST_EQ(path("") / "..", "..");
        PATH_TEST_EQ(path("foo") / path("bar"), "foo/bar");                // path arg
        PATH_TEST_EQ(path("foo") / "bar", "foo/bar");                      // const char* arg
        PATH_TEST_EQ(path("foo") / path("woo/bar").filename(), "foo/bar"); // const std::string & arg
        PATH_TEST_EQ("foo" / path("bar"), "foo/bar");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("..") / "", "..");
#else
        PATH_TEST_EQ(path("..") / "", "../");
#endif
        PATH_TEST_EQ(path("..") / "..", "../..");
        PATH_TEST_EQ(path("/") / "..", "/..");
        PATH_TEST_EQ(path("/..") / "..", "/../..");
        PATH_TEST_EQ(path("..") / "foo", "../foo");
        PATH_TEST_EQ(path("foo") / "..", "foo/..");
        PATH_TEST_EQ(path("..") / "f", "../f");
        PATH_TEST_EQ(path("/..") / "f", "/../f");
        PATH_TEST_EQ(path("f") / "..", "f/..");
        PATH_TEST_EQ(path("foo") / ".." / "..", "foo/../..");
        PATH_TEST_EQ(path("foo") / ".." / ".." / "..", "foo/../../..");
        PATH_TEST_EQ(path("f") / ".." / "b", "f/../b");
        PATH_TEST_EQ(path("foo") / ".." / "bar", "foo/../bar");
        PATH_TEST_EQ(path("foo") / "bar" / "..", "foo/bar/..");
        PATH_TEST_EQ(path("foo") / "bar" / ".." / "..", "foo/bar/../..");
        PATH_TEST_EQ(path("foo") / "bar" / ".." / "blah", "foo/bar/../blah");
        PATH_TEST_EQ(path("f") / "b" / "..", "f/b/..");
        PATH_TEST_EQ(path("f") / "b" / ".." / "a", "f/b/../a");
        PATH_TEST_EQ(path("foo") / "bar" / "blah" / ".." / "..", "foo/bar/blah/../..");
        PATH_TEST_EQ(path("foo") / "bar" / "blah" / ".." / ".." / "bletch", "foo/bar/blah/../../bletch");

        PATH_TEST_EQ(path(".") / "foo", "./foo");
        PATH_TEST_EQ(path(".") / "..", "./..");
        PATH_TEST_EQ(path("foo") / ".", "foo/.");
        PATH_TEST_EQ(path("..") / ".", "../.");
        PATH_TEST_EQ(path(".") / ".", "./.");
        PATH_TEST_EQ(path(".") / "." / ".", "././.");
        PATH_TEST_EQ(path(".") / "foo" / ".", "./foo/.");
        PATH_TEST_EQ(path("foo") / "." / "bar", "foo/./bar");
        PATH_TEST_EQ(path("foo") / "." / ".", "foo/./.");
        PATH_TEST_EQ(path("foo") / "." / "..", "foo/./..");
        PATH_TEST_EQ(path(".") / "." / "..", "././..");
        PATH_TEST_EQ(path(".") / ".." / ".", "./../.");
        PATH_TEST_EQ(path("..") / "." / ".", ".././.");

#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("//net1/foo") / "//net2/bar", "//net1/foo//net2/bar");
        PATH_TEST_EQ(path("//net1/foo") / "/bar", "//net1/foo/bar");
#else
        PATH_TEST_EQ(path("//net1/foo") / "//net2/bar", "//net2/bar");
        PATH_TEST_EQ(path("//net1/foo") / "/bar", "/bar");
#endif
        PATH_TEST_EQ(path("//net1/foo") / "bar", "//net1/foo/bar");
    }

    // probe operator <
    BOOST_TEST(!(e < e2));
    BOOST_TEST(!(es < e2));
    BOOST_TEST(!(ecs < e2));
    BOOST_TEST(!(e < es2));
    BOOST_TEST(!(e < ecs2));

    BOOST_TEST(e < a);
    BOOST_TEST(es < a);
    BOOST_TEST(ecs < a);
    BOOST_TEST(e < as);
    BOOST_TEST(e < acs);

    BOOST_TEST(a < b);
    BOOST_TEST(as < b);
    BOOST_TEST(acs < b);
    BOOST_TEST(a < bs);
    BOOST_TEST(a < bcs);

    BOOST_TEST(!(a < a2));
    BOOST_TEST(!(as < a2));
    BOOST_TEST(!(acs < a2));
    BOOST_TEST(!(a < as2));
    BOOST_TEST(!(a < acs2));

    // make sure basic_path overloads don't conflict with std::string overloads

    BOOST_TEST(!(as < as));
    BOOST_TEST(!(as < acs));
    BOOST_TEST(!(acs < as));

    // character set reality check before lexicographical tests
    BOOST_TEST(std::string("a.b") < std::string("a/b"));
    // verify compare is actually lexicographical
    BOOST_TEST(path("a/b") < path("a.b"));
    BOOST_TEST(path("a/b") == path("a///b"));
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(path("a/b/") == path("a/b/."));
#else
    BOOST_TEST(path("a/b/") != path("a/b/."));
#endif
    BOOST_TEST(path("a/b") != path("a/b/"));

    // make sure the derivative operators also work

    BOOST_TEST(b > a);
    BOOST_TEST(b > as);
    BOOST_TEST(b > acs);
    BOOST_TEST(bs > a);
    BOOST_TEST(bcs > a);

    BOOST_TEST(!(a2 > a));
    BOOST_TEST(!(a2 > as));
    BOOST_TEST(!(a2 > acs));
    BOOST_TEST(!(as2 > a));
    BOOST_TEST(!(acs2 > a));

    BOOST_TEST(a <= b);
    BOOST_TEST(as <= b);
    BOOST_TEST(acs <= b);
    BOOST_TEST(a <= bs);
    BOOST_TEST(a <= bcs);

    BOOST_TEST(a <= a2);
    BOOST_TEST(as <= a2);
    BOOST_TEST(acs <= a2);
    BOOST_TEST(a <= as2);
    BOOST_TEST(a <= acs2);

    BOOST_TEST(b >= a);
    BOOST_TEST(bs >= a);
    BOOST_TEST(bcs >= a);
    BOOST_TEST(b >= as);
    BOOST_TEST(b >= acs);

    BOOST_TEST(a2 >= a);
    BOOST_TEST(as2 >= a);
    BOOST_TEST(acs2 >= a);
    BOOST_TEST(a2 >= as);
    BOOST_TEST(a2 >= acs);

    //  operator == and != are implemented separately, so test separately

    path p101("fe/fi/fo/fum");
    path p102(p101);
    path p103("fe/fi/fo/fumm");
    BOOST_TEST(p101.string() != p103.string());

    // check each overload
    BOOST_TEST(p101 != p103);
    BOOST_TEST(p101 != p103.string());
    BOOST_TEST(p101 != p103.string().c_str());
    BOOST_TEST(p101.string() != p103);
    BOOST_TEST(p101.string().c_str() != p103);

    p103 = p102;
    BOOST_TEST(p101.string() == p103.string());

    // check each overload
    BOOST_TEST(p101 == p103);
    BOOST_TEST(p101 == p103.string());
    BOOST_TEST(p101 == p103.string().c_str());
    BOOST_TEST(p101.string() == p103);
    BOOST_TEST(p101.string().c_str() == p103);

    if (platform == "Windows")
    {
        std::cout << "  Windows relational tests..." << std::endl;
        path p10("c:\\file");
        path p11("c:/file");
        // check each overload
        BOOST_TEST(p10.generic_string() == p11.generic_string());
        BOOST_TEST(p10 == p11);
        BOOST_TEST(p10 == p11.string());
        BOOST_TEST(p10 == p11.string().c_str());
        BOOST_TEST(p10.string() == p11);
        BOOST_TEST(p10.string().c_str() == p11);
        BOOST_TEST(p10 == L"c:\\file");
        BOOST_TEST(p10 == L"c:/file");
        BOOST_TEST(p11 == L"c:\\file");
        BOOST_TEST(p11 == L"c:/file");
        BOOST_TEST(L"c:\\file" == p10);
        BOOST_TEST(L"c:/file" == p10);
        BOOST_TEST(L"c:\\file" == p11);
        BOOST_TEST(L"c:/file" == p11);

        BOOST_TEST(!(p10.generic_string() != p11.generic_string()));
        BOOST_TEST(!(p10 != p11));
        BOOST_TEST(!(p10 != p11.string()));
        BOOST_TEST(!(p10 != p11.string().c_str()));
        BOOST_TEST(!(p10.string() != p11));
        BOOST_TEST(!(p10.string().c_str() != p11));
        BOOST_TEST(!(p10 != L"c:\\file"));
        BOOST_TEST(!(p10 != L"c:/file"));
        BOOST_TEST(!(p11 != L"c:\\file"));
        BOOST_TEST(!(p11 != L"c:/file"));
        BOOST_TEST(!(L"c:\\file" != p10));
        BOOST_TEST(!(L"c:/file" != p10));
        BOOST_TEST(!(L"c:\\file" != p11));
        BOOST_TEST(!(L"c:/file" != p11));

        BOOST_TEST(!(p10.string() < p11.string()));
        BOOST_TEST(!(p10 < p11));
        BOOST_TEST(!(p10 < p11.string()));
        BOOST_TEST(!(p10 < p11.string().c_str()));
        BOOST_TEST(!(p10.string() < p11));
        BOOST_TEST(!(p10.string().c_str() < p11));
        BOOST_TEST(!(p10 < L"c:\\file"));
        BOOST_TEST(!(p10 < L"c:/file"));
        BOOST_TEST(!(p11 < L"c:\\file"));
        BOOST_TEST(!(p11 < L"c:/file"));
        BOOST_TEST(!(L"c:\\file" < p10));
        BOOST_TEST(!(L"c:/file" < p10));
        BOOST_TEST(!(L"c:\\file" < p11));
        BOOST_TEST(!(L"c:/file" < p11));

        BOOST_TEST(!(p10.generic_string() > p11.generic_string()));
        BOOST_TEST(!(p10 > p11));
        BOOST_TEST(!(p10 > p11.string()));
        BOOST_TEST(!(p10 > p11.string().c_str()));
        BOOST_TEST(!(p10.string() > p11));
        BOOST_TEST(!(p10.string().c_str() > p11));
        BOOST_TEST(!(p10 > L"c:\\file"));
        BOOST_TEST(!(p10 > L"c:/file"));
        BOOST_TEST(!(p11 > L"c:\\file"));
        BOOST_TEST(!(p11 > L"c:/file"));
        BOOST_TEST(!(L"c:\\file" > p10));
        BOOST_TEST(!(L"c:/file" > p10));
        BOOST_TEST(!(L"c:\\file" > p11));
        BOOST_TEST(!(L"c:/file" > p11));
    }

    // relative

    BOOST_TEST(fs::relative("/abc/def", "/abc") == path("def"));
    BOOST_TEST(fs::relative("abc/def", "abc") == path("def"));
    BOOST_TEST(fs::relative("/abc/xyz/def", "/abc") == path("xyz/def"));
    BOOST_TEST(fs::relative("abc/xyz/def", "abc") == path("xyz/def"));

    if (platform == "Windows")
    {
        std::cout << "  Windows relatie tests..." << std::endl;
        BOOST_TEST(fs::relative("\\abc\\xyz\\def", "/abc") == path("xyz/def"));
        std::cout << "    fs::relative(\"/abc/xyz/def\", \"/abc\") is "
                  << fs::relative("/abc/xyz/def", "/abc") << std::endl;
        BOOST_TEST(fs::relative("abc\\xyz\\def", "abc") == path("xyz/def"));
    }
}

//  query_and_decomposition_tests  ---------------------------------------------------//
//
//  remove_filename() is also tested here, because its specification depends on
//  a decomposition function.

void query_and_decomposition_tests()
{
    std::cout << "query_and_decomposition_tests..." << std::endl;

    // these are the examples given in reference docs, so check they work
    BOOST_TEST(path("/foo/bar.txt").parent_path() == "/foo");
    BOOST_TEST(path("/foo/bar").parent_path() == "/foo");
    BOOST_TEST(path("/foo/bar/").parent_path() == "/foo/bar");
    BOOST_TEST(path("/").parent_path() == "");
    BOOST_TEST(path(".").parent_path() == "");
    BOOST_TEST(path("..").parent_path() == "");
    BOOST_TEST(path("/foo/bar.txt").filename() == "bar.txt");
    BOOST_TEST(path("/foo/bar").filename() == "bar");
    BOOST_TEST(path("/foo/bar/").filename() == BOOST_FILESYSTEM_V3_TRAILING_DOT);
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(path("/").filename() == "/");
#else
    BOOST_TEST(path("/").filename() == "");
#endif
    BOOST_TEST(path(".").filename() == ".");
    BOOST_TEST(path("..").filename() == "..");

    // stem() tests not otherwise covered
    BOOST_TEST(path(".").stem() == ".");
    BOOST_TEST(path("..").stem() == "..");
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(path(".a").stem() == "");
#else
    BOOST_TEST(path(".a").stem() == ".a");
#endif
    BOOST_TEST(path("b").stem() == "b");
    BOOST_TEST(path("a/b.txt").stem() == "b");
    BOOST_TEST(path("a/b.").stem() == "b");
    BOOST_TEST(path("a.b.c").stem() == "a.b");
    BOOST_TEST(path("a.b.c.").stem() == "a.b.c");

    // extension() tests not otherwise covered
    BOOST_TEST(path(".").extension() == "");
    BOOST_TEST(path("..").extension() == "");
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(path(".a").extension() == ".a");
#else
    BOOST_TEST(path(".a").extension() == "");
#endif
    BOOST_TEST(path("a/b").extension() == "");
    BOOST_TEST(path("a.b/c").extension() == "");
    BOOST_TEST(path("a/b.txt").extension() == ".txt");
    BOOST_TEST(path("a/b.").extension() == ".");
    BOOST_TEST(path("a.b.c").extension() == ".c");
    BOOST_TEST(path("a.b.c.").extension() == ".");
    BOOST_TEST(path("a/").extension() == "");

    // main q & d test sequence
    path p;
    path q;

    p = q = "";
    BOOST_TEST(p.relative_path().string() == "");
    BOOST_TEST(p.parent_path().string() == "");
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == "");
    BOOST_TEST(p.stem() == "");
    BOOST_TEST(p.extension() == "");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "");
    BOOST_TEST(p.root_path().string() == "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(!p.has_relative_path());
    BOOST_TEST(!p.has_filename());
    BOOST_TEST(!p.has_stem());
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "/";
    BOOST_TEST(p.relative_path().string() == "");
    BOOST_TEST(p.parent_path().string() == "");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == "/");
    BOOST_TEST(p.stem() == "/");
#else
    PATH_TEST_EQ(q.remove_filename().string(), p.string());
    BOOST_TEST(p.filename() == "");
    BOOST_TEST(p.stem() == "");
#endif
    BOOST_TEST(p.extension() == "");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "/");
    BOOST_TEST(p.root_path().string() == "/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
#else
    BOOST_TEST(!p.has_filename());
    BOOST_TEST(!p.has_stem());
#endif
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    if (platform == "POSIX")
        BOOST_TEST(p.is_absolute());
    else
        BOOST_TEST(!p.is_absolute());

    p = q = "//";
    PATH_TEST_EQ(p.relative_path().string(), "");
    PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    PATH_TEST_EQ(p.filename(), "//");
    PATH_TEST_EQ(p.stem(), "//");
#else
    PATH_TEST_EQ(q.remove_filename().string(), p.string());
    PATH_TEST_EQ(p.filename(), "");
    PATH_TEST_EQ(p.stem(), "");
#endif
    PATH_TEST_EQ(p.extension(), "");
    PATH_TEST_EQ(p.root_name(), "//");
    PATH_TEST_EQ(p.root_directory(), "");
    PATH_TEST_EQ(p.root_path().string(), "//");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
#else
    BOOST_TEST(!p.has_filename());
    BOOST_TEST(!p.has_stem());
#endif
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "///";
    PATH_TEST_EQ(p.relative_path().string(), "");
    PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    PATH_TEST_EQ(p.filename(), "/");
    PATH_TEST_EQ(p.stem(), "/");
#else
    PATH_TEST_EQ(q.remove_filename().string(), p.string());
    PATH_TEST_EQ(p.filename(), "");
    PATH_TEST_EQ(p.stem(), "");
#endif
    PATH_TEST_EQ(p.extension(), "");
    PATH_TEST_EQ(p.root_name(), "");
    PATH_TEST_EQ(p.root_directory(), "/");
    PATH_TEST_EQ(p.root_path().string(), "/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
#else
    BOOST_TEST(!p.has_filename());
    BOOST_TEST(!p.has_stem());
#endif
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    if (platform == "POSIX")
        BOOST_TEST(p.is_absolute());
    else
        BOOST_TEST(!p.is_absolute());

    p = q = ".";
    BOOST_TEST(p.relative_path().string() == ".");
    BOOST_TEST(p.parent_path().string() == "");
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == ".");
    BOOST_TEST(p.stem() == ".");
    BOOST_TEST(p.extension() == "");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "");
    BOOST_TEST(p.root_path().string() == "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "..";
    BOOST_TEST(p.relative_path().string() == "..");
    BOOST_TEST(p.parent_path().string() == "");
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == "..");
    BOOST_TEST(p.stem() == "..");
    BOOST_TEST(p.extension() == "");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "");
    BOOST_TEST(p.root_path().string() == "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "foo";
    BOOST_TEST(p.relative_path().string() == "foo");
    BOOST_TEST(p.parent_path().string() == "");
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == "foo");
    BOOST_TEST(p.stem() == "foo");
    BOOST_TEST(p.extension() == "");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "");
    BOOST_TEST(p.root_path().string() == "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "/foo";
    PATH_TEST_EQ(p.relative_path().string(), "foo");
    PATH_TEST_EQ(p.parent_path().string(), "/");
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    PATH_TEST_EQ(p.filename(), "foo");
    PATH_TEST_EQ(p.stem(), "foo");
    PATH_TEST_EQ(p.extension(), "");
    PATH_TEST_EQ(p.root_name(), "");
    PATH_TEST_EQ(p.root_directory(), "/");
    PATH_TEST_EQ(p.root_path().string(), "/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(p.has_parent_path());
    if (platform == "POSIX")
        BOOST_TEST(p.is_absolute());
    else
        BOOST_TEST(!p.is_absolute());

    p = q = "/foo/";
    PATH_TEST_EQ(p.relative_path().string(), "foo/");
    PATH_TEST_EQ(p.parent_path().string(), "/foo");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), p.string());
#endif
    PATH_TEST_EQ(p.filename(), BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(p.stem(), BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(p.extension(), "");
    PATH_TEST_EQ(p.root_name(), "");
    PATH_TEST_EQ(p.root_directory(), "/");
    PATH_TEST_EQ(p.root_path().string(), "/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
#else
    BOOST_TEST(!p.has_filename());
    BOOST_TEST(!p.has_stem());
#endif
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(p.has_parent_path());
    if (platform == "POSIX")
        BOOST_TEST(p.is_absolute());
    else
        BOOST_TEST(!p.is_absolute());

    p = q = "///foo";
    PATH_TEST_EQ(p.relative_path().string(), "foo");
    PATH_TEST_EQ(p.parent_path().string(), "/");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), "///");
#endif
    PATH_TEST_EQ(p.filename(), "foo");
    PATH_TEST_EQ(p.root_name(), "");
    PATH_TEST_EQ(p.root_directory(), "/");
    PATH_TEST_EQ(p.root_path().string(), "/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_parent_path());
    if (platform == "POSIX")
        BOOST_TEST(p.is_absolute());
    else
        BOOST_TEST(!p.is_absolute());

    p = q = "foo/bar";
    BOOST_TEST(p.relative_path().string() == "foo/bar");
    BOOST_TEST(p.parent_path().string() == "foo");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), "foo/");
#endif
    BOOST_TEST(p.filename() == "bar");
    BOOST_TEST(p.stem() == "bar");
    BOOST_TEST(p.extension() == "");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "");
    BOOST_TEST(p.root_path().string() == "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_stem());
    BOOST_TEST(!p.has_extension());
    BOOST_TEST(p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "../foo";
    BOOST_TEST(p.relative_path().string() == "../foo");
    BOOST_TEST(p.parent_path().string() == "..");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), "../");
#endif
    BOOST_TEST(p.filename() == "foo");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "");
    BOOST_TEST(p.root_path().string() == "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "..///foo";
    PATH_TEST_EQ(p.relative_path().string(), "..///foo");
    PATH_TEST_EQ(p.parent_path().string(), "..");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), "..///");
#endif
    PATH_TEST_EQ(p.filename(), "foo");
    PATH_TEST_EQ(p.root_name(), "");
    PATH_TEST_EQ(p.root_directory(), "");
    PATH_TEST_EQ(p.root_path().string(), "");
    BOOST_TEST(!p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = "/foo/bar";
    BOOST_TEST(p.relative_path().string() == "foo/bar");
    BOOST_TEST(p.parent_path().string() == "/foo");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), "/foo/");
#endif
    BOOST_TEST(p.filename() == "bar");
    BOOST_TEST(p.root_name() == "");
    BOOST_TEST(p.root_directory() == "/");
    BOOST_TEST(p.root_path().string() == "/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(!p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_parent_path());
    if (platform == "POSIX")
        BOOST_TEST(p.is_absolute());
    else
        BOOST_TEST(!p.is_absolute());

    // Both POSIX and Windows allow two leading slashs
    // (POSIX meaning is implementation defined)
    PATH_TEST_EQ(path("//resource"), "//resource");
    PATH_TEST_EQ(path("//resource/"), "//resource/");
    PATH_TEST_EQ(path("//resource/foo"), "//resource/foo");

    p = q = path("//net");
    PATH_TEST_EQ(p.string(), "//net");
    PATH_TEST_EQ(p.relative_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), p.string());
#endif
    PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(p.filename(), "//net");
#else
    PATH_TEST_EQ(p.filename(), "");
#endif
    PATH_TEST_EQ(p.root_name(), "//net");
    PATH_TEST_EQ(p.root_directory(), "");
    PATH_TEST_EQ(p.root_path().string(), "//net");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(p.has_root_name());
    BOOST_TEST(!p.has_root_directory());
    BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(p.has_filename());
#else
    BOOST_TEST(!p.has_filename());
#endif
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(!p.is_absolute());

    p = q = path("//net/");
    BOOST_TEST(p.relative_path().string() == "");
    BOOST_TEST(p.parent_path().string() == "//net");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == "/");
#else
    PATH_TEST_EQ(q.remove_filename().string(), p.string());
    BOOST_TEST(p.filename() == "");
#endif
    BOOST_TEST(p.root_name() == "//net");
    BOOST_TEST(p.root_directory() == "/");
    BOOST_TEST(p.root_path().string() == "//net/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
    BOOST_TEST(p.has_filename());
#else
    BOOST_TEST(!p.has_filename());
#endif
    BOOST_TEST(p.has_parent_path());
    BOOST_TEST(p.is_absolute());

    p = q = path("//net/foo");
    BOOST_TEST(p.relative_path().string() == "foo");
    BOOST_TEST(p.parent_path().string() == "//net/");
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
    BOOST_TEST(p.filename() == "foo");
    BOOST_TEST(p.root_name() == "//net");
    BOOST_TEST(p.root_directory() == "/");
    BOOST_TEST(p.root_path().string() == "//net/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_parent_path());
    BOOST_TEST(p.is_absolute());

    p = q = path("//net///foo");
    PATH_TEST_EQ(p.relative_path().string(), "foo");
    PATH_TEST_EQ(p.parent_path().string(), "//net/");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
    PATH_TEST_EQ(q.remove_filename().string(), "//net///");
#endif
    PATH_TEST_EQ(p.filename(), "foo");
    PATH_TEST_EQ(p.root_name(), "//net");
    PATH_TEST_EQ(p.root_directory(), "/");
    PATH_TEST_EQ(p.root_path().string(), "//net/");
    BOOST_TEST(p.has_root_path());
    BOOST_TEST(p.has_root_name());
    BOOST_TEST(p.has_root_directory());
    BOOST_TEST(p.has_relative_path());
    BOOST_TEST(p.has_filename());
    BOOST_TEST(p.has_parent_path());
    BOOST_TEST(p.is_absolute());

    //  ticket 2739, infinite recursion leading to stack overflow, was caused
    //  by failure to handle this case correctly on Windows.
    p = path(":");
    PATH_TEST_EQ(p.parent_path().string(), "");
    PATH_TEST_EQ(p.filename(), ":");
    BOOST_TEST(!p.has_parent_path());
    BOOST_TEST(p.has_filename());


    //  Windows specific tests
    if (platform == "Windows")
    {
        p = q = path("\\\\?\\");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\\\?\\");
        PATH_TEST_EQ(p.stem(), "\\\\?\\");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
        PATH_TEST_EQ(p.stem(), "");
#endif
        PATH_TEST_EQ(p.extension(), "");
        PATH_TEST_EQ(p.root_name(), "\\\\?\\");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\\\?\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_stem());
#else
        BOOST_TEST(!p.has_filename());
        BOOST_TEST(!p.has_stem());
#endif
        BOOST_TEST(!p.has_extension());
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\\\.\\");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\\\.\\");
        PATH_TEST_EQ(p.stem(), "\\\\");
        PATH_TEST_EQ(p.extension(), ".\\");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
        PATH_TEST_EQ(p.stem(), "");
        PATH_TEST_EQ(p.extension(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\\\.\\");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\\\.\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_stem());
        BOOST_TEST(p.has_extension());
#else
        BOOST_TEST(!p.has_filename());
        BOOST_TEST(!p.has_stem());
        BOOST_TEST(!p.has_extension());
#endif
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\??\\");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\??\\");
        PATH_TEST_EQ(p.stem(), "\\??\\");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
        PATH_TEST_EQ(p.stem(), "");
#endif
        PATH_TEST_EQ(p.extension(), "");
        PATH_TEST_EQ(p.root_name(), "\\??\\");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\??\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_stem());
#else
        BOOST_TEST(!p.has_filename());
        BOOST_TEST(!p.has_stem());
#endif
        BOOST_TEST(!p.has_extension());
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("c:");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "c:");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\\\?\\c:");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\\\?\\c:");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\\\?\\c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\\\?\\c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\\\.\\c:");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\\\.\\c:");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\\\.\\c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\\\.\\c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\??\\c:");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\??\\c:");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\??\\c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\??\\c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("c:foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "c:");
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\\\?\\c:foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\?\\c:");
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "\\\\?\\c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\\\?\\c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\\\.\\c:foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\.\\c:");
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "\\\\.\\c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\\\.\\c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\??\\c:foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "\\??\\c:");
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "\\??\\c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "\\??\\c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("c:/");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "c:");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "/");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "/");
        PATH_TEST_EQ(p.root_path().string(), "c:/");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("\\\\?\\c:\\");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\?\\c:");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\\\?\\c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\\\?\\c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("\\\\.\\c:\\");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\.\\c:");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\\\.\\c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\\\.\\c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("\\??\\c:\\");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "\\??\\c:");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "\\");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "\\??\\c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\??\\c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("c:..");
        PATH_TEST_EQ(p.relative_path().string(), "..");
        PATH_TEST_EQ(p.parent_path().string(), "c:");
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "..");
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "c:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("c:/foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "c:/");
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "/");
        PATH_TEST_EQ(p.root_path().string(), "c:/");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("c://foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "c:/");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
        PATH_TEST_EQ(q.remove_filename().string(), "c://");
#endif
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "/");
        PATH_TEST_EQ(p.root_path().string(), "c:/");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("c:\\foo\\bar");
        PATH_TEST_EQ(p.relative_path().string(), "foo\\bar");
        PATH_TEST_EQ(p.parent_path().string(), "c:\\foo");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
        PATH_TEST_EQ(q.remove_filename().string(), "c:\\foo\\");
#endif
        PATH_TEST_EQ(p.filename(), "bar");
        PATH_TEST_EQ(p.root_name(), "c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("\\\\?\\c:\\foo\\bar");
        PATH_TEST_EQ(p.relative_path().string(), "foo\\bar");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\?\\c:\\foo");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
        PATH_TEST_EQ(q.remove_filename().string(), "\\\\?\\c:\\foo\\");
#endif
        PATH_TEST_EQ(p.filename(), "bar");
        PATH_TEST_EQ(p.root_name(), "\\\\?\\c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\\\?\\c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("\\\\.\\c:\\foo\\bar");
        PATH_TEST_EQ(p.relative_path().string(), "foo\\bar");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\.\\c:\\foo");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
        PATH_TEST_EQ(q.remove_filename().string(), "\\\\.\\c:\\foo\\");
#endif
        PATH_TEST_EQ(p.filename(), "bar");
        PATH_TEST_EQ(p.root_name(), "\\\\.\\c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\\\.\\c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("\\??\\c:\\foo\\bar");
        PATH_TEST_EQ(p.relative_path().string(), "foo\\bar");
        PATH_TEST_EQ(p.parent_path().string(), "\\??\\c:\\foo");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
        PATH_TEST_EQ(q.remove_filename().string(), "\\??\\c:\\foo\\");
#endif
        PATH_TEST_EQ(p.filename(), "bar");
        PATH_TEST_EQ(p.root_name(), "\\??\\c:");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\??\\c:\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());

        p = q = path("prn:");
        PATH_TEST_EQ(p.relative_path().string(), "");
        PATH_TEST_EQ(p.parent_path().string(), "");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
        PATH_TEST_EQ(p.filename(), "prn:");
#else
        PATH_TEST_EQ(q.remove_filename().string(), p.string());
        PATH_TEST_EQ(p.filename(), "");
#endif
        PATH_TEST_EQ(p.root_name(), "prn:");
        PATH_TEST_EQ(p.root_directory(), "");
        PATH_TEST_EQ(p.root_path().string(), "prn:");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(!p.has_root_directory());
        BOOST_TEST(!p.has_relative_path());
#if BOOST_FILESYSTEM_VERSION == 3
        BOOST_TEST(p.has_filename());
#else
        BOOST_TEST(!p.has_filename());
#endif
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(!p.is_absolute());

        p = q = path("\\\\net\\\\\\foo");
        PATH_TEST_EQ(p.relative_path().string(), "foo");
        PATH_TEST_EQ(p.parent_path().string(), "\\\\net\\");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(q.remove_filename().string(), p.parent_path().string());
#else
        PATH_TEST_EQ(q.remove_filename().string(), "\\\\net\\\\\\");
#endif
        PATH_TEST_EQ(p.filename(), "foo");
        PATH_TEST_EQ(p.root_name(), "\\\\net");
        PATH_TEST_EQ(p.root_directory(), "\\");
        PATH_TEST_EQ(p.root_path().string(), "\\\\net\\");
        BOOST_TEST(p.has_root_path());
        BOOST_TEST(p.has_root_name());
        BOOST_TEST(p.has_root_directory());
        BOOST_TEST(p.has_relative_path());
        BOOST_TEST(p.has_filename());
        BOOST_TEST(p.has_parent_path());
        BOOST_TEST(p.is_absolute());
    } // Windows

    else
    { // POSIX
        p = path("c:");
        PATH_TEST_EQ(p.parent_path().string(), "");
        PATH_TEST_EQ(p.filename(), "c:");
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(p.has_filename());

        p = path("cc:");
        PATH_TEST_EQ(p.parent_path().string(), "");
        PATH_TEST_EQ(p.filename(), "cc:");
        BOOST_TEST(!p.has_parent_path());
        BOOST_TEST(p.has_filename());

        PATH_TEST_EQ(path("/foo/bar/"), "/foo/bar/");
        PATH_TEST_EQ(path("//foo//bar//"), "//foo//bar//");
        PATH_TEST_EQ(path("///foo///bar///"), "///foo///bar///");

        p = path("/usr/local/bin:/usr/bin:/bin");
        BOOST_TEST(p.string() == "/usr/local/bin:/usr/bin:/bin");
    } // POSIX
}

//  composition_tests  ----------------------------------------------------------------//

void composition_tests()
{
    std::cout << "composition_tests..." << std::endl;
}

//  construction_tests  ---------------------------------------------------------------//

void construction_tests()
{
    std::cout << "construction_tests..." << std::endl;

    PATH_TEST_EQ("", "");

    PATH_TEST_EQ("foo", "foo");
    PATH_TEST_EQ("f", "f");

    PATH_TEST_EQ("foo/", "foo/");
    PATH_TEST_EQ("f/", "f/");
    PATH_TEST_EQ("foo/..", "foo/..");
    PATH_TEST_EQ("foo/../", "foo/../");
    PATH_TEST_EQ("foo/bar/../..", "foo/bar/../..");
    PATH_TEST_EQ("foo/bar/../../", "foo/bar/../../");
    PATH_TEST_EQ("/", "/");
    PATH_TEST_EQ("/f", "/f");

    PATH_TEST_EQ("/foo", "/foo");
    PATH_TEST_EQ("/foo/bar/", "/foo/bar/");
    PATH_TEST_EQ("//foo//bar//", "//foo//bar//");
    PATH_TEST_EQ("///foo///bar///", "///foo///bar///");
    PATH_TEST_EQ("\\/foo\\/bar\\/", "\\/foo\\/bar\\/");
    PATH_TEST_EQ("\\//foo\\//bar\\//", "\\//foo\\//bar\\//");

    if (platform == "Windows")
    {
        PATH_TEST_EQ(path("c:") / "foo", "c:foo");
        PATH_TEST_EQ(path("c:") / "/foo", "c:/foo");

        PATH_TEST_EQ("\\foo\\bar\\", "\\foo\\bar\\");
        PATH_TEST_EQ("\\\\foo\\\\bar\\\\", "\\\\foo\\\\bar\\\\");
        PATH_TEST_EQ("\\\\\\foo\\\\\\bar\\\\\\", "\\\\\\foo\\\\\\bar\\\\\\");

        PATH_TEST_EQ("\\", "\\");
        PATH_TEST_EQ("\\f", "\\f");
        PATH_TEST_EQ("\\foo", "\\foo");
        PATH_TEST_EQ("foo\\bar", "foo\\bar");
        PATH_TEST_EQ("foo bar", "foo bar");
        PATH_TEST_EQ("c:", "c:");
        PATH_TEST_EQ("c:/", "c:/");
        PATH_TEST_EQ("c:.", "c:.");
        PATH_TEST_EQ("c:./foo", "c:./foo");
        PATH_TEST_EQ("c:.\\foo", "c:.\\foo");
        PATH_TEST_EQ("c:..", "c:..");
        PATH_TEST_EQ("c:/.", "c:/.");
        PATH_TEST_EQ("c:/..", "c:/..");
        PATH_TEST_EQ("c:/../", "c:/../");
        PATH_TEST_EQ("c:\\..\\", "c:\\..\\");
        PATH_TEST_EQ("c:/../..", "c:/../..");
        PATH_TEST_EQ("c:/../foo", "c:/../foo");
        PATH_TEST_EQ("c:\\..\\foo", "c:\\..\\foo");
        PATH_TEST_EQ("c:../foo", "c:../foo");
        PATH_TEST_EQ("c:..\\foo", "c:..\\foo");
        PATH_TEST_EQ("c:/../../foo", "c:/../../foo");
        PATH_TEST_EQ("c:\\..\\..\\foo", "c:\\..\\..\\foo");
        PATH_TEST_EQ("c:foo/..", "c:foo/..");
        PATH_TEST_EQ("c:/foo/..", "c:/foo/..");
        PATH_TEST_EQ("c:/..foo", "c:/..foo");
        PATH_TEST_EQ("c:foo", "c:foo");
        PATH_TEST_EQ("c:/foo", "c:/foo");
        PATH_TEST_EQ("\\\\netname", "\\\\netname");
        PATH_TEST_EQ("\\\\netname\\", "\\\\netname\\");
        PATH_TEST_EQ("\\\\netname\\foo", "\\\\netname\\foo");
        PATH_TEST_EQ("c:/foo", "c:/foo");
        PATH_TEST_EQ("prn:", "prn:");
    }
    else
    {
    }

    PATH_TEST_EQ("foo/bar", "foo/bar");
    PATH_TEST_EQ("a/b", "a/b"); // probe for length effects
    PATH_TEST_EQ("..", "..");
    PATH_TEST_EQ("../..", "../..");
    PATH_TEST_EQ("/..", "/..");
    PATH_TEST_EQ("/../..", "/../..");
    PATH_TEST_EQ("../foo", "../foo");
    PATH_TEST_EQ("foo/..", "foo/..");
    PATH_TEST_EQ("foo/..bar", "foo/..bar");
    PATH_TEST_EQ("../f", "../f");
    PATH_TEST_EQ("/../f", "/../f");
    PATH_TEST_EQ("f/..", "f/..");
    PATH_TEST_EQ("foo/../..", "foo/../..");
    PATH_TEST_EQ("foo/../../..", "foo/../../..");
    PATH_TEST_EQ("foo/../bar", "foo/../bar");
    PATH_TEST_EQ("foo/bar/..", "foo/bar/..");
    PATH_TEST_EQ("foo/bar/../..", "foo/bar/../..");
    PATH_TEST_EQ("foo/bar/../blah", "foo/bar/../blah");
    PATH_TEST_EQ("f/../b", "f/../b");
    PATH_TEST_EQ("f/b/..", "f/b/..");
    PATH_TEST_EQ("f/b/../a", "f/b/../a");
    PATH_TEST_EQ("foo/bar/blah/../..", "foo/bar/blah/../..");
    PATH_TEST_EQ("foo/bar/blah/../../bletch", "foo/bar/blah/../../bletch");
    PATH_TEST_EQ("...", "...");
    PATH_TEST_EQ("....", "....");
    PATH_TEST_EQ("foo/...", "foo/...");
    PATH_TEST_EQ("abc.", "abc.");
    PATH_TEST_EQ("abc..", "abc..");
    PATH_TEST_EQ("foo/abc.", "foo/abc.");
    PATH_TEST_EQ("foo/abc..", "foo/abc..");

    PATH_TEST_EQ(".abc", ".abc");
    PATH_TEST_EQ("a.c", "a.c");
    PATH_TEST_EQ("..abc", "..abc");
    PATH_TEST_EQ("a..c", "a..c");
    PATH_TEST_EQ("foo/.abc", "foo/.abc");
    PATH_TEST_EQ("foo/a.c", "foo/a.c");
    PATH_TEST_EQ("foo/..abc", "foo/..abc");
    PATH_TEST_EQ("foo/a..c", "foo/a..c");

    PATH_TEST_EQ(".", ".");
    PATH_TEST_EQ("./foo", "./foo");
    PATH_TEST_EQ("./..", "./..");
    PATH_TEST_EQ("./../foo", "./../foo");
    PATH_TEST_EQ("foo/.", "foo/.");
    PATH_TEST_EQ("../.", "../.");
    PATH_TEST_EQ("./.", "./.");
    PATH_TEST_EQ("././.", "././.");
    PATH_TEST_EQ("./foo/.", "./foo/.");
    PATH_TEST_EQ("foo/./bar", "foo/./bar");
    PATH_TEST_EQ("foo/./.", "foo/./.");
    PATH_TEST_EQ("foo/./..", "foo/./..");
    PATH_TEST_EQ("foo/./../bar", "foo/./../bar");
    PATH_TEST_EQ("foo/../.", "foo/../.");
    PATH_TEST_EQ("././..", "././..");
    PATH_TEST_EQ("./../.", "./../.");
    PATH_TEST_EQ(".././.", ".././.");

    PATH_TEST_EQ(derived_from_path("foo"), "foo");
    PATH_TEST_EQ(convertible_to_path("foo"), "foo");
    PATH_TEST_EQ(fs::path(pcustom_string("foo")), "foo");
    PATH_TEST_EQ(mars_boost::string_view("foo"), "foo");
#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
    PATH_TEST_EQ(std::string_view("foo"), "foo");
#endif

    // Check that path constructors don't cause ambiguity for to_string calls
    // https://github.com/boostorg/filesystem/issues/273
    custom_string c("test");
    BOOST_TEST_EQ(to_string(c), std::string("test"));
}

//  append_tests  --------------------------------------------------------------------//

#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
#define APPEND_TEST_STD_STRING_VIEW(appnd, expected)\
        path p6(p);\
        p6 /= std::string_view(appnd);\
        PATH_TEST_EQ(p6, expected);
#else
#define APPEND_TEST_STD_STRING_VIEW(appnd, expected)
#endif

#define APPEND_TEST(pth, appnd, expected)\
    {\
        const path p(pth);\
        const std::string s(appnd);\
        PATH_TEST_EQ(p / appnd, expected);\
        PATH_TEST_EQ((p / path(s)).string(), expected);\
        PATH_TEST_EQ((p / s.c_str()).string(), expected);\
        PATH_TEST_EQ((p / s).string(), expected);\
        path p1(p);\
        p1 /= appnd;\
        PATH_TEST_EQ(p1, expected);\
        path p2(p);\
        p2 /= derived_from_path(appnd);\
        PATH_TEST_EQ(p2, expected);\
        path p3(p);\
        p3 /= convertible_to_path(appnd);\
        PATH_TEST_EQ(p3, expected);\
        path p4(p);\
        p4 /= pcustom_string(appnd);\
        PATH_TEST_EQ(p4, expected);\
        path p5(p);\
        p5 /= mars_boost::string_view(appnd);\
        PATH_TEST_EQ(p5, expected);\
        APPEND_TEST_STD_STRING_VIEW(appnd, expected)\
        path p7(p);\
        p7.append(s.begin(), s.end());\
        PATH_TEST_EQ(p7.string(), expected);\
    }

void append_tests()
{
    std::cout << "append_tests..." << std::endl;

    // There are many control paths to be exercised, since empty paths and arguments,
    // paths with trailing separators, arguments with leading separators, with or without
    // other characters being present, are all separate cases that need to be tested.
    // Furthermore, some of the code to be tested is specific to argument categories,
    // so that results in further permutations to be tested.

    //// code to generate test cases
    ////
    //// expected results must be checked by hand
    //// "foo\bar" expected result must be edited by hand and moved for Windows/POSIX
    ////
    //const char* x[]    = { "", "/", "foo", "foo/" };
    //const char* y[] = { "", "/", "bar", "/bar" };

    //for (int i = 0; i < sizeof(x)/sizeof(char*); ++i)
    //  for (int j = 0; j < sizeof(y)/sizeof(char*); ++j)
    //  {
    //    std::cout << "    APPEND_TEST(\"" << x[i] << "\", \"" << y[j] << "\", \""
    //              << path(x[i]) / y[j] << "\");\n";
    //  }

    APPEND_TEST("", "", "");
    APPEND_TEST("", "/", "/");
    APPEND_TEST("", "bar", "bar");
    APPEND_TEST("", "/bar", "/bar");

    APPEND_TEST("/", "", "/");
#if BOOST_FILESYSTEM_VERSION == 3
    APPEND_TEST("/", "/", "//");
#else
    APPEND_TEST("/", "/", "/");
#endif
    APPEND_TEST("/", "bar", "/bar");
#if BOOST_FILESYSTEM_VERSION == 3
    APPEND_TEST("/", "/bar", "//bar");
#else
    APPEND_TEST("/", "/bar", "/bar");
#endif

#if BOOST_FILESYSTEM_VERSION == 3
    APPEND_TEST("foo", "/", "foo/");
#else
    APPEND_TEST("foo", "/", "/");
#endif
#if BOOST_FILESYSTEM_VERSION == 3
    APPEND_TEST("foo", "/bar", "foo/bar");
#else
    APPEND_TEST("foo", "/bar", "/bar");
#endif

    APPEND_TEST("foo/", "", "foo/");
#if BOOST_FILESYSTEM_VERSION == 3
    APPEND_TEST("foo/", "/", "foo//");
#else
    APPEND_TEST("foo/", "/", "/");
#endif
    APPEND_TEST("foo/", "bar", "foo/bar");

    if (platform == "Windows")
    {
#if BOOST_FILESYSTEM_VERSION == 3
        APPEND_TEST("foo", "", "foo");
#else
        APPEND_TEST("foo", "", "foo\\");
#endif
        APPEND_TEST("foo", "bar", "foo\\bar");

#if BOOST_FILESYSTEM_VERSION == 3
        APPEND_TEST("foo\\", "\\bar", "foo\\\\bar");
#else
        APPEND_TEST("foo\\", "\\bar", "\\bar");
#endif

        // hand created test case specific to Windows
        APPEND_TEST("c:", "bar", "c:bar");
    }
    else
    {
#if BOOST_FILESYSTEM_VERSION == 3
        APPEND_TEST("foo", "", "foo");
#else
        APPEND_TEST("foo", "", "foo/");
#endif
        APPEND_TEST("foo", "bar", "foo/bar");
    }

    // ticket #6819
    union
    {
        char a[1];
        char b[3];
    } u;

    u.b[0] = 'a';
    u.b[1] = 'b';
    u.b[2] = '\0';

    path p6819;
    p6819 /= u.a;
    BOOST_TEST_EQ(p6819, path("ab"));
}

//  concat_tests  --------------------------------------------------------------------//

#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
#define CONCAT_TEST_STD_STRING_VIEW(appnd, expected)\
        path p9(p);\
        p9 += std::string_view(appnd);\
        PATH_TEST_EQ(p9, expected);
#else
#define CONCAT_TEST_STD_STRING_VIEW(appnd, expected)
#endif

#define CONCAT_TEST(pth, appnd, expected)\
    {\
        const path p(pth);\
        const std::string s(appnd);\
        path p1(p);\
        p1 += appnd;\
        PATH_TEST_EQ(p1.string(), expected);\
        path p2(p);\
        p2 += path(appnd);\
        PATH_TEST_EQ(p2.string(), expected);\
        path p3(p);\
        p3 += s;\
        PATH_TEST_EQ(p3.string(), expected);\
        path p4(p);\
        p4 += s.c_str();\
        PATH_TEST_EQ(p4.string(), expected);\
        path p5(p);\
        p5 += derived_from_path(appnd);\
        PATH_TEST_EQ(p5, expected);\
        path p6(p);\
        p6 += convertible_to_path(appnd);\
        PATH_TEST_EQ(p6, expected);\
        path p7(p);\
        p7 += pcustom_string(appnd);\
        PATH_TEST_EQ(p7, expected);\
        path p8(p);\
        p8 += mars_boost::string_view(appnd);\
        PATH_TEST_EQ(p8, expected);\
        CONCAT_TEST_STD_STRING_VIEW(appnd, expected)\
        path p10(p);\
        p10.concat(s.begin(), s.end());\
        PATH_TEST_EQ(p10.string(), expected);\
    }

void concat_tests()
{
    std::cout << "concat_tests..." << std::endl;

    CONCAT_TEST("", "", "");
    CONCAT_TEST("", "/", "/");
    CONCAT_TEST("", "bar", "bar");
    CONCAT_TEST("", "/bar", "/bar");

    CONCAT_TEST("/", "", "/");
    CONCAT_TEST("/", "/", "//");
    CONCAT_TEST("/", "bar", "/bar");
    CONCAT_TEST("/", "/bar", "//bar");

    CONCAT_TEST("foo", "/", "foo/");
    CONCAT_TEST("foo", "/bar", "foo/bar");

    CONCAT_TEST("foo/", "", "foo/");
    CONCAT_TEST("foo/", "/", "foo//");
    CONCAT_TEST("foo/", "bar", "foo/bar");

    CONCAT_TEST("foo", "", "foo");
    CONCAT_TEST("foo", "bar", "foobar");
    CONCAT_TEST("foo\\", "\\bar", "foo\\\\bar");
    CONCAT_TEST("c:", "bar", "c:bar");
}

//  self_assign_append_concat_tests  -------------------------------------------------//

#if defined(__clang__) && defined(__has_warning)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic push
// explicitly assigning value of variable of type 'mars_boost::filesystem::path' to itself
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
#endif

void self_assign_append_concat_tests()
{
    std::cout << "self_assign_append_concat_tests..." << std::endl;

    path p;

    p = "snafubar";
    PATH_TEST_EQ(p = p, "snafubar");

    p = "snafubar";
    p = p.c_str();
    PATH_TEST_EQ(p, "snafubar");

    p = "snafubar";
    p.assign(p.c_str(), path::codecvt());
    PATH_TEST_EQ(p, "snafubar");

    p = "snafubar";
    PATH_TEST_EQ(p = p.c_str() + 5, "bar");

    p = "snafubar";
    PATH_TEST_EQ(p.assign(p.c_str() + 5, p.c_str() + 7), "ba");

    p = "snafubar";
    p /= p;
    PATH_TEST_EQ(p, "snafubar" BOOST_DIR_SEP "snafubar");

    p = "snafubar";
    p /= p.c_str();
    PATH_TEST_EQ(p, "snafubar" BOOST_DIR_SEP "snafubar");

    p = "snafubar";
    p.append(p.c_str(), path::codecvt());
    PATH_TEST_EQ(p, "snafubar" BOOST_DIR_SEP "snafubar");

    p = "snafubar";
    PATH_TEST_EQ(p.append(p.c_str() + 5, p.c_str() + 7), "snafubar" BOOST_DIR_SEP "ba");

    p = "snafubar";
    p += p;
    PATH_TEST_EQ(p, "snafubarsnafubar");

    p = "snafubar";
    p += p.c_str();
    PATH_TEST_EQ(p, "snafubarsnafubar");

    p = "snafubar";
    p.concat(p.c_str(), path::codecvt());
    PATH_TEST_EQ(p, "snafubarsnafubar");

    p = "snafubar";
    PATH_TEST_EQ(p.concat(p.c_str() + 5, p.c_str() + 7), "snafubarba");
}

#if defined(__clang__) && defined(__has_warning)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic pop
#endif
#endif

//  name_function_tests  -------------------------------------------------------------//

void name_function_tests()
{
    std::cout << "name_function_tests..." << std::endl;

    BOOST_TEST(fs::portable_posix_name(std::string("x")));
    BOOST_TEST(fs::windows_name(std::string("x")));
    BOOST_TEST(fs::portable_name(std::string("x")));
    BOOST_TEST(fs::portable_directory_name(std::string("x")));
    BOOST_TEST(fs::portable_file_name(std::string("x")));

    BOOST_TEST(fs::portable_posix_name(std::string(".")));
    BOOST_TEST(fs::windows_name(std::string(".")));
    BOOST_TEST(fs::portable_name(std::string(".")));
    BOOST_TEST(fs::portable_directory_name(std::string(".")));
    BOOST_TEST(!fs::portable_file_name(std::string(".")));

    BOOST_TEST(fs::portable_posix_name(std::string("..")));
    BOOST_TEST(fs::windows_name(std::string("..")));
    BOOST_TEST(fs::portable_name(std::string("..")));
    BOOST_TEST(fs::portable_directory_name(std::string("..")));
    BOOST_TEST(!fs::portable_file_name(std::string("..")));

    BOOST_TEST(!fs::native(std::string("")));
    BOOST_TEST(!fs::portable_posix_name(std::string("")));
    BOOST_TEST(!fs::windows_name(std::string("")));
    BOOST_TEST(!fs::portable_name(std::string("")));
    BOOST_TEST(!fs::portable_directory_name(std::string("")));
    BOOST_TEST(!fs::portable_file_name(std::string("")));

    BOOST_TEST(!fs::native(std::string(" ")));
    BOOST_TEST(!fs::portable_posix_name(std::string(" ")));
    BOOST_TEST(!fs::windows_name(std::string(" ")));
    BOOST_TEST(!fs::portable_name(std::string(" ")));
    BOOST_TEST(!fs::portable_directory_name(std::string(" ")));
    BOOST_TEST(!fs::portable_file_name(std::string(" ")));

    BOOST_TEST(!fs::portable_posix_name(std::string(":")));
    BOOST_TEST(!fs::windows_name(std::string(":")));
    BOOST_TEST(!fs::portable_name(std::string(":")));
    BOOST_TEST(!fs::portable_directory_name(std::string(":")));
    BOOST_TEST(!fs::portable_file_name(std::string(":")));

    BOOST_TEST(fs::portable_posix_name(std::string("-")));
    BOOST_TEST(fs::windows_name(std::string("-")));
    BOOST_TEST(!fs::portable_name(std::string("-")));
    BOOST_TEST(!fs::portable_directory_name(std::string("-")));
    BOOST_TEST(!fs::portable_file_name(std::string("-")));

    BOOST_TEST(!fs::portable_posix_name(std::string("foo bar")));
    BOOST_TEST(fs::windows_name(std::string("foo bar")));
    BOOST_TEST(!fs::windows_name(std::string(" bar")));
    BOOST_TEST(!fs::windows_name(std::string("foo ")));
    BOOST_TEST(!fs::portable_name(std::string("foo bar")));
    BOOST_TEST(!fs::portable_directory_name(std::string("foo bar")));
    BOOST_TEST(!fs::portable_file_name(std::string("foo bar")));

    BOOST_TEST(fs::portable_posix_name(std::string("foo.bar")));
    BOOST_TEST(fs::windows_name(std::string("foo.bar")));
    BOOST_TEST(fs::portable_name(std::string("foo.bar")));
    BOOST_TEST(!fs::portable_directory_name(std::string("foo.bar")));
    BOOST_TEST(fs::portable_file_name(std::string("foo.bar")));

    BOOST_TEST(fs::portable_posix_name(std::string("foo.barf")));
    BOOST_TEST(fs::windows_name(std::string("foo.barf")));
    BOOST_TEST(fs::portable_name(std::string("foo.barf")));
    BOOST_TEST(!fs::portable_directory_name(std::string("foo.barf")));
    BOOST_TEST(!fs::portable_file_name(std::string("foo.barf")));

    BOOST_TEST(fs::portable_posix_name(std::string(".foo")));
    BOOST_TEST(fs::windows_name(std::string(".foo")));
    BOOST_TEST(!fs::portable_name(std::string(".foo")));
    BOOST_TEST(!fs::portable_directory_name(std::string(".foo")));
    BOOST_TEST(!fs::portable_file_name(std::string(".foo")));

    BOOST_TEST(fs::portable_posix_name(std::string("foo.")));
    BOOST_TEST(!fs::windows_name(std::string("foo.")));
    BOOST_TEST(!fs::portable_name(std::string("foo.")));
    BOOST_TEST(!fs::portable_directory_name(std::string("foo.")));
    BOOST_TEST(!fs::portable_file_name(std::string("foo.")));
}

//  replace_extension_tests  ---------------------------------------------------------//

void replace_extension_tests()
{
    std::cout << "replace_extension_tests..." << std::endl;

    BOOST_TEST(path().replace_extension().empty());
    BOOST_TEST(path().replace_extension("a") == ".a");
    BOOST_TEST(path().replace_extension("a.") == ".a.");
    BOOST_TEST(path().replace_extension(".a") == ".a");
    BOOST_TEST(path().replace_extension("a.txt") == ".a.txt");
    // see the rationale in html docs for explanation why this works:
    BOOST_TEST(path().replace_extension(".txt") == ".txt");

    BOOST_TEST(path("a.txt").replace_extension() == "a");
    BOOST_TEST(path("a.txt").replace_extension("") == "a");
    BOOST_TEST(path("a.txt").replace_extension(".") == "a.");
    BOOST_TEST(path("a.txt").replace_extension(".tex") == "a.tex");
    BOOST_TEST(path("a.txt").replace_extension("tex") == "a.tex");
    BOOST_TEST(path("a.").replace_extension(".tex") == "a.tex");
    BOOST_TEST(path("a.").replace_extension("tex") == "a.tex");
    BOOST_TEST(path("a").replace_extension(".txt") == "a.txt");
    BOOST_TEST(path("a").replace_extension("txt") == "a.txt");
    BOOST_TEST(path("a.b.txt").replace_extension(".tex") == "a.b.tex");
    BOOST_TEST(path("a.b.txt").replace_extension("tex") == "a.b.tex");
    BOOST_TEST(path("a/b").replace_extension(".c") == "a/b.c");
    PATH_TEST_EQ(path("a.txt/b").replace_extension(".c"), "a.txt/b.c");         // ticket 4702
    BOOST_TEST(path("foo.txt").replace_extension("exe") == "foo.exe");          // ticket 5118
    BOOST_TEST(path("foo.txt").replace_extension(".tar.bz2") == "foo.tar.bz2"); // ticket 5118
}

//  make_preferred_tests  ------------------------------------------------------------//

void make_preferred_tests()
{
    std::cout << "make_preferred_tests..." << std::endl;

    if (platform == "Windows")
    {
        BOOST_TEST(path("//abc\\def/ghi").make_preferred().native() == path("\\\\abc\\def\\ghi").native());
    }
    else
    {
        BOOST_TEST(path("//abc\\def/ghi").make_preferred().native() == path("//abc\\def/ghi").native());
    }
}

//  lexically_normal_tests  ----------------------------------------------------------//

void lexically_normal_tests()
{
    std::cout << "lexically_normal_tests..." << std::endl;

    //  Note: lexically_normal() uses /= to build up some results, so these results will
    //  have the platform's preferred separator. Since that is immaterial to the correct
    //  functioning of lexically_normal(), the test results are converted to generic form,
    //  and the expected results are also given in generic form. Otherwise many of the
    //  tests would incorrectly be reported as failing on Windows.

    PATH_TEST_EQ(path("").lexically_normal().generic_path(), "");
    PATH_TEST_EQ(path("/").lexically_normal().generic_path(), "/");
    PATH_TEST_EQ(path("//").lexically_normal().generic_path(), "//");
    PATH_TEST_EQ(path("///").lexically_normal().generic_path(), "/");
    PATH_TEST_EQ(path("f").lexically_normal().generic_path(), "f");
    PATH_TEST_EQ(path("foo").lexically_normal().generic_path(), "foo");
    PATH_TEST_EQ(path("foo/").lexically_normal().generic_path(), "foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("f/").lexically_normal().generic_path(), "f/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("/foo").lexically_normal().generic_path(), "/foo");
    PATH_TEST_EQ(path("/./foo").lexically_normal().generic_path(), "/foo");
    PATH_TEST_EQ(path("/./foo/.").lexically_normal().generic_path(), "/foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("foo/bar").lexically_normal().generic_path(), "foo/bar");
    PATH_TEST_EQ(path("..").lexically_normal().generic_path(), "..");
    PATH_TEST_EQ(path("../..").lexically_normal().generic_path(), "../..");
    PATH_TEST_EQ(path("/..").lexically_normal().generic_path(), "/..");
    PATH_TEST_EQ(path("/../..").lexically_normal().generic_path(), "/../..");
    PATH_TEST_EQ(path("../foo").lexically_normal().generic_path(), "../foo");
    PATH_TEST_EQ(path("foo/..").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ(path("foo/../").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ((path("foo") / "..").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ(path("foo/...").lexically_normal().generic_path(), "foo/...");
    PATH_TEST_EQ(path("foo/.../").lexically_normal().generic_path(), "foo/.../" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("foo/..bar").lexically_normal().generic_path(), "foo/..bar");
    PATH_TEST_EQ(path("../f").lexically_normal().generic_path(), "../f");
    PATH_TEST_EQ(path("/../f").lexically_normal().generic_path(), "/../f");
    PATH_TEST_EQ(path("f/..").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ((path("f") / "..").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ(path("foo/../..").lexically_normal().generic_path(), "..");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(path("foo/../../").lexically_normal().generic_path(), "../.");
#else
    PATH_TEST_EQ(path("foo/../../").lexically_normal().generic_path(), "..");
#endif
    PATH_TEST_EQ(path("foo/../../..").lexically_normal().generic_path(), "../..");
#if BOOST_FILESYSTEM_VERSION == 3
    PATH_TEST_EQ(path("foo/../../../").lexically_normal().generic_path(), "../../.");
#else
    PATH_TEST_EQ(path("foo/../../../").lexically_normal().generic_path(), "../..");
#endif
    PATH_TEST_EQ(path("foo/../bar").lexically_normal().generic_path(), "bar");
    PATH_TEST_EQ(path("foo/../bar/").lexically_normal().generic_path(), "bar/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("foo/bar/..").lexically_normal().generic_path(), "foo");
    PATH_TEST_EQ(path("foo/./bar/..").lexically_normal().generic_path(), "foo");
    std::cout << path("foo/./bar/..").lexically_normal() << std::endl; // outputs "foo"
    PATH_TEST_EQ(path("foo/bar/../").lexically_normal().generic_path(), "foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("foo/./bar/../").lexically_normal().generic_path(), "foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    std::cout << path("foo/./bar/../").lexically_normal() << std::endl; // POSIX: "foo/", Windows: "foo\" (with a trailing dot for v3)
    PATH_TEST_EQ(path("foo/bar/../..").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ(path("foo/bar/../../").lexically_normal().generic_path(), ".");
    PATH_TEST_EQ(path("foo/bar/../blah").lexically_normal().generic_path(), "foo/blah");
    PATH_TEST_EQ(path("f/../b").lexically_normal().generic_path(), "b");
    PATH_TEST_EQ(path("f/b/..").lexically_normal().generic_path(), "f");
    PATH_TEST_EQ(path("f/b/../").lexically_normal().generic_path(), "f/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("f/b/../a").lexically_normal().generic_path(), "f/a");
    PATH_TEST_EQ(path("foo/bar/blah/../..").lexically_normal().generic_path(), "foo");
    PATH_TEST_EQ(path("foo/bar/blah/../../bletch").lexically_normal().generic_path(), "foo/bletch");
    PATH_TEST_EQ(path("//net").lexically_normal().generic_path(), "//net");
    PATH_TEST_EQ(path("//net/").lexically_normal().generic_path(), "//net/");
    PATH_TEST_EQ(path("//..net").lexically_normal().generic_path(), "//..net");
    PATH_TEST_EQ(path("//net/..").lexically_normal().generic_path(), "//net/..");
    PATH_TEST_EQ(path("//net/foo").lexically_normal().generic_path(), "//net/foo");
    PATH_TEST_EQ(path("//net/foo/").lexically_normal().generic_path(), "//net/foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("//net/foo/..").lexically_normal().generic_path(), "//net/");
    PATH_TEST_EQ(path("//net/foo/../").lexically_normal().generic_path(), "//net/" BOOST_FILESYSTEM_V3_TRAILING_DOT);

    PATH_TEST_EQ(path("/net/foo/bar").lexically_normal().generic_path(), "/net/foo/bar");
    PATH_TEST_EQ(path("/net/foo/bar/").lexically_normal().generic_path(), "/net/foo/bar/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("/net/foo/..").lexically_normal().generic_path(), "/net");
    PATH_TEST_EQ(path("/net/foo/../").lexically_normal().generic_path(), "/net/" BOOST_FILESYSTEM_V3_TRAILING_DOT);

    PATH_TEST_EQ(path("//net//foo//bar").lexically_normal().generic_path(), "//net/foo/bar");
    PATH_TEST_EQ(path("//net//foo//bar//").lexically_normal().generic_path(), "//net/foo/bar/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("//net//foo//..").lexically_normal().generic_path(), "//net/");
    PATH_TEST_EQ(path("//net//foo//..//").lexically_normal().generic_path(), "//net/" BOOST_FILESYSTEM_V3_TRAILING_DOT);

    PATH_TEST_EQ(path("///net///foo///bar").lexically_normal().generic_path(), "/net/foo/bar");
    PATH_TEST_EQ(path("///net///foo///bar///").lexically_normal().generic_path(), "/net/foo/bar/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
    PATH_TEST_EQ(path("///net///foo///..").lexically_normal().generic_path(), "/net");
    PATH_TEST_EQ(path("///net///foo///..///").lexically_normal().generic_path(), "/net/" BOOST_FILESYSTEM_V3_TRAILING_DOT);

    if (platform == "Windows")
    {
        BOOST_TEST_EQ(path("c:/foo/bar").lexically_normal().string(), "c:\\foo\\bar");

        PATH_TEST_EQ(path("c:foo").lexically_normal().generic_path(), "c:foo");
        PATH_TEST_EQ(path("c:..").lexically_normal().generic_path(), "c:..");
        PATH_TEST_EQ(path("c:foo/..").lexically_normal().generic_path(), "c:");

        PATH_TEST_EQ(path("c:foo/../").lexically_normal().generic_path(), "c:" BOOST_FILESYSTEM_V3_TRAILING_DOT);

        PATH_TEST_EQ(path("c:/foo/..").lexically_normal().generic_path(), "c:/");
        PATH_TEST_EQ(path("c:/foo/../").lexically_normal().generic_path(), "c:/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(path("c:/..").lexically_normal().generic_path(), "c:/..");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("c:/../").lexically_normal().generic_path(), "c:/../.");
#else
        PATH_TEST_EQ(path("c:/../").lexically_normal().generic_path(), "c:/..");
#endif
        PATH_TEST_EQ(path("c:/../..").lexically_normal().generic_path(), "c:/../..");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("c:/../../").lexically_normal().generic_path(), "c:/../../.");
#else
        PATH_TEST_EQ(path("c:/../../").lexically_normal().generic_path(), "c:/../..");
#endif
        PATH_TEST_EQ(path("c:/../foo").lexically_normal().generic_path(), "c:/../foo");
        PATH_TEST_EQ(path("c:/../foo/").lexically_normal().generic_path(), "c:/../foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(path("c:/../../foo").lexically_normal().generic_path(), "c:/../../foo");
        PATH_TEST_EQ(path("c:/../../foo/").lexically_normal().generic_path(), "c:/../../foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(path("c:/..foo").lexically_normal().generic_path(), "c:/..foo");
    }
    else // POSIX
    {
        PATH_TEST_EQ(path("c:..").lexically_normal(), "c:..");
        PATH_TEST_EQ(path("c:foo/..").lexically_normal(), ".");
        PATH_TEST_EQ(path("c:foo/../").lexically_normal(), ".");
        PATH_TEST_EQ(path("c:/foo/..").lexically_normal(), "c:");
        PATH_TEST_EQ(path("c:/foo/../").lexically_normal(), "c:/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(path("c:/..").lexically_normal(), ".");
        PATH_TEST_EQ(path("c:/../").lexically_normal(), ".");
        PATH_TEST_EQ(path("c:/../..").lexically_normal(), "..");
#if BOOST_FILESYSTEM_VERSION == 3
        PATH_TEST_EQ(path("c:/../../").lexically_normal(), "../.");
#else
        PATH_TEST_EQ(path("c:/../../").lexically_normal(), "..");
#endif
        PATH_TEST_EQ(path("c:/../foo").lexically_normal(), "foo");
        PATH_TEST_EQ(path("c:/../foo/").lexically_normal(), "foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(path("c:/../../foo").lexically_normal(), "../foo");
        PATH_TEST_EQ(path("c:/../../foo/").lexically_normal(), "../foo/" BOOST_FILESYSTEM_V3_TRAILING_DOT);
        PATH_TEST_EQ(path("c:/..foo").lexically_normal(), "c:/..foo");
    }
}

//  compare_tests  -------------------------------------------------------------------//

#define COMPARE_TEST(pth1, pth2)\
    {\
        BOOST_TEST_EQ(fs::path(pth1).compare(pth1), 0);\
        BOOST_TEST_LT(fs::path(pth1).compare(pth2), 0);\
        BOOST_TEST_GT(fs::path(pth2).compare(pth1), 0);\
        BOOST_TEST(fs::path(pth1) == pth1);\
        BOOST_TEST(pth1 == fs::path(pth1));\
        BOOST_TEST(fs::path(pth1) != pth2);\
        BOOST_TEST(pth1 != fs::path(pth2));\
        BOOST_TEST(fs::path(pth1) < pth2);\
        BOOST_TEST(pth1 < fs::path(pth2));\
        BOOST_TEST(fs::path(pth2) > pth1);\
        BOOST_TEST(pth2 > fs::path(pth1));\
        BOOST_TEST(fs::path(pth1) <= pth1);\
        BOOST_TEST(pth1 <= fs::path(pth2));\
        BOOST_TEST(fs::path(pth1) >= pth1);\
        BOOST_TEST(pth2 >= fs::path(pth1));\
    }

void compare_tests()
{
    COMPARE_TEST(fs::path("foo"), fs::path("zoo"))
    COMPARE_TEST("foo", "zoo")
    COMPARE_TEST(std::string("foo"), std::string("zoo"))
    COMPARE_TEST(derived_from_path("foo"), derived_from_path("zoo"))
    COMPARE_TEST(pcustom_string("foo"), pcustom_string("zoo"))
    COMPARE_TEST(mars_boost::string_view("foo"), mars_boost::string_view("zoo"))
#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
    COMPARE_TEST(std::string_view("foo"), std::string_view("zoo"))
#endif

    COMPARE_TEST("/foo", "foo")
    COMPARE_TEST("/a/b", "foo")
    COMPARE_TEST("/foo", "/zoo")
    COMPARE_TEST("/foo", "/foo/bar")

    if (platform == "Windows")
    {
        COMPARE_TEST("c:\\foo", "d:\\foo")
        COMPARE_TEST("c:\\foo", "c:\\zoo")
    }
}

inline void odr_use(const path::value_type& c)
{
    static const path::value_type dummy = '\0';
    BOOST_TEST(&c != &dummy);
}

} // unnamed namespace

static mars_boost::filesystem::path ticket_6737 = "FilePath"; // #6737 reported this crashed
                                                         // on VC++ debug mode build
const mars_boost::filesystem::path ticket_6690("test");       // #6690 another V++ static init crash

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                     main                                             //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#if defined(__clang__) && defined(__has_warning)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic push
// explicitly assigning value of variable of type 'mars_boost::filesystem::path' to itself
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
#endif

int cpp_main(int, char*[])
{
    // The choice of platform is make at runtime rather than compile-time
    // so that compile errors for all platforms will be detected even though
    // only the current platform is runtime tested.
    platform = (platform == "Win32" || platform == "Win64" || platform == "Cygwin") ? "Windows" : "POSIX";
    std::cout << "Platform is " << platform << '\n';

    BOOST_TEST(p1.string() != p3.string());
    p3 = p2;
    BOOST_TEST(p1.string() == p3.string());

    path p04("foobar");
    BOOST_TEST(p04.string() == "foobar");
    p04 = p04; // self-assignment
    BOOST_TEST(p04.string() == "foobar");

    construction_tests();
    append_tests();
    concat_tests();
    self_assign_append_concat_tests();
    overload_tests();
    query_and_decomposition_tests();
    composition_tests();
    iterator_tests();
    non_member_tests();
    exception_tests();
    name_function_tests();
    replace_extension_tests();
    make_preferred_tests();
    lexically_normal_tests();
    compare_tests();

    // verify deprecated names still available

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED

    p1.branch_path();
    p1.leaf();
    path p_remove_leaf;
    p_remove_leaf.remove_leaf();

#endif

    std::string s1("//:somestring"); // this used to be treated specially

    // check the path member templates
    p5.assign(s1.begin(), s1.end());

    PATH_TEST_EQ(p5.string(), "//:somestring");
    p5 = s1;
    PATH_TEST_EQ(p5.string(), "//:somestring");

    // this code, courtesy of David Whetstone, detects a now fixed bug that
    // derefereced the end iterator (assuming debug build with checked itors)
    std::vector< char > v1;
    p5.assign(v1.begin(), v1.end());
    std::string s2(v1.begin(), v1.end());
    PATH_TEST_EQ(p5.string(), s2);
    p5.assign(s1.begin(), s1.begin() + 1);
    PATH_TEST_EQ(p5.string(), "/");

    BOOST_TEST(p1 != p4);
    BOOST_TEST(p1.string() == p2.string());
    BOOST_TEST(p1.string() == p3.string());
    BOOST_TEST(path("foo").filename() == "foo");
    BOOST_TEST(path("foo").parent_path().string() == "");
    BOOST_TEST(p1.filename() == "fum");
    BOOST_TEST(p1.parent_path().string() == "fe/fi/fo");
    BOOST_TEST(path("").empty() == true);
    BOOST_TEST(path("foo").empty() == false);

    // inserter and extractor tests
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1300 // bypass VC++ 7.0 and earlier
    std::cout << "\nInserter and extractor test...";
    std::stringstream ss;
    ss << fs::path("foo/bar") << std::endl;
    fs::path round_trip;
    ss >> round_trip;
    BOOST_TEST(round_trip.string() == "foo/bar");
    std::cout << round_trip.string() << "..." << round_trip << " complete\n";
#endif

    // Check that path constants have definitions
    // https://svn.boost.org/trac10/ticket/12759
    odr_use(path::separator);
    odr_use(path::preferred_separator);
    odr_use(path::dot);

    return ::mars_boost::report_errors();
}

#if defined(__clang__) && defined(__has_warning)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic pop
#endif
#endif
