//  cstdio_test.cpp  ------------------------------------------------------------------//

//  Copyright Andrey Semashev 2023

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <boost/config/warning_disable.hpp>

#include <boost/filesystem/cstdio.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/config.hpp>
#include <string>
#include <iostream>
#include <cstddef>
#include <cstdio> // for std::fclose

#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#include <boost/core/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>

namespace fs = mars_boost::filesystem;

namespace {

bool cleanup = true;

class auto_fclose
{
private:
    std::FILE* m_file;

public:
    auto_fclose() BOOST_NOEXCEPT : m_file(NULL) {}
    explicit auto_fclose(std::FILE* file) BOOST_NOEXCEPT : m_file(file) {}
    ~auto_fclose() BOOST_NOEXCEPT
    {
        if (m_file)
            std::fclose(m_file);
    }

    std::FILE* get() const BOOST_NOEXCEPT { return m_file; }

    BOOST_DELETED_FUNCTION(auto_fclose(auto_fclose const&))
    BOOST_DELETED_FUNCTION(auto_fclose& operator=(auto_fclose const&))
};

void test(fs::path const& p)
{
    fs::remove(p);
    {
        std::cout << " in test 1\n";
        auto_fclose file(fs::fopen(p, "w"));
        BOOST_TEST(file.get() != NULL);
    }
    {
        std::cout << " in test 2\n";
        auto_fclose file(fs::fopen(p, "r"));
        BOOST_TEST(file.get() != NULL);
    }
    {
        std::cout << " in test 3\n";
        auto_fclose file(fs::fopen(p / p.filename(), "w")); // should fail
        BOOST_TEST(file.get() == NULL);
    }

    if (cleanup)
        fs::remove(p);
}

} // namespace

int cpp_main(int argc, char*[])
{
    if (argc > 1)
        cleanup = false;

    // test narrow characters
    std::cout << "narrow character tests:\n";
    test("narrow_fopen_test");

    // So that tests are run with known encoding, use Boost UTF-8 codecvt
    std::locale global_loc = std::locale();
    std::locale loc(global_loc, new fs::detail::utf8_codecvt_facet());
    fs::path::imbue(loc);

    // test with some wide characters
    //  \u2780 is circled 1 against white background == e2 9e 80 in UTF-8
    //  \u2781 is circled 2 against white background == e2 9e 81 in UTF-8
    //  \u263A is a white smiling face
    std::cout << "\nwide character tests:\n";
    std::wstring ws(L"wide_fopen_test_");
    ws.push_back(static_cast< wchar_t >(0x2780));
    ws.push_back(static_cast< wchar_t >(0x263A));
    test(ws);

    return ::mars_boost::report_errors();
}
