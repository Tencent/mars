//  Copyright Andrey Semashev 2020.

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

// This test verifies copy operation behavior.

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp> // for BOOST_FILESYSTEM_C_STR
#include <boost/system/error_code.hpp>

#include <set>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/throw_exception.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/core/lightweight_test.hpp>

namespace fs = mars_boost::filesystem;

namespace {

void create_file(fs::path const& ph, std::string const& contents = std::string())
{
    std::ofstream f(BOOST_FILESYSTEM_C_STR(ph), std::ios_base::out | std::ios_base::trunc);
    if (!f)
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create file: " + ph.string()));
    if (!contents.empty())
        f << contents;
}

void verify_file(fs::path const& ph, std::string const& expected)
{
    std::ifstream f(BOOST_FILESYSTEM_C_STR(ph));
    if (!f)
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open file: " + ph.string()));
    std::string contents;
    f >> contents;
    BOOST_TEST_EQ(contents, expected);
    if (contents != expected)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("verify_file failed: contents \"" + contents + "\" != \"" + expected + "\" in " + ph.string()));
    }
}

fs::path create_tree()
{
    fs::path root_dir = fs::unique_path();

    fs::create_directory(root_dir);
    create_file(root_dir / "f1", "f1");
    create_file(root_dir / "f2", "f2");

    fs::create_directory(root_dir / "d1");
    create_file(root_dir / "d1/f1", "d1f1");

    fs::create_directory(root_dir / "d1/d1");
    create_file(root_dir / "d1/d1/f1", "d1d1f1");

    fs::create_directory(root_dir / "d1/d2");

    fs::create_directory(root_dir / "d2");
    create_file(root_dir / "d2/f1", "d2f1");

    return root_dir;
}

typedef std::set< fs::path > directory_tree;

directory_tree collect_directory_tree(fs::path const& root_dir)
{
    std::cout << "Collecting directory tree in: " << root_dir << '\n';

    directory_tree tree;
    fs::recursive_directory_iterator it(root_dir, fs::directory_options::skip_permission_denied |
        fs::directory_options::follow_directory_symlink | fs::directory_options::skip_dangling_symlinks);
    fs::recursive_directory_iterator end;
    while (it != end)
    {
        fs::path p = fs::relative(it->path(), root_dir);
        std::cout << p << '\n';
        tree.insert(p);
        ++it;
    }

    std::cout << "done." << std::endl;

    return tree;
}

void test_copy_file_default(fs::path const& root_dir)
{
    std::cout << "test_copy_file_default" << std::endl;

    fs::path target_dir = fs::unique_path();
    fs::create_directory(target_dir);

    fs::copy(root_dir / "f1", target_dir);
    fs::copy(root_dir / "f2", target_dir / "f3");

    directory_tree tree = collect_directory_tree(target_dir);

    BOOST_TEST_EQ(tree.size(), 2u);
    BOOST_TEST(tree.find("f1") != tree.end());
    BOOST_TEST(tree.find("f3") != tree.end());

    verify_file(target_dir / "f1", "f1");
    verify_file(target_dir / "f3", "f2");

    fs::remove_all(target_dir);
}

void test_copy_dir_default(fs::path const& root_dir, bool with_symlinks)
{
    std::cout << "test_copy_dir_default" << std::endl;

    fs::path target_dir = fs::unique_path();

    fs::copy(root_dir, target_dir);

    directory_tree tree = collect_directory_tree(target_dir);

    BOOST_TEST_EQ(tree.size(), 4u + with_symlinks);
    BOOST_TEST(tree.find("f1") != tree.end());
    BOOST_TEST(tree.find("f2") != tree.end());
    BOOST_TEST(tree.find("d1") != tree.end());
    BOOST_TEST(tree.find("d2") != tree.end());
    if (with_symlinks)
    {
        BOOST_TEST(tree.find("s1") != tree.end());
    }

    verify_file(target_dir / "f1", "f1");
    verify_file(target_dir / "f2", "f2");

    fs::remove_all(target_dir);
}

void test_copy_dir_default_ec(fs::path const& root_dir, bool with_symlinks)
{
    // This test is similar to test_copy_dir_default, but uses an error_code overload of the operation.
    // Tests for https://github.com/boostorg/filesystem/issues/152 fix.

    std::cout << "test_copy_dir_default_ec" << std::endl;

    fs::path target_dir = fs::unique_path();

    mars_boost::system::error_code ec;
    fs::copy(root_dir, target_dir, ec);
    BOOST_TEST(!ec);

    directory_tree tree = collect_directory_tree(target_dir);

    BOOST_TEST_EQ(tree.size(), 4u + with_symlinks);
    BOOST_TEST(tree.find("f1") != tree.end());
    BOOST_TEST(tree.find("f2") != tree.end());
    BOOST_TEST(tree.find("d1") != tree.end());
    BOOST_TEST(tree.find("d2") != tree.end());
    if (with_symlinks)
    {
        BOOST_TEST(tree.find("s1") != tree.end());
    }

    verify_file(target_dir / "f1", "f1");
    verify_file(target_dir / "f2", "f2");

    fs::remove_all(target_dir);
}

void test_copy_dir_recursive(fs::path const& root_dir)
{
    std::cout << "test_copy_dir_recursive" << std::endl;

    fs::path target_dir = fs::unique_path();

    fs::copy(root_dir, target_dir, fs::copy_options::recursive);

    directory_tree tree = collect_directory_tree(target_dir);

    BOOST_TEST_EQ(tree.size(), 9u);
    BOOST_TEST(tree.find("f1") != tree.end());
    BOOST_TEST(tree.find("f2") != tree.end());
    BOOST_TEST(tree.find("d1") != tree.end());
    BOOST_TEST(tree.find(fs::path("d1") / "f1") != tree.end());
    BOOST_TEST(tree.find(fs::path("d1") / "d1") != tree.end());
    BOOST_TEST(tree.find(fs::path("d1") / "d1" / "f1") != tree.end());
    BOOST_TEST(tree.find(fs::path("d1") / "d2") != tree.end());
    BOOST_TEST(tree.find("d2") != tree.end());
    BOOST_TEST(tree.find(fs::path("d2") / "f1") != tree.end());

    verify_file(target_dir / "f1", "f1");
    verify_file(target_dir / "f2", "f2");
    verify_file(target_dir / "d1/f1", "d1f1");
    verify_file(target_dir / "d1/d1/f1", "d1d1f1");
    verify_file(target_dir / "d2/f1", "d2f1");

    fs::remove_all(target_dir);
}

void test_copy_dir_recursive_tree(fs::path const& root_dir)
{
    std::cout << "test_copy_dir_recursive_tree" << std::endl;

    fs::path target_dir = fs::unique_path();

    fs::copy(root_dir, target_dir, fs::copy_options::recursive | fs::copy_options::directories_only);

    directory_tree tree = collect_directory_tree(target_dir);

    BOOST_TEST_EQ(tree.size(), 4u);
    BOOST_TEST(tree.find("d1") != tree.end());
    BOOST_TEST(tree.find(fs::path("d1") / "d1") != tree.end());
    BOOST_TEST(tree.find(fs::path("d1") / "d2") != tree.end());
    BOOST_TEST(tree.find("d2") != tree.end());

    fs::remove_all(target_dir);
}

void test_copy_file_symlinks(fs::path const& root_dir)
{
    std::cout << "test_copy_file_symlinks" << std::endl;

    fs::path target_dir = fs::unique_path();
    fs::create_directory(target_dir);

    fs::copy(root_dir / "f1", target_dir);

    fs::path prev_cur_dir = fs::current_path();
    fs::current_path(target_dir);
    fs::copy(".." / root_dir / "f2", "f2", fs::copy_options::create_symlinks);
    fs::current_path(prev_cur_dir);

    // Copying from a relative path with copy_options::create_symlinks to a directory other than current directory is a non-standard extension
    fs::copy(target_dir / "f1", target_dir / "f3", fs::copy_options::create_symlinks);

    verify_file(target_dir / "f1", "f1");

    fs::path link_target = fs::read_symlink(target_dir / "f2");
    if (link_target != (".." / root_dir / "f2"))
    {
        BOOST_ERROR("Incorrect f2 symlink in test_copy_file_symlinks");
        std::cout << (target_dir / "f2") << " => " << link_target << std::endl;
    }

    link_target = fs::read_symlink(target_dir / "f3");
    if (link_target != "f1" && link_target != (fs::path(".") / "f1"))
    {
        BOOST_ERROR("Incorrect f3 symlink in test_copy_file_symlinks");
        std::cout << (target_dir / "f3") << " => " << link_target << std::endl;
    }

    fs::remove_all(target_dir);
}

void test_copy_errors(fs::path const& root_dir, bool symlinks_supported)
{
    std::cout << "test_copy_errors" << std::endl;

    fs::path target_dir = fs::unique_path();
    fs::create_directory(target_dir);

    BOOST_TEST_THROWS(fs::copy(root_dir / "non-existing", target_dir), fs::filesystem_error);

    create_file(target_dir / "f1");

    BOOST_TEST_THROWS(fs::copy(root_dir / "f1", target_dir), fs::filesystem_error);
    BOOST_TEST_THROWS(fs::copy(root_dir / "f1", target_dir / "f1"), fs::filesystem_error);
    BOOST_TEST_THROWS(fs::copy(root_dir / "d1", target_dir / "f1"), fs::filesystem_error);

    BOOST_TEST_THROWS(fs::copy(target_dir, target_dir), fs::filesystem_error);
    BOOST_TEST_THROWS(fs::copy(target_dir / "f1", target_dir / "f1"), fs::filesystem_error);

    if (symlinks_supported)
    {
        // Should fail with is_a_directory error code
        BOOST_TEST_THROWS(fs::copy(root_dir, target_dir, fs::copy_options::create_symlinks), fs::filesystem_error);
    }

    fs::remove_all(target_dir);
}

} // namespace

int main()
{
    try
    {
        fs::path root_dir = create_tree();

        test_copy_file_default(root_dir);
        test_copy_dir_default(root_dir, false);
        test_copy_dir_default_ec(root_dir, false);
        test_copy_dir_recursive(root_dir);
        test_copy_dir_recursive_tree(root_dir);

        bool symlinks_supported = false;
        try
        {
            fs::create_symlink("f1", root_dir / "s1");
            symlinks_supported = true;
            std::cout << "     *** For information only ***\n"
                         "     create_symlink() attempt succeeded"
                      << std::endl;
        }
        catch (fs::filesystem_error& e)
        {
            std::cout << "     *** For information only ***\n"
                         "     create_symlink() attempt failed\n"
                         "     filesystem_error.what() reports: "
                      << e.what() << "\n"
                                     "     create_symlink() may not be supported on this operating system or file system"
                      << std::endl;
        }

        if (symlinks_supported)
        {
            test_copy_dir_default(root_dir, true);
            test_copy_file_symlinks(root_dir);
        }

        test_copy_errors(root_dir, symlinks_supported);

        fs::remove_all(root_dir);

        return mars_boost::report_errors();
    }
    catch (std::exception& e)
    {
        std::cout << "FAIL, exception caught: " << mars_boost::diagnostic_information(e) << std::endl;
        return 1;
    }
}
