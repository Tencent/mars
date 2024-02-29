//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Antony Polukhin, 2013-2023.
//  Copyright Ruslan Arutyunyan, 2019.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#ifndef BOOST_ANY_TESTS_MOVE_TEST_HPP_INCLUDED
#define BOOST_ANY_TESTS_MOVE_TEST_HPP_INCLUDED

#include <cstdlib>
#include <string>
#include <utility>

#include "test.hpp"
#include <boost/move/move.hpp>
#include <boost/type_index.hpp>

namespace any_tests {

template <typename Any>
struct move_tests // test definitions
{
    static void test_move_construction()
    {
        Any value0 = move_copy_conting_class();
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        Any value(mars_boost::move(value0));

        check(value0.empty(), "moved away value is empty");
        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");
        check_equal(
            move_copy_conting_class::copy_count, 0u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
    }

    static void test_move_assignment()
    {
        Any value0 = move_copy_conting_class();
        Any value = move_copy_conting_class();
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        value = mars_boost::move(value0);

        check(value0.empty(), "moved away is empty");
        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");
        check_equal(
            move_copy_conting_class::copy_count, 0u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
    }

    static void test_copy_construction()
    {
        Any value0 = move_copy_conting_class();
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        Any value(value0);

        check_false(value0.empty(), "copied value is not empty");
        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");
        check_equal(
            move_copy_conting_class::copy_count, 1u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
    }

    static void test_copy_assignment()
    {
        Any value0 = move_copy_conting_class();
        Any value = move_copy_conting_class();
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        value = value0;

        check_false(value0.empty(), "copied value is not empty");
        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");
        check_equal(
            move_copy_conting_class::copy_count, 1u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
    }

    static void test_move_construction_from_value()
    {
        move_copy_conting_class value0;
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;

        Any value(mars_boost::move(value0));

        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");

        check_equal(
            move_copy_conting_class::copy_count, 0u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 1u,
            "checking move counts");
     }

    static void test_move_assignment_from_value()
    {
        move_copy_conting_class value0;
        Any value;
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        value = mars_boost::move(value0);

        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");

        check_equal(
            move_copy_conting_class::copy_count, 0u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 1u,
            "checking move counts");
    }

    static void test_copy_construction_from_value()
    {
        move_copy_conting_class value0;
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        Any value(value0);

        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");

        check_equal(
            move_copy_conting_class::copy_count, 1u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
     }

    static void test_copy_assignment_from_value()
    {
        move_copy_conting_class value0;
        Any value;
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;
        value = value0;

        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<move_copy_conting_class>(), "type");
        check_non_null(mars_boost::any_cast<move_copy_conting_class>(&value), "any_cast<move_copy_conting_class>");

        check_equal(
            move_copy_conting_class::copy_count, 1u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
    }

    static const Any helper_method() {
        return true;
    }

    static const bool helper_method1() {
        return true;
    }

    static void test_construction_from_const_any_rv()
    {
        Any values[] = {helper_method(), helper_method1() };
        (void)values;
    }

    static void test_cast_to_rv()
    {
        move_copy_conting_class value0;
        Any value;
        value = value0;
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;

        move_copy_conting_class value1 = mars_boost::any_cast<move_copy_conting_class&&>(value);

        check_equal(
            move_copy_conting_class::copy_count, 0u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 1u,
            "checking move counts");
        (void)value1;
// Following code shall fail to compile
        const Any cvalue = value0;
        move_copy_conting_class::copy_count = 0;
        move_copy_conting_class::moves_count = 0;

        move_copy_conting_class value2 = mars_boost::any_cast<const move_copy_conting_class&>(cvalue);

        check_equal(
            move_copy_conting_class::copy_count, 1u,
            "checking copy counts");
        check_equal(
            move_copy_conting_class::moves_count, 0u,
            "checking move counts");
        (void)value2;
//
    }

    class move_copy_conting_class {
    public:
        static unsigned int moves_count;
        static unsigned int copy_count;

        move_copy_conting_class(){}
        move_copy_conting_class(move_copy_conting_class&& /*param*/) {
            ++ moves_count;
        }

        move_copy_conting_class& operator=(move_copy_conting_class&& /*param*/) {
            ++ moves_count;
            return *this;
        }

        move_copy_conting_class(const move_copy_conting_class&) {
            ++ copy_count;
        }
        move_copy_conting_class& operator=(const move_copy_conting_class& /*param*/) {
            ++ copy_count;
            return *this;
        }
    };

    static int run_tests() {
        typedef test<const char *, void (*)()> test_case;
        const test_case test_cases[] =
        {
            { "move construction of any",             test_move_construction      },
            { "move assignment of any",               test_move_assignment        },
            { "copy construction of any",             test_copy_construction      },
            { "copy assignment of any",               test_copy_assignment        },

            { "move construction from value",         test_move_construction_from_value },
            { "move assignment from value",           test_move_assignment_from_value  },
            { "copy construction from value",         test_copy_construction_from_value },
            { "copy assignment from value",           test_copy_assignment_from_value },
            { "constructing from const any&&",        test_construction_from_const_any_rv },
            { "casting to rvalue reference",          test_cast_to_rv }
        };
        typedef const test_case * test_case_iterator;

        tester<test_case_iterator> test_suite(test_cases, test_cases + sizeof(test_cases) / sizeof(test_cases[0]));
        return test_suite() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}; // struct move_tests

template <typename Any>
unsigned int move_tests<Any>::move_copy_conting_class::moves_count = 0;

template <typename Any>
unsigned int move_tests<Any>::move_copy_conting_class::copy_count = 0;

} // namespace any_tests

#endif // BOOST_ANY_TESTS_MOVE_TEST_HPP_INCLUDED
