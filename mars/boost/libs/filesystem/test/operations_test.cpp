//  Boost operations_test.cpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 2002, 2009.

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
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/file_status.hpp>
#include <boost/filesystem/fstream.hpp> // for BOOST_FILESYSTEM_C_STR

#include <boost/cerrno.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/system_category.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>

namespace fs = mars_boost::filesystem;
using mars_boost::system::error_code;
using mars_boost::system::system_category;
using mars_boost::system::system_error;

#include <fstream>
#include <iostream>

using std::cout;
using std::endl;

#include <string>
#include <vector>
#include <algorithm>
#include <cstring> // for strncmp, etc.
#include <ctime>
#include <cstdlib> // for system(), getenv(), etc.
#ifdef BOOST_POSIX_API
#include <unistd.h>
#endif

#ifdef BOOST_WINDOWS_API
#include <windows.h>

inline std::wstring convert(const char* c)
{
    std::string s(c);

    return std::wstring(s.begin(), s.end());
}

//  Note: these three setenv* functions are not general solutions for the missing
//  setenv* problem on VC++. See Microsoft's _putenv for that need, and ticker #7018
//  for discussion and rationale for returning void for this test program, which needs
//  to work for both the MSVC Runtime and the Windows Runtime (which does not support
//  _putenv).

inline void setenv_(const char* name, const fs::path::value_type* val, int)
{
    SetEnvironmentVariableW(convert(name).c_str(), val);
}

inline void setenv_(const char* name, const char* val, int)
{
    SetEnvironmentVariableW(convert(name).c_str(), convert(val).c_str());
}

inline void unsetenv_(const char* name)
{
    SetEnvironmentVariableW(convert(name).c_str(), 0);
}

//! Sets read-only attribute on a file
inline void set_read_only(fs::path const& p)
{
    DWORD attrs = GetFileAttributesW(p.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        DWORD err = GetLastError();
        throw fs::filesystem_error("operations_test set_read_only: failed to get file attributes", p, error_code(err, system_category()));
    }

    attrs |= FILE_ATTRIBUTE_READONLY;
    if (!SetFileAttributesW(p.c_str(), attrs))
    {
        DWORD err = GetLastError();
        throw fs::filesystem_error("operations_test set_read_only: failed to set file attributes", p, error_code(err, system_category()));
    }
}

//! Converts path to a Windows long path
inline fs::path make_long_path(fs::path const& p)
{
    fs::path lp;
    // Long paths must be absolute, use preferred separators and not contain dot and dot-dot elements.
    // Also, UNC paths must use the "\\?\UNC\" prefix.
    fs::path np = p.lexically_normal();
    np.make_preferred();
    std::wstring rp = np.root_path().wstring();
    if (rp.size() > 4u && rp[0] == L'\\' && rp[1] == '\\' && (rp[2] == L'?' || rp[2] == L'.') && rp[3] == '\\')
        lp = rp;
    else if (rp.size() > 2u && rp[0] == L'\\' && rp[1] == '\\')
        lp = L"\\\\?\\UNC\\" + rp.substr(2);
    else
        lp = L"\\\\?\\" + rp;

    lp /= np.relative_path();

    return lp;
}

#else

#include <unistd.h> // sleep
#include <stdlib.h> // allow unqualifed calls to env funcs on SunOS

inline void setenv_(const char* name, const char* val, int ovw)
{
    setenv(name, val, ovw);
}

inline void unsetenv_(const char* name)
{
    unsetenv(name);
}

#endif

#define CHECK_EXCEPTION(Functor, Expect) throws_fs_error(Functor, Expect, __LINE__)

namespace {
typedef int errno_t;
std::string platform(BOOST_PLATFORM);
bool report_throws = false;
bool cleanup = true;
bool skip_long_windows_tests = false;

fs::directory_iterator end_itr;
fs::path dir;
fs::path d1;
fs::path d2;
fs::path f0;
fs::path f1;
fs::path d1f1;

bool create_symlink_ok(true);

fs::path ng(" no-way, Jose");

const fs::path temp_dir(fs::unique_path("op-test-%%%%-%%%%"));

void create_file(const fs::path& ph, const std::string& contents = std::string())
{
    std::ofstream f(BOOST_FILESYSTEM_C_STR(ph));
    if (!f)
        throw fs::filesystem_error("operations_test create_file", ph, error_code(errno, system_category()));
    if (!contents.empty())
        f << contents;
}

void verify_file(const fs::path& ph, const std::string& expected)
{
    std::ifstream f(BOOST_FILESYSTEM_C_STR(ph));
    if (!f)
        throw fs::filesystem_error("operations_test verify_file", ph, error_code(errno, system_category()));
    std::string contents;
    f >> contents;
    if (contents != expected)
        throw fs::filesystem_error("operations_test verify_file contents \"" + contents + "\" != \"" + expected + "\"", ph, error_code());
}

template< typename F >
bool throws_fs_error(F func, errno_t en, int line)
{
    try
    {
        func();
    }
    catch (const fs::filesystem_error& ex)
    {
        if (report_throws)
        {
            // use the what() convenience function to display exceptions
            cout << "\n"
                 << ex.what() << "\n";
        }
        if (en == 0 || en == ex.code().default_error_condition().value())
            return true;
        cout
            << "\nWarning: line " << line
            << " exception reports default_error_condition().value() "
            << ex.code().default_error_condition().value()
            << ", should be " << en
            << "\n value() is " << ex.code().value()
            << endl;
        return true;
    }
    return false;
}

struct poison_category_impl : public mars_boost::system::error_category
{
    char const* name() const BOOST_NOEXCEPT { return "poison"; }
    std::string message(int) const { return "poison_category::message"; }
};

mars_boost::system::error_category& poison_category()
{
    static poison_category_impl instance;
    return instance;
}

// compile-only two argument "do-the-right-thing" tests
//   verifies that all overload combinations compile without error
void do_the_right_thing_tests(bool call_ = false)
{
    if (call_)
    {
        fs::path p;
        std::string s;
        const char* a = 0;
        fs::copy_file(p, p);
        fs::copy_file(s, p);
        fs::copy_file(a, p);
        fs::copy_file(p, s);
        fs::copy_file(p, a);
        fs::copy_file(s, s);
        fs::copy_file(a, s);
        fs::copy_file(s, a);
        fs::copy_file(a, a);
    }
}

void bad_file_size()
{
    fs::file_size(" No way, Jose");
}

void bad_directory_size()
{
    fs::file_size(fs::current_path());
}

fs::path bad_create_directory_path;
void bad_create_directory()
{
    fs::create_directory(bad_create_directory_path);
}

void bad_equivalent()
{
    fs::equivalent("no-such-path", "another-not-present-path");
}

fs::path bad_remove_dir;
void bad_remove()
{
    fs::remove(bad_remove_dir);
}

void bad_space()
{
    fs::space("no-such-path");
}

class renamer
{
public:
    renamer(const fs::path& p1, const fs::path& p2) :
        from(p1), to(p2) {}
    void operator()()
    {
        fs::rename(from, to);
    }

private:
    fs::path from;
    fs::path to;
};

//------------------------------ debugging aids --------------------------------------//

//std::ostream& operator<<(std::ostream& os, const fs::file_status& s)
//{
//  if (s.type() == fs::status_error)        { os << "status_error"; }
//  else if (s.type() == fs::file_not_found) { os << "file_not_found"; }
//  else if (s.type() == fs::regular_file)   { os << "regular_file"; }
//  else if (s.type() == fs::directory_file) { os << "directory_file"; }
//  else if (s.type() == fs::symlink_file)   { os << "symlink_file"; }
//  else if (s.type() == fs::block_file)     { os << "block_file"; }
//  else if (s.type() == fs::character_file) { os << "character_file"; }
//  else if (s.type() == fs::fifo_file)      { os << "fifo_file"; }
//  else if (s.type() == fs::socket_file)    { os << "socket_file"; }
//  else if (s.type() == fs::reparse_file)   { os << "reparse_file"; }
//  else                                     { os << "type_unknown"; }
//  return os;
//}

//void dump_tree(const fs::path & root)
//{
//  cout << "dumping tree rooted at " << root << endl;
//  for (fs::recursive_directory_iterator it (root, fs::directory_options::follow_directory_symlink);
//       it != fs::recursive_directory_iterator();
//       ++it)
//  {
//    for (int i = 0; i <= it.level(); ++i)
//      cout << "  ";

//    cout << it->path();
//    if (fs::is_symlink(it->path()))
//    {
//      cout << " [symlink]" << endl;
//    }
//    else
//      cout << endl;
//  }
//}

//  exception_tests()  ---------------------------------------------------------------//

#if defined(BOOST_GCC) && BOOST_GCC >= 80000
#pragma GCC diagnostic push
// catching polymorphic type "X" by value - that's the intention of the test
#pragma GCC diagnostic ignored "-Wcatch-value"
#endif

void exception_tests()
{
    cout << "exception_tests..." << endl;
    bool exception_thrown;

    //  catch runtime_error by value

    cout << "  catch runtime_error by value" << endl;
    exception_thrown = false;
    try
    {
        fs::create_directory("no-such-dir/foo/bar");
    }
    catch (std::runtime_error x)
    {
        exception_thrown = true;
        cout << "    x.what() returns \"" << x.what() << "\"" << endl;
    }
    BOOST_TEST(exception_thrown);

    //  catch system_error by value

    cout << "  catch system_error by value" << endl;
    exception_thrown = false;
    try
    {
        fs::create_directory("no-such-dir/foo/bar");
    }
    catch (system_error x)
    {
        exception_thrown = true;
        cout << "    x.what() returns \"" << x.what() << "\"" << endl;
    }
    BOOST_TEST(exception_thrown);

    //  catch filesystem_error by value

    cout << "  catch filesystem_error by value" << endl;
    exception_thrown = false;
    try
    {
        fs::create_directory("no-such-dir/foo/bar");
    }
    catch (fs::filesystem_error x)
    {
        exception_thrown = true;
        cout << "    x.what() returns \"" << x.what() << "\"" << endl;
    }
    BOOST_TEST(exception_thrown);

    //  catch filesystem_error by const reference

    cout << "  catch filesystem_error by const reference" << endl;
    exception_thrown = false;
    try
    {
        fs::create_directory("no-such-dir/foo/bar");
    }
    catch (const fs::filesystem_error& x)
    {
        exception_thrown = true;
        cout << "    x.what() returns \"" << x.what() << "\"" << endl;
    }
    BOOST_TEST(exception_thrown);

    // the bound functions should throw, so CHECK_EXCEPTION() should return true

    BOOST_TEST(CHECK_EXCEPTION(bad_file_size, ENOENT));

    if (platform == "Windows")
        BOOST_TEST(CHECK_EXCEPTION(bad_directory_size, ENOENT));
    else
        BOOST_TEST(CHECK_EXCEPTION(bad_directory_size, 0));

    // test path::exception members
    try
    {
        fs::file_size(ng); // will throw
    }
    catch (fs::filesystem_error& ex)
    {
        BOOST_TEST(ex.path1().string() == " no-way, Jose");
    }

    cout << "  exception_tests complete" << endl;
}

#if defined(BOOST_GCC) && BOOST_GCC >= 80000
#pragma GCC diagnostic pop
#endif

// create a directory tree that can be used by subsequent tests  ---------------------//
//
//    dir
//      d1
//        d1f1       // an empty file
//      f0           // an empty file
//      f1           // a file containing "file-f1"

void create_tree()
{
    cout << "creating test directories and files in " << dir << endl;

    // create directory d1
    BOOST_TEST(!fs::create_directory(dir));
    BOOST_TEST(!fs::is_symlink(dir));
    BOOST_TEST(!fs::is_symlink("nosuchfileordirectory"));
    d1 = dir / "d1";
    BOOST_TEST(fs::create_directory(d1));
    BOOST_TEST(fs::exists(d1));
    BOOST_TEST(fs::is_directory(d1));
    BOOST_TEST(fs::is_empty(d1));

    // create an empty file named "d1f1"
    d1f1 = d1 / "d1f1";
    create_file(d1f1, "");
    BOOST_TEST(fs::exists(d1f1));
    BOOST_TEST(!fs::is_directory(d1f1));
    BOOST_TEST(fs::is_regular_file(d1f1));
    BOOST_TEST(fs::is_empty(d1f1));
    BOOST_TEST(fs::file_size(d1f1) == 0);
    BOOST_TEST(fs::hard_link_count(d1f1) == 1);

    // create an empty file named "f0"
    f0 = dir / "f0";
    create_file(f0, "");
    BOOST_TEST(fs::exists(f0));
    BOOST_TEST(!fs::is_directory(f0));
    BOOST_TEST(fs::is_regular_file(f0));
    BOOST_TEST(fs::is_empty(f0));
    BOOST_TEST(fs::file_size(f0) == 0);
    BOOST_TEST(fs::hard_link_count(f0) == 1);

    // create a file named "f1"
    f1 = dir / "f1";
    create_file(f1, "file-f1");
    BOOST_TEST(fs::exists(f1));
    BOOST_TEST(!fs::is_directory(f1));
    BOOST_TEST(fs::is_regular_file(f1));
    BOOST_TEST(fs::file_size(f1) == 7);
    verify_file(f1, "file-f1");
}

//  directory_iterator_tests  --------------------------------------------------------//

void directory_iterator_tests()
{
    cout << "directory_iterator_tests..." << endl;

    bool dir_itr_exception(false);
    try
    {
        fs::directory_iterator it("");
    }
    catch (const fs::filesystem_error&)
    {
        dir_itr_exception = true;
    }
    BOOST_TEST(dir_itr_exception);

    error_code ec;

    BOOST_TEST(!ec);
    fs::directory_iterator it("", ec);
    BOOST_TEST(ec);

    dir_itr_exception = false;
    try
    {
        fs::directory_iterator itx("nosuchdirectory");
    }
    catch (const fs::filesystem_error&)
    {
        dir_itr_exception = true;
    }
    BOOST_TEST(dir_itr_exception);

    ec.clear();
    fs::directory_iterator it2x("nosuchdirectory", ec);
    BOOST_TEST(ec);

    dir_itr_exception = false;
    try
    {
        error_code ecx;
        fs::directory_iterator itx("nosuchdirectory", ecx);
        BOOST_TEST(ecx);
        BOOST_TEST(ecx == mars_boost::system::errc::no_such_file_or_directory);
    }
    catch (const fs::filesystem_error&)
    {
        dir_itr_exception = true;
    }
    BOOST_TEST(!dir_itr_exception);

    // create a second directory named d2
    d2 = dir / "d2";
    fs::create_directory(d2);
    BOOST_TEST(fs::exists(d2));
    BOOST_TEST(fs::is_directory(d2));

    // test the basic operation of directory_iterators, and test that
    // stepping one iterator doesn't affect a different iterator.
    {
        typedef std::vector< fs::directory_entry > vec_type;
        vec_type vec;

        fs::directory_iterator it1(dir);
        BOOST_TEST(it1 != fs::directory_iterator());
        BOOST_TEST(fs::exists(it1->status()));
        vec.push_back(*it1);
        BOOST_TEST(*it1 == vec[0]);

        fs::directory_iterator it2(dir);
        BOOST_TEST(it2 != fs::directory_iterator());
        BOOST_TEST(*it1 == *it2);

        ++it1;
        BOOST_TEST(it1 != fs::directory_iterator());
        BOOST_TEST(fs::exists(it1->status()));
        BOOST_TEST(it1 != it2);
        BOOST_TEST(*it1 != vec[0]);
        BOOST_TEST(*it2 == vec[0]);
        vec.push_back(*it1);

        ++it1;
        BOOST_TEST(it1 != fs::directory_iterator());
        BOOST_TEST(fs::exists(it1->status()));
        BOOST_TEST(it1 != it2);
        BOOST_TEST(*it2 == vec[0]);
        vec.push_back(*it1);

        ++it1;
        BOOST_TEST(it1 != fs::directory_iterator());
        BOOST_TEST(fs::exists(it1->status()));
        BOOST_TEST(it1 != it2);
        BOOST_TEST(*it2 == vec[0]);
        vec.push_back(*it1);

        ++it1;
        BOOST_TEST(it1 == fs::directory_iterator());

        BOOST_TEST(*it2 == vec[0]);
        ec.clear();
        it2.increment(ec);
        BOOST_TEST(!ec);
        BOOST_TEST(it2 != fs::directory_iterator());
        BOOST_TEST(it1 == fs::directory_iterator());
        BOOST_TEST(*it2 == vec[1]);
        ++it2;
        BOOST_TEST(*it2 == vec[2]);
        BOOST_TEST(it1 == fs::directory_iterator());
        ++it2;
        BOOST_TEST(*it2 == vec[3]);
        ++it2;
        BOOST_TEST(it1 == fs::directory_iterator());
        BOOST_TEST(it2 == fs::directory_iterator());

        // sort vec and check that the right directory entries were found
        std::sort(vec.begin(), vec.end());

        BOOST_TEST_EQ(vec[0].path().filename().string(), std::string("d1"));
        BOOST_TEST_EQ(vec[1].path().filename().string(), std::string("d2"));
        BOOST_TEST_EQ(vec[2].path().filename().string(), std::string("f0"));
        BOOST_TEST_EQ(vec[3].path().filename().string(), std::string("f1"));
    }

    { // *i++ must meet the standard's InputIterator requirements
        fs::directory_iterator dir_itr(dir);
        BOOST_TEST(dir_itr != fs::directory_iterator());
        fs::path p = dir_itr->path();
        BOOST_TEST((*dir_itr++).path() == p);
        BOOST_TEST(dir_itr != fs::directory_iterator());
        BOOST_TEST(dir_itr->path() != p);

        // test case reported in comment to SourceForge bug tracker [937606]
        // augmented to test single pass semantics of a copied iterator [#12578]
        fs::directory_iterator itx(dir);
        fs::directory_iterator itx2(itx);
        BOOST_TEST(itx == itx2);
        const fs::path p1 = (*itx++).path();
        BOOST_TEST(itx == itx2);
        BOOST_TEST(itx != fs::directory_iterator());
        const fs::path p2 = (*itx++).path();
        BOOST_TEST(itx == itx2);
        BOOST_TEST(p1 != p2);
        ++itx;
        BOOST_TEST(itx == itx2);
        ++itx;
        BOOST_TEST(itx == itx2);
        BOOST_TEST(itx == fs::directory_iterator());
        BOOST_TEST(itx2 == fs::directory_iterator());
    }

    //  Windows has a tricky special case when just the root-name is given,
    //  causing the rest of the path to default to the current directory.
    //  Reported as S/F bug [ 1259176 ]
    if (platform == "Windows")
    {
        fs::path root_name_path(fs::current_path().root_name());
        fs::directory_iterator itx(root_name_path);
        BOOST_TEST(itx != fs::directory_iterator());
        //      BOOST_TEST(fs::exists((*itx).path()));
        BOOST_TEST(fs::exists(itx->path()));
        BOOST_TEST(itx->path().parent_path() == root_name_path);
        bool found(false);
        do
        {
            if (itx->path().filename() == temp_dir.filename())
                found = true;
        } while (++itx != fs::directory_iterator());
        BOOST_TEST(found);
    }

    // there was an inital bug in directory_iterator that caused premature
    // close of an OS handle. This block will detect regression.
    {
        fs::directory_iterator di;
        {
            di = fs::directory_iterator(dir);
        }
        BOOST_TEST(++di != fs::directory_iterator());
    }

    cout << "  directory_iterator_tests complete" << endl;
}

//  recursive_directory_iterator_tests  ----------------------------------------------//

int walk_tree(bool recursive)
{
    //cout << "    walk_tree" << endl;
    error_code ec;
    int d1f1_count = 0;
    for (fs::recursive_directory_iterator it(dir, recursive ? (fs::directory_options::follow_directory_symlink | fs::directory_options::skip_dangling_symlinks) : fs::directory_options::none);
         it != fs::recursive_directory_iterator();
         it.increment(ec))
    {
        //cout << "      " << it->path() << " : " << ec << endl;
        if (it->path().filename() == "d1f1")
            ++d1f1_count;
    }
    //cout << "      last error : " << ec << endl;
    return d1f1_count;
}

void recursive_directory_iterator_tests()
{
    cout << "recursive_directory_iterator_tests..." << endl;
    BOOST_TEST_EQ(walk_tree(false), 1);
    if (create_symlink_ok)
        BOOST_TEST(walk_tree(true) > 1);

    //  test iterator increment with error_code argument
    cout << "  with error_code argument" << endl;
    mars_boost::system::error_code ec;
    int d1f1_count = 0;
    fs::recursive_directory_iterator it(dir, fs::directory_options::none);
    fs::recursive_directory_iterator it2(it); // test single pass shallow copy semantics
    for (;
         it != fs::recursive_directory_iterator();
         it.increment(ec))
    {
        if (it->path().filename() == "d1f1")
            ++d1f1_count;
        BOOST_TEST(it == it2); // verify single pass shallow copy semantics
    }
    BOOST_TEST(!ec);
    BOOST_TEST_EQ(d1f1_count, 1);
    BOOST_TEST(it == it2); // verify single pass shallow copy semantics

    cout << "  recursive_directory_iterator_tests complete" << endl;
}

//  iterator_status_tests  -----------------------------------------------------------//

void iterator_status_tests()
{
    cout << "iterator_status_tests..." << endl;

    error_code ec;
    // harmless if these fail:
    fs::create_symlink(dir / "f0", dir / "f0_symlink", ec);
    fs::create_symlink(dir / "no such file", dir / "dangling_symlink", ec);
    fs::create_directory_symlink(dir / "d1", dir / "d1_symlink", ec);
    fs::create_directory_symlink(dir / "no such directory", dir / "dangling_directory_symlink", ec);

    for (fs::directory_iterator it(dir);
         it != fs::directory_iterator(); ++it)
    {
        BOOST_TEST(fs::status(it->path()).type() == it->status().type());
        BOOST_TEST(fs::symlink_status(it->path()).type() == it->symlink_status().type());
        if (it->path().filename() == "d1")
        {
            BOOST_TEST(fs::is_directory(it->status()));
            BOOST_TEST(fs::is_directory(it->symlink_status()));
        }
        else if (it->path().filename() == "d2")
        {
            BOOST_TEST(fs::is_directory(it->status()));
            BOOST_TEST(fs::is_directory(it->symlink_status()));
        }
        else if (it->path().filename() == "f0")
        {
            BOOST_TEST(fs::is_regular_file(it->status()));
            BOOST_TEST(fs::is_regular_file(it->symlink_status()));
        }
        else if (it->path().filename() == "f1")
        {
            BOOST_TEST(fs::is_regular_file(it->status()));
            BOOST_TEST(fs::is_regular_file(it->symlink_status()));
        }
        else if (it->path().filename() == "f0_symlink")
        {
            BOOST_TEST(fs::is_regular_file(it->status()));
            BOOST_TEST(fs::is_symlink(it->symlink_status()));
            BOOST_TEST(fs::is_symlink(*it));
        }
        else if (it->path().filename() == "dangling_symlink")
        {
            BOOST_TEST(it->status().type() == fs::file_not_found);
            BOOST_TEST(fs::is_symlink(it->symlink_status()));
        }
        else if (it->path().filename() == "d1_symlink")
        {
            BOOST_TEST(fs::is_directory(it->status()));
            BOOST_TEST(fs::is_symlink(it->symlink_status()));
        }
        else if (it->path().filename() == "dangling_directory_symlink")
        {
            BOOST_TEST(it->status().type() == fs::file_not_found);
            BOOST_TEST(fs::is_symlink(it->symlink_status()));
        }
        //else
        //  cout << "    Note: unexpected directory entry " << it->path().filename() << endl;
    }
}

//  recursive_iterator_status_tests  -------------------------------------------------//

void recursive_iterator_status_tests()
{
    cout << "recursive_iterator_status_tests..." << endl;
    for (fs::recursive_directory_iterator it(dir);
         it != fs::recursive_directory_iterator();
         ++it)
    {
        BOOST_TEST(fs::status(it->path()).type() == it->status().type());
        BOOST_TEST(fs::symlink_status(it->path()).type() == it->symlink_status().type());
    }
}

//  create_hard_link_tests  ----------------------------------------------------------//

void create_hard_link_tests()
{
    cout << "create_hard_link_tests..." << endl;

    fs::path from_ph(dir / "f3");
    fs::path f1x(dir / "f1");

    BOOST_TEST(!fs::exists(from_ph));
    BOOST_TEST(fs::exists(f1x));
    bool create_hard_link_ok(true);
    try
    {
        fs::create_hard_link(f1x, from_ph);
    }
    catch (const fs::filesystem_error& ex)
    {
        create_hard_link_ok = false;
        cout
            << "     *** For information only ***\n"
               "     create_hard_link() attempt failed\n"
               "     filesystem_error.what() reports: "
            << ex.what() << "\n"
                            "     create_hard_link() may not be supported on this file system\n";
    }

    if (create_hard_link_ok)
    {
        cout
            << "     *** For information only ***\n"
               "     create_hard_link() succeeded\n";
        BOOST_TEST(fs::exists(from_ph));
        BOOST_TEST(fs::exists(f1x));
        BOOST_TEST(fs::equivalent(from_ph, f1x));
        BOOST_TEST(fs::hard_link_count(from_ph) == 2);
        BOOST_TEST(fs::hard_link_count(f1x) == 2);
    }

    //  Although tests may be running on a FAT or other file system that does
    //  not support hard links, that is unusual enough that it is considered
    //  a test failure.
    BOOST_TEST(create_hard_link_ok);

    error_code ec;
    fs::create_hard_link(fs::path("doesnotexist"), fs::path("shouldnotwork"), ec);
    BOOST_TEST(ec);
}

//  create_symlink_tests  ------------------------------------------------------------//

void create_symlink_tests()
{
    cout << "create_symlink_tests..." << endl;

    fs::path from_ph(dir / "f4");
    fs::path f1x(dir / "f1");
    BOOST_TEST(!fs::exists(from_ph));
    BOOST_TEST(fs::exists(f1x));
    try
    {
        fs::create_symlink(f1x, from_ph);
    }
    catch (const fs::filesystem_error& ex)
    {
        create_symlink_ok = false;
        cout
            << "     *** For information only ***\n"
               "     create_symlink() attempt failed\n"
               "     filesystem_error.what() reports: "
            << ex.what() << "\n"
                            "     create_symlink() may not be supported on this operating system or file system\n";
    }

    if (create_symlink_ok)
    {
        cout
            << "     *** For information only ***\n"
               "     create_symlink() succeeded\n";
        BOOST_TEST(fs::exists(from_ph));
        BOOST_TEST(fs::is_symlink(from_ph));
        BOOST_TEST(fs::exists(f1x));
        BOOST_TEST(fs::equivalent(from_ph, f1x));
        BOOST_TEST(fs::read_symlink(from_ph) == f1x);

        fs::file_status stat = fs::symlink_status(from_ph);
        BOOST_TEST(fs::exists(stat));
        BOOST_TEST(!fs::is_directory(stat));
        BOOST_TEST(!fs::is_regular_file(stat));
        BOOST_TEST(!fs::is_other(stat));
        BOOST_TEST(fs::is_symlink(stat));

        stat = fs::status(from_ph);
        BOOST_TEST(fs::exists(stat));
        BOOST_TEST(!fs::is_directory(stat));
        BOOST_TEST(fs::is_regular_file(stat));
        BOOST_TEST(!fs::is_other(stat));
        BOOST_TEST(!fs::is_symlink(stat));

        // since create_symlink worked, copy_symlink should also work
        fs::path symlink2_ph(dir / "symlink2");
        fs::copy_symlink(from_ph, symlink2_ph);
        stat = fs::symlink_status(symlink2_ph);
        BOOST_TEST(fs::is_symlink(stat));
        BOOST_TEST(fs::exists(stat));
        BOOST_TEST(!fs::is_directory(stat));
        BOOST_TEST(!fs::is_regular_file(stat));
        BOOST_TEST(!fs::is_other(stat));
    }

    error_code ec = error_code();
    fs::create_symlink("doesnotexist", "", ec);
    BOOST_TEST(ec);
}

//  permissions_tests  ---------------------------------------------------------------//

void permissions_tests()
{
    cout << "permissions_tests..." << endl;

    fs::path p(dir / "permissions.txt");
    create_file(p);

    if (platform == "POSIX")
    {
        cout << "  fs::status(p).permissions() " << std::oct << fs::status(p).permissions()
             << std::dec << endl;
        BOOST_TEST((fs::status(p).permissions() & 0600) == 0600); // 0644, 0664 sometimes returned

        fs::permissions(p, fs::owner_all);
        BOOST_TEST(fs::status(p).permissions() == fs::owner_all);

        fs::permissions(p, fs::add_perms | fs::group_all);
        BOOST_TEST(fs::status(p).permissions() == (fs::owner_all | fs::group_all));

        fs::permissions(p, fs::remove_perms | fs::group_all);
        BOOST_TEST(fs::status(p).permissions() == fs::owner_all);

        // some POSIX platforms cache permissions during directory iteration, some don't
        // so test that iteration finds the correct permissions
        for (fs::directory_iterator itr(dir); itr != fs::directory_iterator(); ++itr)
            if (itr->path().filename() == fs::path("permissions.txt"))
                BOOST_TEST(itr->status().permissions() == fs::owner_all);

        if (create_symlink_ok) // only if symlinks supported
        {
            BOOST_TEST(fs::status(p).permissions() == fs::owner_all);
            fs::path p2(dir / "permissions-symlink.txt");
            fs::create_symlink(p, p2);
            cout << std::oct;
            cout << "   status(p).permissions() " << fs::status(p).permissions() << endl;
            cout << "  status(p2).permissions() " << fs::status(p).permissions() << endl;
            fs::permissions(p2, fs::add_perms | fs::others_read);
            cout << "   status(p).permissions(): " << fs::status(p).permissions() << endl;
            cout << "  status(p2).permissions(): " << fs::status(p2).permissions() << endl;
            cout << std::dec;
        }
    }
    else // Windows
    {
        BOOST_TEST(fs::status(p).permissions() == 0666);
        fs::permissions(p, fs::remove_perms | fs::group_write);
        BOOST_TEST(fs::status(p).permissions() == 0444);
        fs::permissions(p, fs::add_perms | fs::group_write);
        BOOST_TEST(fs::status(p).permissions() == 0666);
    }
}

//  rename_tests  --------------------------------------------------------------------//

void rename_tests()
{
    cout << "rename_tests..." << endl;

    fs::path f1x(dir / "f1");
    BOOST_TEST(fs::exists(f1x));

    // error: rename a non-existent old file
    BOOST_TEST(!fs::exists(d1 / "f99"));
    BOOST_TEST(!fs::exists(d1 / "f98"));
    renamer n1a(d1 / "f99", d1 / "f98");
    BOOST_TEST(CHECK_EXCEPTION(n1a, ENOENT));
    renamer n1b(fs::path(""), d1 / "f98");
    BOOST_TEST(CHECK_EXCEPTION(n1b, ENOENT));

    // error: rename an existing file to ""
    renamer n2(f1x, "");
    BOOST_TEST(CHECK_EXCEPTION(n2, ENOENT));

    // rename an existing file to an existent file
    create_file(dir / "ff1", "ff1");
    create_file(dir / "ff2", "ff2");
    fs::rename(dir / "ff2", dir / "ff1");
    BOOST_TEST(fs::exists(dir / "ff1"));
    verify_file(dir / "ff1", "ff2");
    BOOST_TEST(!fs::exists(dir / "ff2"));

    // rename an existing file to itself
    BOOST_TEST(fs::exists(dir / "f1"));
    fs::rename(dir / "f1", dir / "f1");
    BOOST_TEST(fs::exists(dir / "f1"));

    // error: rename an existing directory to an existing non-empty directory
    BOOST_TEST(fs::exists(dir / "f1"));
    BOOST_TEST(fs::exists(d1 / "f2"));
    // several POSIX implementations (cygwin, openBSD) report ENOENT instead of EEXIST,
    // so we don't verify error type on the following test.
    renamer n3b(dir, d1);
    BOOST_TEST(CHECK_EXCEPTION(n3b, 0));

    //  error: move existing file to a nonexistent parent directory
    BOOST_TEST(!fs::is_directory(dir / "f1"));
    BOOST_TEST(!fs::exists(dir / "d3/f3"));
    renamer n4a(dir / "f1", dir / "d3/f3");
    BOOST_TEST(CHECK_EXCEPTION(n4a, ENOENT));

    // rename existing file in same directory
    BOOST_TEST(fs::exists(d1 / "f2"));
    BOOST_TEST(!fs::exists(d1 / "f50"));
    fs::rename(d1 / "f2", d1 / "f50");
    BOOST_TEST(!fs::exists(d1 / "f2"));
    BOOST_TEST(fs::exists(d1 / "f50"));
    fs::rename(d1 / "f50", d1 / "f2");
    BOOST_TEST(fs::exists(d1 / "f2"));
    BOOST_TEST(!fs::exists(d1 / "f50"));

    // move and rename an existing file to a different directory
    fs::rename(d1 / "f2", d2 / "f3");
    BOOST_TEST(!fs::exists(d1 / "f2"));
    BOOST_TEST(!fs::exists(d2 / "f2"));
    BOOST_TEST(fs::exists(d2 / "f3"));
    BOOST_TEST(!fs::is_directory(d2 / "f3"));
    verify_file(d2 / "f3", "file-f1");
    fs::rename(d2 / "f3", d1 / "f2");
    BOOST_TEST(fs::exists(d1 / "f2"));

    // error: move existing directory to nonexistent parent directory
    BOOST_TEST(fs::exists(d1));
    BOOST_TEST(!fs::exists(dir / "d3/d5"));
    BOOST_TEST(!fs::exists(dir / "d3"));
    renamer n5a(d1, dir / "d3/d5");
    BOOST_TEST(CHECK_EXCEPTION(n5a, ENOENT));

    // rename existing directory
    fs::path d3(dir / "d3");
    BOOST_TEST(fs::exists(d1));
    BOOST_TEST(fs::exists(d1 / "f2"));
    BOOST_TEST(!fs::exists(d3));
    fs::rename(d1, d3);
    BOOST_TEST(!fs::exists(d1));
    BOOST_TEST(fs::exists(d3));
    BOOST_TEST(fs::is_directory(d3));
    BOOST_TEST(!fs::exists(d1 / "f2"));
    BOOST_TEST(fs::exists(d3 / "f2"));
    fs::rename(d3, d1);
    BOOST_TEST(fs::exists(d1));
    BOOST_TEST(fs::exists(d1 / "f2"));
    BOOST_TEST(!fs::exists(d3));

    // rename and move d1 to d2 / "d20"
    BOOST_TEST(fs::exists(d1));
    BOOST_TEST(!fs::exists(d2 / "d20"));
    BOOST_TEST(fs::exists(d1 / "f2"));
    fs::rename(d1, d2 / "d20");
    BOOST_TEST(!fs::exists(d1));
    BOOST_TEST(fs::exists(d2 / "d20"));
    BOOST_TEST(fs::exists(d2 / "d20" / "f2"));
    fs::rename(d2 / "d20", d1);
    BOOST_TEST(fs::exists(d1));
    BOOST_TEST(!fs::exists(d2 / "d20"));
    BOOST_TEST(fs::exists(d1 / "f2"));
}

//  predicate_and_status_tests  ------------------------------------------------------//

void predicate_and_status_tests()
{
    cout << "predicate_and_status_tests..." << endl;

    BOOST_TEST(!fs::exists(ng));
    BOOST_TEST(!fs::is_directory(ng));
    BOOST_TEST(!fs::is_regular_file(ng));
    BOOST_TEST(!fs::is_symlink(ng));
    fs::file_status stat(fs::status(ng));
    BOOST_TEST(fs::type_present(stat));
    BOOST_TEST(fs::permissions_present(stat));
    BOOST_TEST(fs::status_known(stat));
    BOOST_TEST(!fs::exists(stat));
    BOOST_TEST(!fs::is_directory(stat));
    BOOST_TEST(!fs::is_regular_file(stat));
    BOOST_TEST(!fs::is_other(stat));
    BOOST_TEST(!fs::is_symlink(stat));
    stat = fs::status("");
    BOOST_TEST(fs::type_present(stat));
    BOOST_TEST(fs::permissions_present(stat));
    BOOST_TEST(fs::status_known(stat));
    BOOST_TEST(!fs::exists(stat));
    BOOST_TEST(!fs::is_directory(stat));
    BOOST_TEST(!fs::is_regular_file(stat));
    BOOST_TEST(!fs::is_other(stat));
    BOOST_TEST(!fs::is_symlink(stat));

#ifdef BOOST_WINDOWS_API
    stat = fs::status(L"\\System Volume Information");
    BOOST_TEST(fs::type_present(stat));
    BOOST_TEST(fs::permissions_present(stat));
    BOOST_TEST(fs::status_known(stat));
    BOOST_TEST(fs::exists(stat));
    BOOST_TEST(fs::is_directory(stat));
    BOOST_TEST(!fs::is_regular_file(stat));
    BOOST_TEST(!fs::is_other(stat));
    BOOST_TEST(!fs::is_symlink(stat));
#endif // BOOST_WINDOWS_API
}

//  create_directory_tests  ----------------------------------------------------------//

void create_directory_tests()
{
    cout << "create_directory_tests..." << endl;

    error_code ec;
    BOOST_TEST(!fs::create_directory("", ec));
    BOOST_TEST(ec);

#ifdef BOOST_WINDOWS_API
    ec.clear();
    BOOST_TEST(!fs::create_directory(" ", ec)); // OK on Linux
    BOOST_TEST(ec);
#endif

    ec.clear();
    BOOST_TEST(!fs::create_directory("/", ec));
    BOOST_TEST(!ec);
    BOOST_TEST(fs::is_directory("/")); // this is a post-condition

    ec.clear();
    BOOST_TEST(!fs::create_directory(".", ec));
    BOOST_TEST(!ec);

    ec.clear();
    BOOST_TEST(!fs::create_directory("..", ec));
    BOOST_TEST(!ec);

    // create a directory, then check it for consistency
    //   take extra care to report problems, since if this fails
    //   many subsequent tests will fail
    try
    {
        fs::create_directory(dir);
    }

    catch (const fs::filesystem_error& x)
    {
        cout << x.what() << "\n\n"
                            "***** Creating directory "
             << dir << " failed.   *****\n"
                       "***** This is a serious error that will prevent further tests    *****\n"
                       "***** from returning useful results. Further testing is aborted. *****\n\n";
        std::exit(1);
    }

    catch (...)
    {
        cout << "\n\n"
                "***** Creating directory "
             << dir << " failed.   *****\n"
                       "***** This is a serious error that will prevent further tests    *****\n"
                       "***** from returning useful results. Further testing is aborted. *****\n\n";
        std::exit(1);
    }

    BOOST_TEST(fs::exists(dir));
    BOOST_TEST(fs::is_empty(dir));
    BOOST_TEST(fs::is_directory(dir));
    BOOST_TEST(!fs::is_regular_file(dir));
    BOOST_TEST(!fs::is_other(dir));
    BOOST_TEST(!fs::is_symlink(dir));
    fs::file_status stat = fs::status(dir);
    BOOST_TEST(fs::exists(stat));
    BOOST_TEST(fs::is_directory(stat));
    BOOST_TEST(!fs::is_regular_file(stat));
    BOOST_TEST(!fs::is_other(stat));
    BOOST_TEST(!fs::is_symlink(stat));

    cout << "  create_directory_tests complete" << endl;
}

//  current_directory_tests  ---------------------------------------------------------//

void current_directory_tests()
{
    cout << "current_directory_tests..." << endl;

    // set the current directory, then check it for consistency
    fs::path original_dir = fs::current_path();
    BOOST_TEST(dir != original_dir);
    fs::current_path(dir);
    BOOST_TEST(fs::current_path() == dir);
    BOOST_TEST(fs::current_path() != original_dir);
    fs::current_path(original_dir);
    BOOST_TEST(fs::current_path() == original_dir);
    BOOST_TEST(fs::current_path() != dir);

    // make sure the overloads work
    fs::current_path(dir.c_str());
    BOOST_TEST(fs::current_path() == dir);
    BOOST_TEST(fs::current_path() != original_dir);
    fs::current_path(original_dir.string());
    BOOST_TEST(fs::current_path() == original_dir);
    BOOST_TEST(fs::current_path() != dir);
}

//  create_directories_tests  --------------------------------------------------------//

void create_directories_tests()
{
    cout << "create_directories_tests..." << endl;

    error_code ec;
    BOOST_TEST(!fs::create_directories("", ec));
    BOOST_TEST(ec);

#ifdef BOOST_WINDOWS_API
    // Windows only test, since " " is OK on Linux as a directory name
    ec.clear();
    BOOST_TEST(!fs::create_directories(" ", ec));
    BOOST_TEST(ec);
#endif

    ec.clear();
    BOOST_TEST(!fs::create_directories("/", ec));
    BOOST_TEST(!ec);

    ec.clear();
    BOOST_TEST(!fs::create_directories(".", ec));
    BOOST_TEST(!ec);

    ec.clear();
    BOOST_TEST(!fs::create_directories("..", ec));
    BOOST_TEST(!ec);

#ifdef BOOST_POSIX_API
    if (access("/", W_OK) != 0)
    {
        ec.clear();
        BOOST_TEST(!fs::create_directories("/foo", ec)); // may be OK on Windows
                                                         //  but unlikely to be OK on POSIX, unless running as root
        BOOST_TEST(ec);
    }
#endif

    fs::path p = dir / "level1/." / "level2/./.." / "level3/";
    // trailing "/.", "/./..", and "/" in the above elements test ticket #7258 and
    // related issues

    cout << "    p is " << p << endl;
    BOOST_TEST(!fs::exists(p));
    BOOST_TEST(fs::create_directories(p));
    BOOST_TEST(fs::exists(p));
    BOOST_TEST(fs::is_directory(p));

    if (fs::exists("/permissions_test"))
    {
        BOOST_TEST(!fs::create_directories("/permissions_test", ec));
        BOOST_TEST(!fs::create_directories("/permissions_test/another_directory", ec));
        BOOST_TEST(ec);
    }
}

//  resize_file_tests  ---------------------------------------------------------------//

void resize_file_tests()
{
    cout << "resize_file_tests..." << endl;

    fs::path p(dir / "resize_file_test.txt");

    fs::remove(p);
    create_file(p, "1234567890");

    BOOST_TEST(fs::exists(p));
    BOOST_TEST_EQ(fs::file_size(p), 10U);
    fs::resize_file(p, 5);
    BOOST_TEST(fs::exists(p));
    BOOST_TEST_EQ(fs::file_size(p), 5U);
    fs::resize_file(p, 15);
    BOOST_TEST(fs::exists(p));
    BOOST_TEST_EQ(fs::file_size(p), 15U);

    error_code ec;
    fs::resize_file("no such file", 15, ec);
    BOOST_TEST(ec);
}

//  status_of_nonexistent_tests  -----------------------------------------------------//

void status_of_nonexistent_tests()
{
    cout << "status_of_nonexistent_tests..." << endl;
    fs::path p("nosuch");
    BOOST_TEST(!fs::exists(p));
    BOOST_TEST(!fs::is_regular_file(p));
    BOOST_TEST(!fs::is_directory(p));
    BOOST_TEST(!fs::is_symlink(p));
    BOOST_TEST(!fs::is_other(p));

    fs::file_status s = fs::status(p);
    BOOST_TEST(!fs::exists(s));
    BOOST_TEST_EQ(s.type(), fs::file_not_found);
    BOOST_TEST(fs::type_present(s));
    BOOST_TEST(!fs::is_regular_file(s));
    BOOST_TEST(!fs::is_directory(s));
    BOOST_TEST(!fs::is_symlink(s));
    BOOST_TEST(!fs::is_other(s));

    // ticket #12574 was just user confusion, but are the tests are worth keeping
    error_code ec;
    BOOST_TEST(!fs::is_directory(dir / "no-such-directory", ec));
    BOOST_TEST(ec);
    //cout << "error_code value: " << ec.value() << endl;
    ec.clear();
    BOOST_TEST(!fs::is_directory(dir / "no-such-directory" / "bar", ec));
    BOOST_TEST(ec);
    //cout << "error_code value: " << ec.value() << endl;
}

//  status_error_reporting_tests  ----------------------------------------------------//

void status_error_reporting_tests()
{
    cout << "status_error_reporting_tests..." << endl;

    error_code ec;

    // test status, ec, for existing file
    ec.assign(-1, poison_category());
    BOOST_TEST(ec.value() == -1);
    BOOST_TEST(&ec.category() == &poison_category());
    fs::file_status s = fs::status(".", ec);
    BOOST_TEST(ec.value() == 0);
    BOOST_TEST(ec.category() == system_category());
    BOOST_TEST(fs::exists(s));
    BOOST_TEST(fs::is_directory(s));

    // test status, ec, for non-existing file
    fs::path p("nosuch");
    ec.assign(-1, poison_category());
    s = fs::status(p, ec);
    BOOST_TEST(ec.value() != 0);
    BOOST_TEST(ec.category() == system_category());

    BOOST_TEST(!fs::exists(s));
    BOOST_TEST_EQ(s.type(), fs::file_not_found);
    BOOST_TEST(fs::type_present(s));
    BOOST_TEST(!fs::is_regular_file(s));
    BOOST_TEST(!fs::is_directory(s));
    BOOST_TEST(!fs::is_symlink(s));
    BOOST_TEST(!fs::is_other(s));

    // test queries, ec, for existing file
    ec.assign(-1, poison_category());
    BOOST_TEST(fs::exists(".", ec));
    BOOST_TEST(ec.value() == 0);
    BOOST_TEST(ec.category() == system_category());
    ec.assign(-1, poison_category());
    BOOST_TEST(!fs::is_regular_file(".", ec));
    BOOST_TEST(ec.value() == 0);
    BOOST_TEST(ec.category() == system_category());
    ec.assign(-1, poison_category());
    BOOST_TEST(fs::is_directory(".", ec));
    BOOST_TEST(ec.value() == 0);
    BOOST_TEST(ec.category() == system_category());

    // test queries, ec, for non-existing file
    ec.assign(-1, poison_category());
    BOOST_TEST(!fs::exists(p, ec));
    BOOST_TEST(ec.value() != 0);
    BOOST_TEST(ec.category() == system_category());
    ec.assign(-1, poison_category());
    BOOST_TEST(!fs::is_regular_file(p, ec));
    BOOST_TEST(ec.value() != 0);
    BOOST_TEST(ec.category() == system_category());
    ec.assign(-1, poison_category());
    BOOST_TEST(!fs::is_directory(p, ec));
    BOOST_TEST(ec.value() != 0);
    BOOST_TEST(ec.category() == system_category());
}

//  remove_tests  --------------------------------------------------------------------//

void remove_tests(const fs::path& dirx)
{
    cout << "remove_tests..." << endl;

    // remove() file
    fs::path f1x = dirx / "shortlife";
    BOOST_TEST(!fs::exists(f1x));
    create_file(f1x, "");
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::is_directory(f1x));
    BOOST_TEST(fs::remove(f1x));
    BOOST_TEST(!fs::exists(f1x));
    BOOST_TEST(!fs::remove("no-such-file"));
    BOOST_TEST(!fs::remove("no-such-directory/no-such-file"));

#if defined(BOOST_WINDOWS_API)
    // remove() read-only file
    BOOST_TEST(!fs::exists(f1x));
    create_file(f1x, "");
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::is_directory(f1x));
    set_read_only(f1x);
    BOOST_TEST(fs::remove(f1x));
    BOOST_TEST(!fs::exists(f1x));
#endif // defined(BOOST_WINDOWS_API)

    // remove() directory
    fs::path d1x = dirx / "shortlife_dir";
    BOOST_TEST(!fs::exists(d1x));
    fs::create_directory(d1x);
    BOOST_TEST(fs::exists(d1x));
    BOOST_TEST(fs::is_directory(d1x));
    BOOST_TEST(fs::is_empty(d1x));
    bad_remove_dir = dirx;
    BOOST_TEST(CHECK_EXCEPTION(bad_remove, ENOTEMPTY));
    BOOST_TEST(fs::remove(d1x));
    BOOST_TEST(!fs::exists(d1x));
}

//  remove_symlink_tests  ------------------------------------------------------------//

void remove_symlink_tests()
{
    cout << "remove_symlink_tests..." << endl;

    // remove() dangling symbolic link
    fs::path link = dir / "dangling_link";
    fs::remove(link); // remove any residue from past tests
    BOOST_TEST(!fs::is_symlink(link));
    BOOST_TEST(!fs::exists(link));
    fs::create_symlink("nowhere", link);
    BOOST_TEST(!fs::exists(link));
    BOOST_TEST(fs::is_symlink(link));
    BOOST_TEST(fs::remove(link));
    BOOST_TEST(!fs::is_symlink(link));

    // remove() self-refering symbolic link
    link = dir / "link_to_self";
    fs::remove(link); // remove any residue from past tests
    BOOST_TEST(!fs::is_symlink(link));
    BOOST_TEST(!fs::exists(link));
    fs::create_symlink(link, link);
    BOOST_TEST(fs::remove(link));
    BOOST_TEST(!fs::exists(link));
    BOOST_TEST(!fs::is_symlink(link));

    // remove() cyclic symbolic link
    link = dir / "link_to_a";
    fs::path link2 = dir / "link_to_b";
    fs::remove(link);  // remove any residue from past tests
    fs::remove(link2); // remove any residue from past tests
    BOOST_TEST(!fs::is_symlink(link));
    BOOST_TEST(!fs::exists(link));
    fs::create_symlink(link, link2);
    fs::create_symlink(link2, link);
    BOOST_TEST(fs::remove(link));
    BOOST_TEST(fs::remove(link2));
    BOOST_TEST(!fs::exists(link));
    BOOST_TEST(!fs::exists(link2));
    BOOST_TEST(!fs::is_symlink(link));

    // remove() symbolic link to file
    fs::path f1x = dir / "link_target";
    fs::remove(f1x); // remove any residue from past tests
    BOOST_TEST(!fs::exists(f1x));
    create_file(f1x, "");
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::is_directory(f1x));
    BOOST_TEST(fs::is_regular_file(f1x));
    link = dir / "non_dangling_link";
    fs::create_symlink(f1x, link);
    BOOST_TEST(fs::exists(link));
    BOOST_TEST(!fs::is_directory(link));
    BOOST_TEST(fs::is_regular_file(link));
    BOOST_TEST(fs::is_symlink(link));
    BOOST_TEST(fs::remove(link));
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::exists(link));
    BOOST_TEST(!fs::is_symlink(link));
    BOOST_TEST(fs::remove(f1x));
    BOOST_TEST(!fs::exists(f1x));
}

//  remove_all_tests  ----------------------------------------------------------------//

void remove_all_tests(const fs::path& dirx)
{
    cout << "remove_all_tests..." << endl;

    // remove_all() file
    {
        fs::path f1x = dirx / "shortlife";
        BOOST_TEST(!fs::exists(f1x));
        create_file(f1x, "");
        BOOST_TEST(fs::exists(f1x));
        BOOST_TEST(!fs::is_directory(f1x));
        BOOST_TEST_EQ(fs::remove_all(f1x), 1u);
        BOOST_TEST(!fs::exists(f1x));
        BOOST_TEST_EQ(fs::remove_all("no-such-file"), 0u);
        BOOST_TEST_EQ(fs::remove_all("no-such-directory/no-such-file"), 0u);
    }

    // remove_all() directory tree
    {
        unsigned int created_count = 0u;
        fs::path d1x = dirx / "shortlife_dir";
        BOOST_TEST(!fs::exists(d1x));
        fs::create_directory(d1x);
        ++created_count;
        BOOST_TEST(fs::exists(d1x));
        BOOST_TEST(fs::is_directory(d1x));

        fs::path d2x = d1x / "nested_dir";
        BOOST_TEST(!fs::exists(d2x));
        fs::create_directory(d2x);
        ++created_count;
        BOOST_TEST(fs::exists(d2x));
        BOOST_TEST(fs::is_directory(d2x));

        fs::path f1x = d1x / "shortlife";
        BOOST_TEST(!fs::exists(f1x));
        create_file(f1x, "");
        ++created_count;
        BOOST_TEST(fs::exists(f1x));
        BOOST_TEST(!fs::is_directory(f1x));

#if defined(BOOST_WINDOWS_API)
        // read-only file
        fs::path f2x = d1x / "shortlife_ro";
        BOOST_TEST(!fs::exists(f2x));
        create_file(f2x, "");
        ++created_count;
        BOOST_TEST(fs::exists(f2x));
        BOOST_TEST(!fs::is_directory(f2x));
        set_read_only(f2x);
#endif // defined(BOOST_WINDOWS_API)

        mars_boost::uintmax_t removed_count = fs::remove_all(d1x);
        BOOST_TEST_EQ(removed_count, created_count);

        BOOST_TEST(!fs::exists(d1x));
    }
}

//  remove_all_symlink_tests  --------------------------------------------------------//

void remove_all_symlink_tests(const fs::path& dirx)
{
    cout << "remove_all_symlink_tests..." << endl;

    // External directory tree
    fs::path d1x = dirx / "shortlife_dir1";
    BOOST_TEST(!fs::exists(d1x));
    fs::create_directory(d1x);
    BOOST_TEST(fs::exists(d1x));
    BOOST_TEST(fs::is_directory(d1x));

    fs::path f1x = d1x / "shortlife1";
    BOOST_TEST(!fs::exists(f1x));
    create_file(f1x, "");
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::is_directory(f1x));

    fs::path f2x = d1x / "shortlife2";
    BOOST_TEST(!fs::exists(f2x));
    create_file(f2x, "");
    BOOST_TEST(fs::exists(f2x));
    BOOST_TEST(!fs::is_directory(f2x));

    // remove_all() directory tree that has symlinks to external directories
    unsigned int created_count = 0u;
    fs::path d2x = dirx / "shortlife_dir2";
    BOOST_TEST(!fs::exists(d2x));
    fs::create_directory(d2x);
    ++created_count;
    BOOST_TEST(fs::exists(d2x));
    BOOST_TEST(fs::is_directory(d2x));

    fs::path f3x = d2x / "shortlife";
    BOOST_TEST(!fs::exists(f3x));
    create_file(f3x, "");
    ++created_count;
    BOOST_TEST(fs::exists(f3x));
    BOOST_TEST(!fs::is_directory(f3x));

    fs::path d3x = d2x / "symlink_dir";
    BOOST_TEST(!fs::exists(d3x));
    fs::create_directory_symlink(d1x, d3x);
    ++created_count;
    BOOST_TEST(fs::exists(d3x));
    BOOST_TEST(fs::is_symlink(d3x));

#if defined(BOOST_FILESYSTEM_HAS_MKLINK)
    fs::path junc = d2x / "junc";
    fs::path cur_path(fs::current_path());
    fs::current_path(d2x);
    BOOST_TEST(std::system("mklink /J junc ..\\shortlife_dir1") == 0);
    fs::current_path(cur_path);
    ++created_count;
    BOOST_TEST(fs::exists(junc));
#endif

    fs::path f4x = d2x / "symlink";
    BOOST_TEST(!fs::exists(f4x));
    fs::create_symlink(f1x, f4x);
    ++created_count;
    BOOST_TEST(fs::exists(f4x));
    BOOST_TEST(fs::is_symlink(f4x));

    fs::path f5x = d2x / "hardlink";
    BOOST_TEST(!fs::exists(f5x));
    fs::create_hard_link(f2x, f5x);
    ++created_count;
    BOOST_TEST(fs::exists(f5x));
    BOOST_TEST(!fs::is_directory(f5x));

    mars_boost::uintmax_t removed_count = fs::remove_all(d2x);
    BOOST_TEST_EQ(removed_count, created_count);

    BOOST_TEST(!fs::exists(d2x));

    // Check that external directory and file are intact
    BOOST_TEST(fs::exists(d1x));
    BOOST_TEST(fs::is_directory(d1x));
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::is_directory(f1x));
    BOOST_TEST(fs::exists(f2x));
    BOOST_TEST(!fs::is_directory(f2x));

    // Cleanup
    fs::remove_all(d1x);
}

//  absolute_tests  -----------------------------------------------------------------//

void absolute_tests()
{
    cout << "absolute_tests..." << endl;

    BOOST_TEST_EQ(fs::absolute(""), fs::current_path());
    BOOST_TEST_EQ(fs::absolute("", ""), fs::current_path());
    BOOST_TEST_EQ(fs::absolute(fs::current_path() / "foo/bar"), fs::current_path() / "foo/bar");
    BOOST_TEST_EQ(fs::absolute("foo"), fs::current_path() / "foo");
    BOOST_TEST_EQ(fs::absolute("foo", fs::current_path()), fs::current_path() / "foo");
    BOOST_TEST_EQ(fs::absolute("bar", "foo"), fs::current_path() / "foo" / "bar");
    BOOST_TEST_EQ(fs::absolute("/foo"), fs::current_path().root_path().string() + "foo");

#ifdef BOOST_WINDOWS_API
    BOOST_TEST_EQ(fs::absolute("a:foo", "b:/bar"), fs::path(L"a:/bar/foo"));
#endif

    // these tests were moved from elsewhere, so may duplicate some of the above tests

    // p.empty()
    BOOST_TEST_EQ(fs::absolute(fs::path(), "//foo/bar"), fs::path("//foo/bar"));
    if (platform == "Windows")
    {
        BOOST_TEST_EQ(fs::absolute(fs::path(), "a:/bar"), fs::path("a:/bar"));
    }

    // p.has_root_name()
    //   p.has_root_directory()
    BOOST_TEST_EQ(fs::absolute(fs::path("//foo/bar"), "//uvw/xyz"), fs::path("//foo/bar"));
    if (platform == "Windows")
    {
        BOOST_TEST_EQ(fs::absolute(fs::path("a:/bar"), "b:/xyz"), fs::path("a:/bar"));
    }
    //   !p.has_root_directory()
    BOOST_TEST_EQ(fs::absolute(fs::path("//net"), "//xyz/"), fs::path("//net/"));
    BOOST_TEST_EQ(fs::absolute(fs::path("//net"), "//xyz/abc"), fs::path("//net/abc"));
    BOOST_TEST_EQ(fs::absolute(fs::path("//net"), "//xyz/abc/def"), fs::path("//net/abc/def"));
    if (platform == "Windows")
    {
        BOOST_TEST_EQ(fs::absolute(fs::path("a:"), "b:/"), fs::path("a:/"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:"), "b:/abc"), fs::path("a:/abc"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:"), "b:/abc/def"), fs::path("a:/abc/def"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:foo"), "b:/"), fs::path("a:/foo"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:foo"), "b:/abc"), fs::path("a:/abc/foo"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:foo"), "b:/abc/def"), fs::path("a:/abc/def/foo"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:foo/bar"), "b:/"), fs::path("a:/foo/bar"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:foo/bar"), "b:/abc"), fs::path("a:/abc/foo/bar"));
        BOOST_TEST_EQ(fs::absolute(fs::path("a:foo/bar"), "b:/abc/def"), fs::path("a:/abc/def/foo/bar"));
        BOOST_TEST_EQ(fs::absolute(fs::path("\\\\net\\share\\folder"), "c:\\"), fs::path("\\\\net\\share\\folder"));
    }
    // !p.has_root_name()
    //   p.has_root_directory()
#ifdef BOOST_WINDOWS_API
    BOOST_TEST_EQ(fs::absolute(fs::path("/"), "//xyz/"), fs::path("//xyz/"));
    BOOST_TEST_EQ(fs::absolute(fs::path("/"), "//xyz/abc"), fs::path("//xyz/"));
    BOOST_TEST_EQ(fs::absolute(fs::path("/foo"), "//xyz/"), fs::path("//xyz/foo"));
    BOOST_TEST_EQ(fs::absolute(fs::path("/foo"), "//xyz/abc"), fs::path("//xyz/foo"));
#else
    BOOST_TEST_EQ(fs::absolute(fs::path("/"), "//xyz/"), fs::path("/"));
    BOOST_TEST_EQ(fs::absolute(fs::path("/"), "//xyz/abc"), fs::path("/"));
    BOOST_TEST_EQ(fs::absolute(fs::path("/foo"), "//xyz/"), fs::path("/foo"));
    BOOST_TEST_EQ(fs::absolute(fs::path("/foo"), "//xyz/abc"), fs::path("/foo"));
#endif
    //   !p.has_root_directory()
    BOOST_TEST_EQ(fs::absolute(fs::path("foo"), "//xyz/abc"), fs::path("//xyz/abc/foo"));
    BOOST_TEST_EQ(fs::absolute(fs::path("foo/bar"), "//xyz/abc"), fs::path("//xyz/abc/foo/bar"));
    BOOST_TEST_EQ(fs::absolute(fs::path("."), "//xyz/abc"), fs::path("//xyz/abc/."));
    BOOST_TEST_EQ(fs::absolute(fs::path(".."), "//xyz/abc"), fs::path("//xyz/abc/.."));
    BOOST_TEST_EQ(fs::absolute(fs::path("./foo"), "//xyz/abc"), fs::path("//xyz/abc/./foo"));
    BOOST_TEST_EQ(fs::absolute(fs::path("../foo"), "//xyz/abc"), fs::path("//xyz/abc/../foo"));
    if (platform == "POSIX")
    {
        BOOST_TEST_EQ(fs::absolute(fs::path("foo"), "/abc"), fs::path("/abc/foo"));
        BOOST_TEST_EQ(fs::absolute(fs::path("foo/bar"), "/abc"), fs::path("/abc/foo/bar"));
        BOOST_TEST_EQ(fs::absolute(fs::path("."), "/abc"), fs::path("/abc/."));
        BOOST_TEST_EQ(fs::absolute(fs::path(".."), "/abc"), fs::path("/abc/.."));
        BOOST_TEST_EQ(fs::absolute(fs::path("./foo"), "/abc"), fs::path("/abc/./foo"));
        BOOST_TEST_EQ(fs::absolute(fs::path("../foo"), "/abc"), fs::path("/abc/../foo"));
    }
}

//  canonical_basic_tests  -----------------------------------------------------------//

void canonical_basic_tests()
{
    cout << "canonical_basic_tests..." << endl;

    // error handling
    error_code ec;
    ec.clear();
    fs::canonical("no-such-file", ec);
    BOOST_TEST(ec);
    ec.clear();
    fs::canonical("no-such-file", "x", ec);
    BOOST_TEST(ec);
    bool ok(false);
    try
    {
        fs::canonical("no-such-file");
    }
    catch (const fs::filesystem_error&)
    {
        ok = true;
    }
    BOOST_TEST(ok);

    // non-symlink tests; also see canonical_symlink_tests()
    BOOST_TEST_EQ(fs::canonical(""), fs::current_path());
    BOOST_TEST_EQ(fs::canonical("", fs::current_path()), fs::current_path());
    BOOST_TEST_EQ(fs::canonical("", ""), fs::current_path());
    BOOST_TEST_EQ(fs::canonical(fs::current_path()), fs::current_path());
    BOOST_TEST_EQ(fs::canonical(fs::current_path(), ""), fs::current_path());
    BOOST_TEST_EQ(fs::canonical(fs::current_path(), "no-such-file"), fs::current_path());

    BOOST_TEST_EQ(fs::canonical("."), fs::current_path());
    BOOST_TEST_EQ(fs::canonical(".."), fs::current_path().parent_path());
    BOOST_TEST_EQ(fs::canonical("/"), fs::current_path().root_path());

    fs::path relative_dir(dir.filename());
    BOOST_TEST_EQ(fs::canonical(dir), dir);
    BOOST_TEST_EQ(fs::canonical(relative_dir), dir);
    BOOST_TEST_EQ(fs::canonical(dir / "f0"), dir / "f0");
    BOOST_TEST_EQ(fs::canonical(relative_dir / "f0"), dir / "f0");
    BOOST_TEST_EQ(fs::canonical(relative_dir / "./f0"), dir / "f0");
    BOOST_TEST_EQ(fs::canonical(relative_dir / "d1/../f0"), dir / "f0");

    // treat parent of root as itself on both POSIX and Windows
    fs::path init(fs::initial_path());
    fs::path root(init.root_path());
    fs::path::const_iterator it(init.begin());
    fs::path first; // relative first non-root directory
#ifdef BOOST_WINDOWS_API
    if (!init.empty())
        ++it;
#endif
    if (++it != init.end())
        first = *it;
    fs::path expected(root / first);

    cout << "  init: " << init << endl;
    cout << "  root: " << root << endl;
    cout << "  first: " << first << endl;
    cout << "  expected: " << expected << endl;

    //  ticket 10187 tests
    BOOST_TEST_EQ(fs::canonical(root / "../.." / first), expected);
    BOOST_TEST_EQ(fs::canonical(fs::path("../..") / first, root), expected);
    BOOST_TEST_EQ(fs::canonical(fs::path("/../..") / first, fs::current_path().root_name()), expected);

    //  ticket 9683 test
    BOOST_TEST_EQ(fs::canonical(root / first / "../../../../.."), root);

#ifdef BOOST_WINDOWS_API
    // Test Windows long paths
    fs::path long_path = make_long_path(dir / L"f0");
    BOOST_TEST_EQ(fs::canonical(long_path), long_path);
#endif
}

//  canonical_symlink_tests  -----------------------------------------------------------//

void canonical_symlink_tests()
{
    cout << "canonical_symlink_tests..." << endl;

    fs::path relative_dir(dir.filename());
    BOOST_TEST_EQ(fs::canonical(dir / "sym-d1/f2"), d1 / "f2");
    BOOST_TEST_EQ(fs::canonical(relative_dir / "sym-d1/f2"), d1 / "f2");
}

//  copy_file_tests  ------------------------------------------------------------------//

void copy_file_tests(const fs::path& f1x, const fs::path& d1x)
{
    cout << "copy_file_tests..." << endl;

    BOOST_TEST(fs::exists(f1x));
    fs::remove(d1x / "f2"); // remove possible residue from prior testing
    BOOST_TEST(fs::exists(d1x));
    BOOST_TEST(!fs::exists(d1x / "f2"));
    cout << " copy " << f1x << " to " << d1x / "f2" << endl;
    bool file_copied = fs::copy_file(f1x, d1x / "f2");
    cout << " copy complete" << endl;
    BOOST_TEST(file_copied);
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(fs::exists(d1x / "f2"));
    BOOST_TEST(!fs::is_directory(d1x / "f2"));
    verify_file(d1x / "f2", "file-f1");

    bool copy_ex_ok = false;
    file_copied = false;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2");
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = true;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(!file_copied);

    file_copied = false;
    copy_ex_ok = false;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2", fs::copy_options::none);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = true;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(!file_copied);

    fs::remove(d1x / "f2");
    create_file(d1x / "f2", "1234567890");
    BOOST_TEST_EQ(fs::file_size(d1x / "f2"), 10U);
    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2", fs::copy_options::skip_existing);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(!file_copied);
    BOOST_TEST_EQ(fs::file_size(d1x / "f2"), 10U);
    verify_file(d1x / "f2", "1234567890");

    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2-non-existing", fs::copy_options::skip_existing);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(file_copied);
    BOOST_TEST_EQ(fs::file_size(d1x / "f2-non-existing"), 7U);
    verify_file(d1x / "f2-non-existing", "file-f1");
    fs::remove(d1x / "f2-non-existing");

    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2", fs::copy_options::update_existing);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(!file_copied);
    BOOST_TEST_EQ(fs::file_size(d1x / "f2"), 10U);
    verify_file(d1x / "f2", "1234567890");

    // Sleep for a while so that the last modify time is more recent for new files
#if defined(BOOST_POSIX_API)
    sleep(2);
#else
    Sleep(2000);
#endif

    create_file(d1x / "f2-more-recent", "x");
    BOOST_TEST_EQ(fs::file_size(d1x / "f2-more-recent"), 1U);
    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(d1x / "f2-more-recent", d1x / "f2", fs::copy_options::update_existing);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(file_copied);
    BOOST_TEST_EQ(fs::file_size(d1x / "f2"), 1U);
    verify_file(d1x / "f2", "x");
    fs::remove(d1x / "f2-more-recent");

    fs::remove(d1x / "f2");
    create_file(d1x / "f2", "1234567890");
    BOOST_TEST_EQ(fs::file_size(d1x / "f2"), 10U);
    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2", fs::copy_options::overwrite_existing);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(file_copied);
    BOOST_TEST_EQ(fs::file_size(d1x / "f2"), 7U);
    verify_file(d1x / "f2", "file-f1");

    fs::remove(d1x / "f2");
    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2", fs::copy_options::synchronize_data);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(file_copied);
    verify_file(d1x / "f2", "file-f1");

    fs::remove(d1x / "f2");
    file_copied = false;
    copy_ex_ok = true;
    try
    {
        file_copied = fs::copy_file(f1x, d1x / "f2", fs::copy_options::synchronize);
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = false;
    }
    BOOST_TEST(copy_ex_ok);
    BOOST_TEST(file_copied);
    verify_file(d1x / "f2", "file-f1");

    // Test copy_file with special files with generated content. Such files have zero size,
    // but have contents.
    if (fs::is_regular_file("/proc/self/cmdline"))
    {
        file_copied = false;
        copy_ex_ok = true;
        try
        {
            file_copied = fs::copy_file("/proc/self/cmdline", d1x / "cmdline");
        }
        catch (const fs::filesystem_error&)
        {
            copy_ex_ok = false;
        }
        BOOST_TEST(copy_ex_ok);
        BOOST_TEST(file_copied);
        BOOST_TEST_GT(fs::file_size(d1x / "cmdline"), 0u);
    }

#ifdef BOOST_WINDOWS_API
    // Test copying files with multiple NTFS streams
    fs::path multi_stream_path = d1x / "multi-stream";
    fs::path multi_stream_alt_path = d1x / "multi-stream:alt-stream";
    fs::path multi_stream_copy_path = d1x / "multi-stream-copy";
    fs::path multi_stream_alt_copy_path = d1x / "multi-stream-copy:alt-stream";
    create_file(multi_stream_path, "multi-stream:default");
    try
    {
        // Check that the filesystem supports alternate streams
        create_file(multi_stream_alt_path, "multi-stream:alternate");
        verify_file(multi_stream_alt_path, "multi-stream:alternate");

        fs::remove(multi_stream_copy_path);
        copy_ex_ok = true;
        file_copied = false;
        try
        {
            file_copied = fs::copy_file(multi_stream_path, multi_stream_copy_path);
        }
        catch (const fs::filesystem_error&)
        {
            copy_ex_ok = false;
        }
        BOOST_TEST(copy_ex_ok);
        BOOST_TEST(file_copied);
        verify_file(multi_stream_copy_path, "multi-stream:default");
        verify_file(multi_stream_alt_copy_path, "multi-stream:alternate");

        fs::remove(multi_stream_copy_path);
        copy_ex_ok = true;
        file_copied = false;
        try
        {
            file_copied = fs::copy_file(multi_stream_path, multi_stream_copy_path, fs::copy_options::synchronize_data);
        }
        catch (const fs::filesystem_error&)
        {
            copy_ex_ok = false;
        }
        BOOST_TEST(copy_ex_ok);
        BOOST_TEST(file_copied);
        verify_file(multi_stream_copy_path, "multi-stream:default");
        verify_file(multi_stream_alt_copy_path, "multi-stream:alternate");

        fs::remove(multi_stream_copy_path);
        copy_ex_ok = true;
        file_copied = false;
        try
        {
            file_copied = fs::copy_file(multi_stream_path, multi_stream_copy_path, fs::copy_options::synchronize);
        }
        catch (const fs::filesystem_error&)
        {
            copy_ex_ok = false;
        }
        BOOST_TEST(copy_ex_ok);
        BOOST_TEST(file_copied);
        verify_file(multi_stream_copy_path, "multi-stream:default");
        verify_file(multi_stream_alt_copy_path, "multi-stream:alternate");

        fs::remove(multi_stream_copy_path);
    }
    catch (const fs::filesystem_error& e)
    {
        cout << "Multiple streams per file are not supported: " << e.what() << "\nSkipping multi-stream tests..." << endl;
    }
#endif // BOOST_WINDOWS_API
}

//  symlink_status_tests  -------------------------------------------------------------//

void symlink_status_tests()
{
    cout << "symlink_status_tests..." << endl;

    mars_boost::system::error_code ec;

    fs::path dangling_sym(dir / "dangling-sym");
    fs::path dangling_directory_sym(dir / "dangling-directory-sym");
    fs::path sym_d1(dir / "sym-d1");
    fs::path symsym_d1(dir / "symsym-d1");
    fs::path sym_f1(dir / "sym-f1");
    fs::path symsym_f1(dir / "symsym-f1");
    fs::create_symlink("does not exist", dangling_sym);
    fs::create_directory_symlink("does not exist", dangling_directory_sym);
    fs::create_directory_symlink(d1, sym_d1);
    fs::create_directory_symlink(sym_d1, symsym_d1);
    fs::create_symlink(f1, sym_f1);
    fs::create_symlink(sym_f1, symsym_f1);

    //  verify all cases detected as symlinks
    BOOST_TEST_EQ(fs::symlink_status(dangling_sym, ec).type(), fs::symlink_file);
    BOOST_TEST_EQ(fs::symlink_status(dangling_directory_sym, ec).type(), fs::symlink_file);
    BOOST_TEST_EQ(fs::symlink_status(sym_d1, ec).type(), fs::symlink_file);
    BOOST_TEST_EQ(fs::symlink_status(symsym_d1, ec).type(), fs::symlink_file);
    BOOST_TEST_EQ(fs::symlink_status(sym_f1, ec).type(), fs::symlink_file);
    BOOST_TEST_EQ(fs::symlink_status(symsym_f1, ec).type(), fs::symlink_file);

    //  verify all cases resolve to the (possibly recursive) symlink target
    BOOST_TEST_EQ(fs::status(dangling_sym, ec).type(), fs::file_not_found);
    BOOST_TEST_EQ(fs::status(dangling_directory_sym, ec).type(), fs::file_not_found);

    BOOST_TEST_EQ(fs::status(sym_d1, ec).type(), fs::directory_file);
    BOOST_TEST_EQ(fs::status(sym_d1 / "d1f1", ec).type(), fs::regular_file);
    BOOST_TEST_EQ(fs::status(symsym_d1, ec).type(), fs::directory_file);
    BOOST_TEST_EQ(fs::status(symsym_d1 / "d1f1", ec).type(), fs::regular_file);
    BOOST_TEST_EQ(fs::status(sym_f1, ec).type(), fs::regular_file);
    BOOST_TEST_EQ(fs::status(symsym_f1, ec).type(), fs::regular_file);

#ifdef BOOST_WINDOWS_API

    //  On Windows, telling if a filesystem entry is a symlink (or junction which is
    //  treated as a symlink), rather than some other kind of reparse point, requires some
    //  baroque code. See ticket #4663, filesystem objects falsely identified as symlinks.
    //  This test checks two directory entries created by Windows itself to verify
    //  is_symlink() works correctly. Try "dir /A %HOMEPATH%\.." from the command line to
    //  verify this test is valid on your version of Windows. It only works on Vista and
    //  later.

    fs::path users(getenv("HOMEDRIVE"));
    BOOST_TEST(!users.empty());
    users /= "\\Users";
    BOOST_TEST(fs::exists(users));
    BOOST_TEST(fs::exists(users / "All Users"));
    BOOST_TEST(fs::exists(users / "Default User"));
    BOOST_TEST(fs::is_symlink(users / "All Users"));    // dir /A reports <SYMLINKD>
    BOOST_TEST(fs::is_symlink(users / "Default User")); // dir /A reports <JUNCTION>

    fs::file_status stat(fs::symlink_status(L"\\System Volume Information"));
    BOOST_TEST(fs::type_present(stat));
    BOOST_TEST(fs::permissions_present(stat));
    BOOST_TEST(fs::status_known(stat));
    BOOST_TEST(fs::exists(stat));
    BOOST_TEST(fs::is_directory(stat));
    BOOST_TEST(!fs::is_regular_file(stat));
    BOOST_TEST(!fs::is_other(stat));
    BOOST_TEST(!fs::is_symlink(stat));

#endif
}

//  copy_symlink_tests  ---------------------------------------------------------------//

void copy_symlink_tests(const fs::path& f1x, const fs::path& d1x)
{
    cout << "copy_symlink_tests..." << endl;

    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(fs::exists(d1x));
    fs::path sym1(d1x / "symlink1");
    fs::remove(sym1); // remove possible residue from prior testing
    fs::create_symlink(f1x, sym1);
    BOOST_TEST(fs::exists(sym1));
    BOOST_TEST(fs::is_symlink(sym1));
    fs::path sym2(d1x / "symlink2");
    fs::copy_symlink(sym1, sym2);
    BOOST_TEST(fs::exists(sym2));
    BOOST_TEST(fs::is_symlink(sym2));
    //fs::path sym3(d1x / "symlink3");
    //fs::copy(sym1, sym3);
    //BOOST_TEST(fs::exists(sym3));
    //BOOST_TEST(fs::is_symlink(sym3));

    bool copy_ex_ok = false;
    try
    {
        fs::copy_symlink("no-such-file", "new-symlink1");
    }
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = true;
    }
    BOOST_TEST(copy_ex_ok);

    copy_ex_ok = false;
    try
    {
        fs::copy_symlink(f1x, "new-symlink2");
    } // should fail; f1x not symlink
    catch (const fs::filesystem_error&)
    {
        copy_ex_ok = true;
    }
    BOOST_TEST(copy_ex_ok);
}

//  creation_time_tests  -------------------------------------------------------------//

void creation_time_tests(const fs::path& dirx)
{
    cout << "creation_time_tests..." << endl;

    fs::path f1x = dirx / "creation_time_file";

    std::time_t start = std::time(NULL);

    // These pauses are inserted because the test spuriously fails on Windows, presumably because of
    // different converting FILETIME to seconds in time() and Boost.Filesystem or some sort of quirk
    // in the Windows implementation of filesystem API.
#if defined(BOOST_POSIX_API)
    sleep(1);
#else
    Sleep(1000);
#endif
    create_file(f1x, "creation_time_file");
    BOOST_TEST(fs::is_regular_file(f1x));
    try
    {
        std::time_t ft = fs::creation_time(f1x);
#if defined(BOOST_POSIX_API)
        sleep(1);
#else
        Sleep(1000);
#endif
        std::time_t finish = std::time(NULL);
        cout << "  start time: " << start << ", file creation time: " << ft << ", finish time: " << finish << endl;

        BOOST_TEST(ft >= start && ft <= finish);
    }
    catch (fs::filesystem_error& e)
    {
        if (e.code() == make_error_condition(mars_boost::system::errc::function_not_supported))
        {
            cout << "creation_time is not supported by the current system" << endl;
        }
        else
        {
            cout << "creation_time failed: " << e.what() << endl;
            BOOST_TEST(false);
        }
    }

    fs::remove(f1x);
}

//  write_time_tests  ----------------------------------------------------------------//

void write_time_tests(const fs::path& dirx)
{
    cout << "write_time_tests..." << endl;

    fs::path f1x = dirx / "foobar2";
    create_file(f1x, "foobar2");
    BOOST_TEST(fs::exists(f1x));
    BOOST_TEST(!fs::is_directory(f1x));
    BOOST_TEST(fs::is_regular_file(f1x));
    BOOST_TEST(fs::file_size(f1x) == 7);
    verify_file(f1x, "foobar2");

    // Some file system report last write time as local (FAT), while
    // others (NTFS) report it as UTC. The C standard does not specify
    // if time_t is local or UTC.

    std::time_t ft = fs::last_write_time(f1x);
    cout << "\n  UTC last_write_time() for a file just created is "
         << std::asctime(std::gmtime(&ft)) << endl;

    std::tm* tmp = std::localtime(&ft);
    cout << "\n  Year is " << tmp->tm_year << endl;
    --tmp->tm_year;
    cout << "  Change year to " << tmp->tm_year << endl;
    fs::last_write_time(f1x, std::mktime(tmp));
    std::time_t ft2 = fs::last_write_time(f1x);
    cout << "  last_write_time() for the file is now "
         << std::asctime(std::gmtime(&ft2)) << endl;
    BOOST_TEST(ft != fs::last_write_time(f1x));

    cout << "\n  Reset to current time" << endl;
    fs::last_write_time(f1x, ft);
    double time_diff = std::difftime(ft, fs::last_write_time(f1x));
    cout
        << "  original last_write_time() - current last_write_time() is "
        << time_diff << " seconds" << endl;
    BOOST_TEST(time_diff >= -60.0 && time_diff <= 60.0);
}

//  platform_specific_tests  ---------------------------------------------------------//

void platform_specific_tests()
{
    // Windows only tests
    if (platform == "Windows")
    {
        cout << "Windows specific tests..." << endl;
        if (!skip_long_windows_tests)
        {
            cout << "  (may take several seconds)" << endl;

            BOOST_TEST(!fs::exists(fs::path("//share-not")));
            BOOST_TEST(!fs::exists(fs::path("//share-not/")));
            BOOST_TEST(!fs::exists(fs::path("//share-not/foo")));
        }
        cout << endl;

        BOOST_TEST(!fs::exists("tools/jam/src/:sys:stat.h"));          // !exists() if ERROR_INVALID_NAME
        BOOST_TEST(!fs::exists(":sys:stat.h"));                        // !exists() if ERROR_INVALID_PARAMETER
        BOOST_TEST(dir.string().size() > 1 && dir.string()[1] == ':'); // verify path includes drive

        BOOST_TEST(fs::system_complete("").empty());
        BOOST_TEST(fs::system_complete("/") == fs::initial_path().root_path());
        BOOST_TEST(fs::system_complete("foo") == fs::initial_path() / "foo");

        fs::path p1(fs::system_complete("/foo"));
        BOOST_TEST_EQ(p1.string().size(), 6U); // this failed during v3 development due to bug
        std::string s1(p1.string());
        std::string s2(fs::initial_path().root_path().string() + "foo");
        BOOST_TEST_EQ(s1, s2);

        BOOST_TEST(fs::system_complete(fs::path(fs::initial_path().root_name())) == fs::initial_path());
        BOOST_TEST(fs::system_complete(fs::path(fs::initial_path().root_name().string() + "foo")).string() == fs::initial_path() / "foo");
        BOOST_TEST(fs::system_complete(fs::path("c:/")).generic_string() == "c:/");
        BOOST_TEST(fs::system_complete(fs::path("c:/foo")).generic_string() == "c:/foo");
        BOOST_TEST(fs::system_complete(fs::path("//share")).generic_string() == "//share");

#if defined(BOOST_FILESYSTEM_HAS_MKLINK)
        // Issue 9016 asked that NTFS directory junctions be recognized as directories.
        // That is equivalent to recognizing them as symlinks, and then the normal symlink
        // mechanism takes care of recognizing them as directories.
        //
        // Directory junctions are very similar to symlinks, but have some performance
        // and other advantages over symlinks. They can be created from the command line
        // with "mklink /J junction-name target-path".

        {
            cout << "  directory junction tests..." << endl;
            BOOST_TEST(fs::exists(dir));
            BOOST_TEST(fs::exists(dir / "d1/d1f1"));
            fs::path junc(dir / "junc");
            if (fs::exists(junc))
                fs::remove(junc);
            fs::path new_junc(dir / "new-junc");
            if (fs::exists(new_junc))
                fs::remove(new_junc);

            //cout << "    dir is " << dir << endl;
            //cout << "    junc is " << junc << endl;
            //cout << "    new_junc is " << new_junc << endl;
            //cout << "    current_path() is " << fs::current_path() << endl;

            fs::path cur_path(fs::current_path());
            fs::current_path(dir);
            //cout << "    current_path() is " << fs::current_path() << endl;
            BOOST_TEST(std::system("mklink /J junc d1") == 0);
            //std::system("dir");
            fs::current_path(cur_path);
            //cout << "    current_path() is " << fs::current_path() << endl;

            BOOST_TEST(fs::exists(junc));
            BOOST_TEST(fs::is_symlink(junc));
            BOOST_TEST(fs::is_directory(junc));
            BOOST_TEST(!fs::is_regular_file(junc));
            BOOST_TEST(fs::exists(junc / "d1f1"));
            BOOST_TEST(fs::is_regular_file(junc / "d1f1"));

            int count = 0;
            for (fs::directory_iterator itr(junc); itr != fs::directory_iterator(); ++itr)
            {
                //cout << itr->path() << endl;
                ++count;
            }
            cout << "    iteration count is " << count << endl;
            BOOST_TEST(count > 0);

            fs::rename(junc, new_junc);
            BOOST_TEST(!fs::exists(junc));
            BOOST_TEST(fs::exists(new_junc));
            BOOST_TEST(fs::is_symlink(new_junc));
            BOOST_TEST(fs::is_directory(new_junc));
            BOOST_TEST(!fs::is_regular_file(new_junc));
            BOOST_TEST(fs::exists(new_junc / "d1f1"));
            BOOST_TEST(fs::is_regular_file(new_junc / "d1f1"));

            fs::remove(new_junc);
            BOOST_TEST(!fs::exists(new_junc / "d1f1"));
            BOOST_TEST(!fs::exists(new_junc));
            BOOST_TEST(fs::exists(dir));
            BOOST_TEST(fs::exists(dir / "d1/d1f1"));
        }
#endif // defined(BOOST_FILESYSTEM_HAS_MKLINK)
    }  // Windows

    else if (platform == "POSIX")
    {
        cout << "POSIX specific tests..." << endl;
        BOOST_TEST(fs::system_complete("").empty());
        BOOST_TEST(fs::initial_path().root_path().string() == "/");
        BOOST_TEST(fs::system_complete("/").string() == "/");
        BOOST_TEST(fs::system_complete("foo").string() == fs::initial_path().string() + "/foo");
        BOOST_TEST(fs::system_complete("/foo").string() == fs::initial_path().root_path().string() + "foo");
    } // POSIX
}

//  initial_tests  -------------------------------------------------------------------//

void initial_tests()
{
    cout << "initial_tests..." << endl;

    cout << "  current_path().string() is\n  \""
         << fs::initial_path().string()
         << "\"\n\n";
    BOOST_TEST(fs::initial_path() == fs::current_path());
    BOOST_TEST(fs::initial_path().is_absolute());
    BOOST_TEST(fs::current_path().is_absolute());
    BOOST_TEST(fs::initial_path().string() == fs::current_path().string());
}

//  space_tests  ---------------------------------------------------------------------//

void space_tests()
{
    cout << "space_tests..." << endl;

    // make some reasonable assuptions for testing purposes
    fs::space_info spi(fs::space(dir));
    BOOST_TEST(spi.capacity > 1000000);
    BOOST_TEST(spi.free > 1000);
    BOOST_TEST(spi.capacity > spi.free);
    BOOST_TEST(spi.free >= spi.available);

    // it is convenient to display space, but older VC++ versions choke
#if !defined(BOOST_MSVC) || _MSC_VER >= 1300 // 1300 == VC++ 7.0
    cout << "   capacity = " << spi.capacity << '\n';
    cout << "       free = " << spi.free << '\n';
    cout << "  available = " << spi.available << '\n';
#endif

    // Test that we can specify path to file
    fs::path file = dir / "file";
    create_file(file);

    fs::space_info spi_file(fs::space(file));
    BOOST_TEST_EQ(spi_file.capacity, spi.capacity);

    fs::remove(file);

    // Test that an error is indicated if a path to a non-existing file is passed
    BOOST_TEST(CHECK_EXCEPTION(bad_space, ENOENT));
}

//  equivalent_tests  ----------------------------------------------------------------//

void equivalent_tests(const fs::path& f1x)
{
    cout << "equivalent_tests..." << endl;

    BOOST_TEST(CHECK_EXCEPTION(bad_equivalent, ENOENT));
    BOOST_TEST(fs::equivalent(f1x, dir / "f1"));
    BOOST_TEST(fs::equivalent(dir, d1 / ".."));
    BOOST_TEST(!fs::equivalent(f1x, dir));
    BOOST_TEST(!fs::equivalent(dir, f1x));
    BOOST_TEST(!fs::equivalent(d1, d2));
    BOOST_TEST(!fs::equivalent(dir, ng));
    BOOST_TEST(!fs::equivalent(ng, dir));
    BOOST_TEST(!fs::equivalent(f1x, ng));
    BOOST_TEST(!fs::equivalent(ng, f1x));
}

//  temp_directory_path_tests  -------------------------------------------------------//
//    contributed by Jeff Flinn

struct guarded_env_var
{
    struct previous_value
    {
        std::string m_name;
        std::string m_string;
        bool m_empty;

        previous_value(const char* name) :
            m_name(name), m_empty(true)
        {
            if (const char* value = getenv(name))
            {
                m_string.assign(value);
                m_empty = false;
            }
            else
            {
                m_empty = true;
            }
        }
        ~previous_value()
        {
            m_empty ? unsetenv_(m_name.c_str()) : setenv_(m_name.c_str(), m_string.c_str(), 1);
        }
    };

    previous_value m_previous_value;

    guarded_env_var(const char* name, const char* value) :
        m_previous_value(name)
    {
        //      std::cout << name << " old value is \"" << getenv(name) << "\"" << std::endl;
        value ? setenv_(name, value, 1) : unsetenv_(name);
        //      std::cout << name << " new value is \"" << getenv(name) << "\"" << std::endl;
    }
};

void temp_directory_path_tests()
{
    {
        cout << "temp_directory_path_tests..." << endl;
        cout << " temp_directory_path() is " << fs::temp_directory_path() << endl;

#if defined(BOOST_WINDOWS_API)

        //**************************************************************************************//
        //   Bug in GCC 4.9 getenv() when !defined(__GXX_EXPERIMENTAL_CXX0X__) makes these
        //   tests meaningless, so skip them
        //**************************************************************************************//

#if defined(__CYGWIN__) && !defined(__GXX_EXPERIMENTAL_CXX0X__) && __GNUC__ == 4
        cout << "Bug in GCC 4.9 getenv() when !defined(__GXX_EXPERIMENTAL_CXX0X__) makes these"
                "tests meaningless, so skip them"
             << endl;
        return;
#endif
        // Test ticket #5300, temp_directory_path failure on Windows with path length > 130.
        // (This test failed prior to the fix being applied.)
        {
            const wchar_t long_name[] =
                L"12345678901234567890123456789012345678901234567890"
                L"12345678901234567890123456789012345678901234567890"
                L"12345678901234567890123456789012345678901234567890#" // total 151 chars
                ;
            fs::path p(temp_dir);
            p /= long_name;
            fs::create_directory(p);

            guarded_env_var tmp_guard("TMP", p.string().c_str());
            error_code ec;
            fs::path tmp_path = fs::temp_directory_path(ec);
            BOOST_TEST(!ec);
            BOOST_TEST_EQ(p, tmp_path);
            fs::remove(p);
        }

        // Test ticket #10388, null character at end of filesystem::temp_directory_path path
        {
            guarded_env_var tmp_guard("TMP", fs::initial_path().string().c_str());

            error_code ec;
            fs::path tmp_path = fs::temp_directory_path(ec);
            BOOST_TEST_EQ(tmp_path, fs::initial_path());
        }

#endif
        BOOST_TEST(!fs::temp_directory_path().empty());
        BOOST_TEST(exists(fs::temp_directory_path()));
        fs::path ph = fs::temp_directory_path() / fs::unique_path("temp_directory_path_test_%%%%_%%%%.txt");
        {
            if (exists(ph))
                remove(ph);
            std::ofstream f(BOOST_FILESYSTEM_C_STR(ph));
            f << "passed";
        }
        BOOST_TEST(exists(ph));
        {
            std::ifstream f(BOOST_FILESYSTEM_C_STR(ph));
            std::string s;
            f >> s;
            BOOST_TEST(s == "passed");
        }
        remove(ph);
        BOOST_TEST(!exists(ph));
    }

    fs::path test_temp_dir = temp_dir;

#if defined(BOOST_POSIX_API)
    {
        struct guarded_tmp_vars
        {
            guarded_env_var m_tmpdir;
            guarded_env_var m_tmp;
            guarded_env_var m_temp;
            guarded_env_var m_tempdir;

            guarded_tmp_vars(const fs::path::value_type* tmpdir, const fs::path::value_type* tmp, const fs::path::value_type* temp, const fs::path::value_type* tempdir) :
                m_tmpdir("TMPDIR", tmpdir), m_tmp("TMP", tmp), m_temp("TEMP", temp), m_tempdir("TEMPDIR", tempdir)
            {
            }
        };

        {
            guarded_tmp_vars vars(test_temp_dir.c_str(), 0, 0, 0);
            fs::path ph = fs::temp_directory_path();
            BOOST_TEST(equivalent(test_temp_dir, ph));
        }
        {
            guarded_tmp_vars vars(0, test_temp_dir.c_str(), 0, 0);
            fs::path ph = fs::temp_directory_path();
            BOOST_TEST(equivalent(test_temp_dir, ph));
        }
        {
            guarded_tmp_vars vars(0, 0, test_temp_dir.c_str(), 0);
            fs::path ph = fs::temp_directory_path();
            BOOST_TEST(equivalent(test_temp_dir, ph));
        }
        {
            guarded_tmp_vars vars(0, 0, 0, test_temp_dir.c_str());
            fs::path ph = fs::temp_directory_path();
            BOOST_TEST(equivalent(test_temp_dir, ph));
        }
    }
#endif

#if defined(BOOST_WINDOWS_API)

    struct guarded_tmp_vars
    {
        guarded_env_var m_tmp;
        guarded_env_var m_temp;
        guarded_env_var m_localappdata;
        guarded_env_var m_userprofile;

        guarded_tmp_vars(const char* tmp, const char* temp, const char* localappdata, const char* userprofile) :
            m_tmp("TMP", tmp), m_temp("TEMP", temp), m_localappdata("LOCALAPPDATA", localappdata), m_userprofile("USERPROFILE", userprofile)
        {
        }
    };

    // test the GetWindowsDirectoryW()/Temp fallback
    {
        guarded_tmp_vars vars(0, 0, 0, 0);
        error_code ec;
        fs::path ph = fs::temp_directory_path(ec);
        BOOST_TEST(!ec);
        cout << "Fallback test, temp_directory_path() returned " << ph << endl;
    }

    {
        guarded_tmp_vars vars(test_temp_dir.string().c_str(), 0, 0, 0);
        fs::path ph = fs::temp_directory_path();
        BOOST_TEST(equivalent(test_temp_dir, ph));
    }
    {
        guarded_tmp_vars vars(0, test_temp_dir.string().c_str(), 0, 0);
        fs::path ph = fs::temp_directory_path();
        BOOST_TEST(equivalent(test_temp_dir, ph));
    }

    fs::create_directory(test_temp_dir / L"Temp");
    {
        guarded_tmp_vars vars(0, 0, test_temp_dir.string().c_str(), 0);
        fs::path ph = fs::temp_directory_path();
        BOOST_TEST(equivalent(test_temp_dir / L"Temp", ph));
        cout << "temp_directory_path() returned " << ph << endl;
    }
    {
        guarded_tmp_vars vars(0, 0, 0, test_temp_dir.string().c_str());
        fs::path ph = fs::temp_directory_path();
        BOOST_TEST(equivalent(test_temp_dir / L"Temp", ph));
        cout << "temp_directory_path() returned " << ph << endl;
    }
#endif
}

//  weakly_canonical_basic_tests  ----------------------------------------------------//

void weakly_canonical_basic_tests()
{
    cout << "weakly_canonical_basic_tests..." << endl;
    cout << "  dir is " << dir << endl;

    BOOST_TEST_EQ(fs::weakly_canonical("no-such/foo/bar"), fs::path("no-such/foo/bar"));
    BOOST_TEST_EQ(fs::weakly_canonical("no-such/foo/../bar"), fs::path("no-such/bar"));
    BOOST_TEST_EQ(fs::weakly_canonical(dir), dir);
    BOOST_TEST_EQ(fs::weakly_canonical(dir / "no-such/foo/bar"), dir / "no-such/foo/bar");
    BOOST_TEST_EQ(fs::weakly_canonical(dir / "no-such/foo/../bar"), dir / "no-such/bar");
    BOOST_TEST_EQ(fs::weakly_canonical(dir / "../no-such/foo/../bar"), dir.parent_path() / "no-such/bar");
    BOOST_TEST_EQ(fs::weakly_canonical(dir / "no-such/../f0"), dir / "f0"); // dir / "f0" exists, dir / "no-such" does not
    BOOST_TEST_EQ(fs::weakly_canonical("c:/no-such/foo/bar"), fs::path("c:/no-such/foo/bar"));

#ifdef BOOST_WINDOWS_API
    // Test Windows long paths
    fs::path long_path = make_long_path(dir / L"f0");
    BOOST_TEST_EQ(fs::weakly_canonical(long_path), long_path);

    long_path = make_long_path(dir / L"no-such/foo/bar");
    BOOST_TEST_EQ(fs::weakly_canonical(long_path), long_path);
#endif
}

//  weakly_canonical_symlink_tests  --------------------------------------------------//

void weakly_canonical_symlink_tests()
{
    cout << "weakly_canonical_symlink_tests..." << endl;
    cout << "  dir is " << dir << endl;

    fs::create_directory_symlink(dir / "d1", dir / "sld1");
    BOOST_TEST_EQ(fs::weakly_canonical(dir / "sld1/foo/bar"), dir / "d1/foo/bar");

    BOOST_TEST_EQ(relative(dir / "sld1/foo/bar/baz", dir / "d1/foo"), fs::path("bar/baz"));
}

//  _tests  --------------------------------------------------------------------------//

//void _tests()
//{
//  cout << "_tests..." << endl;
//}

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
        if (*argv[1] == '-' && *(argv[1] + 1) == 't')
            report_throws = true;
        else if (*argv[1] == '-' && *(argv[1] + 1) == 'x')
            cleanup = false;
        else if (*argv[1] == '-' && *(argv[1] + 1) == 'w')
            skip_long_windows_tests = true;
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
    do_the_right_thing_tests(); // compile-only tests, but call anyhow to suppress warnings

    for (fs::path::const_iterator it = ip.begin(); it != ip.end(); ++it)
    {
        if (it != ip.begin())
            cout << ", ";
        cout << *it;
    }
    cout << endl;

    dir = fs::initial_path() / temp_dir;

    if (fs::exists(dir))
    {
        cout << "remove residue from prior failed tests..." << endl;
        fs::remove_all(dir);
    }
    BOOST_TEST(!fs::exists(dir));

    // several functions give unreasonable results if uintmax_t isn't 64-bits
    cout << "sizeof(mars_boost::uintmax_t) = " << sizeof(mars_boost::uintmax_t) << '\n';
    BOOST_TEST(sizeof(mars_boost::uintmax_t) >= 8);

    initial_tests();
    predicate_and_status_tests();
    exception_tests();
    create_directory_tests();
    current_directory_tests();
    space_tests();

    // create a directory tree that can be used by subsequent tests
    //
    //    dir
    //      d1
    //        d1f1       // an empty file
    //      f0           // an empty file
    //      f1           // a file containing "file f1"
    //
    create_tree();

    status_of_nonexistent_tests();
    status_error_reporting_tests();
    directory_iterator_tests();
    create_directories_tests(); // must run AFTER directory_iterator_tests

    bad_create_directory_path = f1;
    BOOST_TEST(CHECK_EXCEPTION(bad_create_directory, EEXIST));
    fs::file_status stat = fs::status(f1);
    BOOST_TEST(fs::status_known(stat));
    BOOST_TEST(fs::exists(stat));
    BOOST_TEST(!fs::is_directory(stat));
    BOOST_TEST(fs::is_regular_file(stat));
    BOOST_TEST(!fs::is_other(stat));
    BOOST_TEST(!fs::is_symlink(stat));

    equivalent_tests(f1);
    create_hard_link_tests();
    create_symlink_tests();
    resize_file_tests();
    absolute_tests();
    canonical_basic_tests();
    weakly_canonical_basic_tests();
    permissions_tests();
    copy_file_tests(f1, d1);
    if (create_symlink_ok) // only if symlinks supported
    {
        symlink_status_tests();
        copy_symlink_tests(f1, d1);
        canonical_symlink_tests();
        weakly_canonical_symlink_tests();
    }
    iterator_status_tests(); // lots of cases by now, so a good time to test
                             //  dump_tree(dir);
    recursive_directory_iterator_tests();
    recursive_iterator_status_tests(); // lots of cases by now, so a good time to test
    rename_tests();
    remove_tests(dir);
    remove_all_tests(dir);
    if (create_symlink_ok) // only if symlinks supported
    {
        remove_symlink_tests();
        remove_all_symlink_tests(dir);
    }
    creation_time_tests(dir);
    write_time_tests(dir);
    temp_directory_path_tests();

    platform_specific_tests(); // do these last since they take a lot of time on Windows,
                               // and that's a pain during manual testing

    cout << "testing complete" << endl;

    // post-test cleanup
    if (cleanup)
    {
        cout << "post-test removal of " << dir << endl;
        BOOST_TEST(fs::remove_all(dir) != 0);
        // above was added just to simplify testing, but it ended up detecting
        // a bug (failure to close an internal search handle).
        cout << "post-test removal complete" << endl;
        //    BOOST_TEST(!fs::exists(dir));  // nice test, but doesn't play well with TortoiseGit cache
    }

    cout << "returning from main()" << endl;
    return ::mars_boost::report_errors();
}
