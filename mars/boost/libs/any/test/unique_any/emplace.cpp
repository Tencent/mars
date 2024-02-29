// Copyright Antony Polukhin, 2013-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/any/unique_any.hpp>

#include <boost/core/lightweight_test.hpp>

#include <memory>

namespace {

struct parameter_t{};

class non_movable_t {
public:
    non_movable_t() = delete;
    non_movable_t(non_movable_t&&) = delete;
    non_movable_t(const non_movable_t&) = delete;
    non_movable_t& operator=(non_movable_t&&) = delete;
    non_movable_t& operator=(const non_movable_t&) = delete;

    non_movable_t(parameter_t, int i, std::unique_ptr<int>) : value(i) {}

    non_movable_t(std::initializer_list<parameter_t> list, std::unique_ptr<int>)
      : value(list.size())
    {}

    non_movable_t(std::initializer_list<parameter_t> list)
      : value(-int(list.size()))
    {}

    const int value;
};

}

void test_emplace_const() {
    mars_boost::anys::unique_any a;
    BOOST_TEST(!a.has_value());

    auto& str = a.emplace<const std::string>("Test");
    BOOST_TEST_EQ(str, "Test");
    BOOST_TEST(a.has_value());

    BOOST_TEST_EQ(mars_boost::any_cast<const std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<const std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::any_cast<const std::string&>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<const std::string&>(a), "Test");
    BOOST_TEST_EQ(&mars_boost::any_cast<const std::string&>(a), &str);
    BOOST_TEST_EQ(&mars_boost::anys::any_cast<const std::string&>(a), &str);

    BOOST_TEST_EQ(mars_boost::any_cast<std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::any_cast<std::string&>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<std::string&>(a), "Test");
    BOOST_TEST_EQ(&mars_boost::any_cast<std::string&>(a), &str);
    BOOST_TEST_EQ(&mars_boost::anys::any_cast<std::string&>(a), &str);

    mars_boost::any_cast<std::string&>(a) = "Hello world";
    BOOST_TEST_EQ(mars_boost::any_cast<std::string>(a), "Hello world");
    BOOST_TEST_EQ(mars_boost::any_cast<std::string&>(a), "Hello world");
    BOOST_TEST_EQ(mars_boost::any_cast<const std::string&>(a), "Hello world");
}

void test_emplace_non_const() {
    mars_boost::anys::unique_any a;
    BOOST_TEST(!a.has_value());

    auto& str = a.emplace<std::string>("Test");
    BOOST_TEST_EQ(str, "Test");
    BOOST_TEST(a.has_value());

    BOOST_TEST_EQ(mars_boost::any_cast<const std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<const std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::any_cast<const std::string&>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<const std::string&>(a), "Test");
    BOOST_TEST_EQ(&mars_boost::any_cast<const std::string&>(a), &str);
    BOOST_TEST_EQ(&mars_boost::anys::any_cast<const std::string&>(a), &str);

    BOOST_TEST_EQ(mars_boost::any_cast<std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<std::string>(a), "Test");
    BOOST_TEST_EQ(mars_boost::any_cast<std::string&>(a), "Test");
    BOOST_TEST_EQ(mars_boost::anys::any_cast<std::string&>(a), "Test");
    BOOST_TEST_EQ(&mars_boost::any_cast<std::string&>(a), &str);
    BOOST_TEST_EQ(&mars_boost::anys::any_cast<std::string&>(a), &str);

    mars_boost::any_cast<std::string&>(a) = "Hello world";
    BOOST_TEST_EQ(mars_boost::any_cast<std::string>(a), "Hello world");
    BOOST_TEST_EQ(mars_boost::any_cast<std::string&>(a), "Hello world");
    BOOST_TEST_EQ(mars_boost::any_cast<const std::string&>(a), "Hello world");
}

void test_emplace_constructor() {
    mars_boost::anys::unique_any a(
        mars_boost::anys::in_place_type_t<non_movable_t>(),
        parameter_t(),
        42,
        std::unique_ptr<int>()
    );
    BOOST_TEST(a.has_value());
    BOOST_TEST_EQ(mars_boost::any_cast<non_movable_t&>(a).value, 42);


    mars_boost::anys::in_place_type_t<int> int_tag;
    mars_boost::anys::unique_any b(int_tag);
    BOOST_TEST(b.has_value());
    BOOST_TEST_EQ(mars_boost::any_cast<int&>(b), 0);
}

void test_emplace_member() {
    mars_boost::anys::unique_any a;
    auto& ref = a.emplace<non_movable_t>(
        parameter_t(),
        42,
        std::unique_ptr<int>()
    );
    BOOST_TEST(a.has_value());
    BOOST_TEST_EQ(ref.value, 42);
    BOOST_TEST_EQ(mars_boost::any_cast<non_movable_t&>(a).value, 42);


    a.emplace<int>();
    BOOST_TEST(a.has_value());
    BOOST_TEST_EQ(mars_boost::any_cast<int&>(a), 0);
}

void test_emplace_constructor_init_list() {
    {
        mars_boost::anys::unique_any a(
            mars_boost::anys::in_place_type_t<non_movable_t>(),
            {parameter_t(), parameter_t(), parameter_t(), parameter_t()},
            std::unique_ptr<int>()
        );
        BOOST_TEST(a.has_value());
        BOOST_TEST_EQ(mars_boost::any_cast<non_movable_t&>(a).value, 4);
    }
    {
        mars_boost::anys::unique_any a(
            mars_boost::anys::in_place_type_t<non_movable_t>(),
            {parameter_t(), parameter_t(), parameter_t(), parameter_t()}
        );
        BOOST_TEST(a.has_value());
        BOOST_TEST_EQ(mars_boost::any_cast<non_movable_t&>(a).value, -4);
    }
}

void test_emplace_member_init_list() {
    {
        mars_boost::anys::unique_any a;
        auto& ref = a.emplace<non_movable_t>(
            {parameter_t(), parameter_t(), parameter_t(), parameter_t()},
            std::unique_ptr<int>()
        );
        BOOST_TEST(a.has_value());
        BOOST_TEST_EQ(ref.value, 4);
        BOOST_TEST_EQ(mars_boost::any_cast<non_movable_t&>(a).value, 4);
    }
    {
        mars_boost::anys::unique_any a;
        auto& ref = a.emplace<non_movable_t>(
            {parameter_t(), parameter_t(), parameter_t(), parameter_t()}
        );
        BOOST_TEST(a.has_value());
        BOOST_TEST_EQ(ref.value, -4);
        BOOST_TEST_EQ(mars_boost::any_cast<non_movable_t&>(a).value, -4);
    }
}

int main() {
    test_emplace_const();
    test_emplace_non_const();
    test_emplace_constructor();
    test_emplace_member();
    test_emplace_constructor_init_list();
    test_emplace_member_init_list();

    return mars_boost::report_errors();
}

