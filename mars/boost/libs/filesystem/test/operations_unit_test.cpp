//  operations_unit_test.cpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2008, 2009, 2015

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//  ------------------------------------------------------------------------------------//

//  This program is misnamed - it is really a smoke test rather than a unit test

//  ------------------------------------------------------------------------------------//

#include <boost/config/warning_disable.hpp>

//  See deprecated_test for tests of deprecated features
#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include <boost/filesystem.hpp> // make sure filesystem.hpp works
#include <boost/filesystem/fstream.hpp> // for BOOST_FILESYSTEM_C_STR

#include <boost/system/error_code.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>

using namespace mars_boost::filesystem;
using namespace mars_boost::system;
using std::cout;
using std::endl;
using std::string;

#define CHECK(x) check(x, __FILE__, __LINE__)

namespace {
bool cleanup = true;

void create_file(const path& ph, const std::string& contents = std::string())
{
    std::ofstream f(BOOST_FILESYSTEM_C_STR(ph));
    if (!f)
        throw filesystem_error("operations_test create_file", ph, error_code(errno, system_category()));
    if (!contents.empty())
        f << contents;
}

void check(bool ok, const char* file, int line)
{
    if (ok)
        return;

    ++::mars_boost::detail::test_errors();

    cout << file << '(' << line << "): test failed\n";
}

//  file_status_test  ----------------------------------------------------------------//

void file_status_test()
{
    cout << "file_status test..." << endl;

    file_status s = status(".");
    int v = s.permissions();
    cout << "  status(\".\") permissions are "
         << std::oct << (v & 0777) << std::dec << endl;
    CHECK((v & 0400) == 0400);

    s = symlink_status(".");
    v = s.permissions();
    cout << "  symlink_status(\".\") permissions are "
         << std::oct << (v & 0777) << std::dec << endl;
    CHECK((v & 0400) == 0400);
}

//  query_test  ----------------------------------------------------------------------//

void query_test()
{
    cout << "query test..." << endl;

    error_code ec;

    CHECK(file_size("no-such-file", ec) == static_cast< mars_boost::uintmax_t >(-1));
    CHECK(ec == errc::no_such_file_or_directory);

    CHECK(status("no-such-file") == file_status(file_not_found, no_perms));

    CHECK(exists("/"));
    CHECK(is_directory("/"));
    CHECK(!exists("no-such-file"));

    exists("/", ec);
    if (ec)
    {
        cout << "exists(\"/\", ec) resulted in non-zero ec.value()" << endl;
        cout << "ec value: " << ec.value() << ", message: " << ec.message() << endl;
    }
    CHECK(!ec);

    CHECK(exists("/"));
    CHECK(is_directory("/"));
    CHECK(!is_regular_file("/"));
    CHECK(!mars_boost::filesystem::is_empty("/"));
    CHECK(!is_other("/"));
}

//  directory_iterator_test  -----------------------------------------------//

void directory_iterator_test()
{
    cout << "directory_iterator_test..." << endl;

    directory_iterator end;

    directory_iterator it(".");

    CHECK(!it->path().empty());

    if (is_regular_file(it->status()))
    {
        CHECK(is_regular_file(it->symlink_status()));
        CHECK(!is_directory(it->status()));
        CHECK(!is_symlink(it->status()));
        CHECK(!is_directory(it->symlink_status()));
        CHECK(!is_symlink(it->symlink_status()));
    }
    else
    {
        CHECK(is_directory(it->status()));
        CHECK(is_directory(it->symlink_status()));
        CHECK(!is_regular_file(it->status()));
        CHECK(!is_regular_file(it->symlink_status()));
        CHECK(!is_symlink(it->status()));
        CHECK(!is_symlink(it->symlink_status()));
    }

    for (; it != end; ++it)
    {
        //cout << "  " << it->path() << "\n";
    }

    CHECK(directory_iterator(".") != directory_iterator());
    CHECK(directory_iterator() == end);

#ifndef BOOST_NO_CXX11_RANGE_BASED_FOR
    for (directory_entry& x : directory_iterator("."))
    {
        CHECK(!x.path().empty());
        //cout << "  " << x.path() << "\n";
    }
    const directory_iterator dir_itr(".");
    for (directory_entry& x : dir_itr)
    {
        CHECK(!x.path().empty());
        //cout << "  " << x.path() << "\n";
    }
#endif

    for (directory_iterator itr("."); itr != directory_iterator(); ++itr)
    {
        CHECK(!itr->path().empty());
        //cout << "  " << itr->path() << "\n";
    }

    cout << "directory_iterator_test complete" << endl;
}

//  recursive_directory_iterator_test  -----------------------------------------------//

void recursive_directory_iterator_test()
{
    cout << "recursive_directory_iterator_test..." << endl;

    recursive_directory_iterator end;

    recursive_directory_iterator it(".");

    CHECK(!it->path().empty());

    if (is_regular_file(it->status()))
    {
        CHECK(is_regular_file(it->symlink_status()));
        CHECK(!is_directory(it->status()));
        CHECK(!is_symlink(it->status()));
        CHECK(!is_directory(it->symlink_status()));
        CHECK(!is_symlink(it->symlink_status()));
    }
    else
    {
        CHECK(is_directory(it->status()));
        CHECK(is_directory(it->symlink_status()));
        CHECK(!is_regular_file(it->status()));
        CHECK(!is_regular_file(it->symlink_status()));
        CHECK(!is_symlink(it->status()));
        CHECK(!is_symlink(it->symlink_status()));
    }

    for (; it != end; ++it)
    {
        //cout << "  " << it->path() << "\n";
    }

    CHECK(recursive_directory_iterator(".") != recursive_directory_iterator());
    CHECK(recursive_directory_iterator() == end);

#ifndef BOOST_NO_CXX11_RANGE_BASED_FOR
    for (directory_entry& x : recursive_directory_iterator("."))
    {
        CHECK(!x.path().empty());
        //cout << "  " << x.path() << "\n";
    }
    const recursive_directory_iterator dir_itr(".");
    for (directory_entry& x : dir_itr)
    {
        CHECK(!x.path().empty());
        //cout << "  " << x.path() << "\n";
    }
#endif

    for (recursive_directory_iterator itr(".");
         itr != recursive_directory_iterator(); ++itr)
    {
        CHECK(!itr->path().empty());
        //cout << "  " << itr->path() << "\n";
    }

    cout << "recursive_directory_iterator_test complete" << endl;
}

//  operations_test  -------------------------------------------------------//

void operations_test()
{
    cout << "operations test..." << endl;

    error_code ec;

    CHECK(!create_directory("/", ec));

    CHECK(!mars_boost::filesystem::remove("no-such-file-or-directory"));
    CHECK(!remove_all("no-such-file-or-directory"));

    space_info info = space("/");

    CHECK(info.available <= info.capacity);

    CHECK(equivalent("/", "/"));
    CHECK(!equivalent("/", "."));

    std::time_t ft = last_write_time(".");
    ft = -1;
    last_write_time(".", ft, ec);
}

//  directory_entry_test  ------------------------------------------------------------//

void directory_entry_test(path const& temp_dir)
{
    cout << "directory_entry test..." << endl;

    create_file(temp_dir / "foo.bar");
    create_file(temp_dir / "goo.bar");
    create_directory(temp_dir / "bar.foo");

    directory_entry de(temp_dir / "foo.bar");

    CHECK(de.path() == temp_dir / "foo.bar");
    CHECK(de.status().type() == regular_file);
    CHECK(de.symlink_status().type() == regular_file);
    CHECK(de.is_regular_file());
    CHECK(de < directory_entry(temp_dir / "goo.bar"));
    CHECK(de == directory_entry(temp_dir / "foo.bar"));
    CHECK(de != directory_entry(temp_dir / "goo.bar"));
    de.replace_filename("bar.foo");
    CHECK(de.path() == temp_dir / "bar.foo");
    CHECK(de.is_directory());
    CHECK(de.status().type() == directory_file);
    CHECK(de.symlink_status().type() == directory_file);

    mars_boost::filesystem::remove(temp_dir / "bar.foo");
    mars_boost::filesystem::remove(temp_dir / "goo.bar");
    mars_boost::filesystem::remove(temp_dir / "foo.bar");
}

//  directory_entry_overload_test  ---------------------------------------------------//

void directory_entry_overload_test()
{
    cout << "directory_entry overload test..." << endl;

    directory_iterator it(".");
    path p(*it);
}

//  error_handling_test  -------------------------------------------------------------//

void error_handling_test()
{
    cout << "error handling test..." << endl;

    bool threw(false);
    try
    {
        file_size("no-such-file");
    }
    catch (const mars_boost::filesystem::filesystem_error& ex)
    {
        threw = true;
        cout << "\nas expected, attempt to get size of non-existent file threw a filesystem_error\n"
                "what() returns "
             << ex.what() << "\n";
    }
    catch (...)
    {
        cout << "\nunexpected exception type caught" << endl;
    }

    CHECK(threw);

    error_code ec;
    CHECK(!create_directory("/", ec));
}

} // unnamed namespace

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                    main                                              //
//                                                                                      //
//--------------------------------------------------------------------------------------//

int cpp_main(int argc, char* argv[])
{
// document state of critical macros
#ifdef BOOST_POSIX_API
    cout << "BOOST_POSIX_API is defined\n";
#endif
#ifdef BOOST_WINDOWS_API
    cout << "BOOST_WINDOWS_API is defined\n";
#endif
    cout << "BOOST_FILESYSTEM_DECL" << BOOST_STRINGIZE(=BOOST_FILESYSTEM_DECL) << "\n";
    cout << "BOOST_SYMBOL_VISIBLE" << BOOST_STRINGIZE(=BOOST_SYMBOL_VISIBLE) << "\n";

    cout << "current_path() is " << current_path().string() << endl;

    if (argc >= 2)
    {
        cout << "argv[1] is '" << argv[1] << "', changing current_path() to it" << endl;

        error_code ec;
        current_path(argv[1], ec);

        if (ec)
        {
            cout << "current_path('" << argv[1] << "') failed: " << ec << ": " << ec.message() << endl;
        }

        cout << "current_path() is " << current_path().string() << endl;
    }

    const path temp_dir(current_path() / ".." / unique_path("op-unit_test-%%%%-%%%%-%%%%"));
    cout << "temp_dir is " << temp_dir.string() << endl;

    create_directory(temp_dir);

    file_status_test();
    query_test();
    directory_iterator_test();
    recursive_directory_iterator_test();
    operations_test();
    directory_entry_test(temp_dir);
    directory_entry_overload_test();
    error_handling_test();

    cout << unique_path() << endl;
    cout << unique_path("foo-%%%%%-%%%%%-bar") << endl;
    cout << unique_path("foo-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%-bar") << endl;
    cout << unique_path("foo-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-bar") << endl;

    cout << "testing complete" << endl;

    // post-test cleanup
    if (cleanup)
    {
        cout << "post-test removal of " << temp_dir << endl;
        BOOST_TEST(remove_all(temp_dir) != 0);
        // above was added just to simplify testing, but it ended up detecting
        // a bug (failure to close an internal search handle).
        cout << "post-test removal complete" << endl;
        //    BOOST_TEST(!fs::exists(dir));  // nice test, but doesn't play well with TortoiseGit cache
    }

    return ::mars_boost::report_errors();
}
