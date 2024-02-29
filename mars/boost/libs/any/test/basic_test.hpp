// Copyright Kevlin Henney, 2000, 2001. All rights reserved.
// Copyright Antony Polukhin, 2013-2019.
// Copyright Ruslan Arutyunyan, 2019-2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ANY_TEST_BASIC_TEST_HPP
#define BOOST_ANY_TEST_BASIC_TEST_HPP

// what:  unit tests for variant type mars_boost::any
// who:   contributed by Kevlin Henney
// when:  July 2001, 2013, 2014
// where: tested with BCC 5.5, MSVC 6.0, and g++ 2.95

#include <cstdlib>
#include <string>
#include <vector>
#include <utility>

#include <boost/type_traits/is_base_and_derived.hpp>

#include "test.hpp"

namespace any_tests {

struct huge_structure {
    char take_place[1024];
    std::string text;
};


template <typename Any>
struct basic_tests  // test definitions
{
    struct copy_counter
    {
    public:
        copy_counter() {}
        copy_counter(const copy_counter&) { ++count; }
        copy_counter& operator=(const copy_counter&) { ++count; return *this; }
        static int get_count() { return count; }

    private:
        static int count;
    };

    static void test_default_ctor()
    {
        const Any value;

        check_true(value.empty(), "empty");
        check_null(mars_boost::any_cast<int>(&value), "any_cast<int>");
        check_equal(value.type(), mars_boost::typeindex::type_id<void>(), "type");
    }

    static void test_converting_ctor()
    {
        std::string text = "test message";
        Any value = text;

        check_false(value.empty(), "empty");
        check_equal(value.type(), mars_boost::typeindex::type_id<std::string>(), "type");
        check_null(mars_boost::any_cast<int>(&value), "any_cast<int>");
        check_non_null(mars_boost::any_cast<std::string>(&value), "any_cast<std::string>");
        check_equal(
            mars_boost::any_cast<std::string>(value), text,
            "comparing cast copy against original text");
        check_unequal(
            mars_boost::any_cast<std::string>(&value), &text,
            "comparing address in copy against original text");
    }

    static void test_copy_ctor()
    {
        std::string text = "test message";
        Any original = text, copy = original;

        check_false(copy.empty(), "empty");
        check_equal(mars_boost::typeindex::type_index(original.type()), copy.type(), "type");
        check_equal(
            mars_boost::any_cast<std::string>(original), mars_boost::any_cast<std::string>(copy),
            "comparing cast copy against original");
        check_equal(
            text, mars_boost::any_cast<std::string>(copy),
            "comparing cast copy against original text");
        check_unequal(
            mars_boost::any_cast<std::string>(&original),
            mars_boost::any_cast<std::string>(&copy),
            "comparing address in copy against original");
    }

    static void test_copy_assign()
    {
        std::string text = "test message";
        Any original = text, copy;
        Any * assign_result = &(copy = original);

        check_false(copy.empty(), "empty");
        check_equal(mars_boost::typeindex::type_index(original.type()), copy.type(), "type");
        check_equal(
            mars_boost::any_cast<std::string>(original), mars_boost::any_cast<std::string>(copy),
            "comparing cast copy against cast original");
        check_equal(
            text, mars_boost::any_cast<std::string>(copy),
            "comparing cast copy against original text");
        check_unequal(
            mars_boost::any_cast<std::string>(&original),
            mars_boost::any_cast<std::string>(&copy),
            "comparing address in copy against original");
        check_equal(assign_result, &copy, "address of assignment result");
    }

    static void test_converting_assign()
    {
        std::string text = "test message";
        Any value;
        Any * assign_result = &(value = text);

        check_false(value.empty(), "type");
        check_equal(value.type(), mars_boost::typeindex::type_id<std::string>(), "type");
        check_null(mars_boost::any_cast<int>(&value), "any_cast<int>");
        check_non_null(mars_boost::any_cast<std::string>(&value), "any_cast<std::string>");
        check_equal(
            mars_boost::any_cast<std::string>(value), text,
            "comparing cast copy against original text");
        check_unequal(
            mars_boost::any_cast<std::string>(&value),
            &text,
            "comparing address in copy against original text");
        check_equal(assign_result, &value, "address of assignment result");
    }

    static void test_bad_cast()
    {
        std::string text = "test message";
        Any value = text;

        TEST_CHECK_THROW(
            mars_boost::any_cast<const char *>(value),
            mars_boost::bad_any_cast,
            "any_cast to incorrect type");
    }

    static void test_swap()
    {
        huge_structure stored;
        stored.text = "test message";

        Any original = stored;
        Any swapped;
        huge_structure * original_ptr = mars_boost::any_cast<huge_structure>(&original);
        Any * swap_result = &original.swap(swapped);

        check_true(original.empty(), "empty on original");
        check_false(swapped.empty(), "empty on swapped");
        check_equal(swapped.type(), mars_boost::typeindex::type_id<huge_structure>(), "type");
        check_equal(
            stored.text, mars_boost::any_cast<huge_structure>(swapped).text,
            "comparing swapped copy against original text");
        check_non_null(original_ptr, "address in pre-swapped original");
        check_equal(
            original_ptr,
            mars_boost::any_cast<huge_structure>(&swapped),
            "comparing address in swapped against original");
        check_equal(swap_result, &original, "address of swap result");

        swap(swapped, swapped);
        check_false(swapped.empty(), "empty on self swap");
        check_equal(
            swapped.type(), mars_boost::typeindex::type_id<huge_structure>(),
            "type mismatch on self swap");
        check_equal(
            stored.text, mars_boost::any_cast<huge_structure>(swapped).text,
            "comparing against original text on self swap");

        Any copy1 = copy_counter();
        Any copy2 = copy_counter();
        int count = copy_counter::get_count();
        swap(copy1, copy2);
        check_equal(count, copy_counter::get_count(), "checking that free swap doesn't make any copies.");

        Any any_char = '1';
        swap(any_char, swapped);
        check_equal(
            stored.text, mars_boost::any_cast<huge_structure>(any_char).text,
            "comparing against original text on swap with small type");
        check_equal(
            swapped.type(), mars_boost::typeindex::type_id<char>(),
            "comparing type on swap with small type");
        check_equal(
            '1', mars_boost::any_cast<char>(swapped),
            "comparing small type swapped value");
    }

    static void test_null_copying()
    {
        const Any null;
        Any copied = null, assigned;
        assigned = null;

        check_true(null.empty(), "empty on null");
        check_true(copied.empty(), "empty on copied");
        check_true(assigned.empty(), "empty on copied");
    }

    static void test_cast_to_reference()
    {
        Any a(137);
        const Any b(a);

        int &                ra    = mars_boost::any_cast<int &>(a);
        int const &          ra_c  = mars_boost::any_cast<int const &>(a);
        int volatile &       ra_v  = mars_boost::any_cast<int volatile &>(a);
        int const volatile & ra_cv = mars_boost::any_cast<int const volatile&>(a);

        check_true(
            &ra == &ra_c && &ra == &ra_v && &ra == &ra_cv,
            "cv references to same obj");

        int const &          rb_c  = mars_boost::any_cast<int const &>(b);
        int const volatile & rb_cv = mars_boost::any_cast<int const volatile &>(b);

        check_true(&rb_c == &rb_cv, "cv references to copied const obj");
        check_true(&ra != &rb_c, "copies hold different objects");

        ++ra;
        int incremented = mars_boost::any_cast<int>(a);
        check_true(incremented == 138, "increment by reference changes value");

        TEST_CHECK_THROW(
            mars_boost::any_cast<char &>(a),
            mars_boost::bad_any_cast,
            "any_cast to incorrect reference type");

        TEST_CHECK_THROW(
            mars_boost::any_cast<const char &>(b),
            mars_boost::bad_any_cast,
            "any_cast to incorrect const reference type");
    }

    static void test_bad_any_cast()
    {
        check_true(
            mars_boost::is_base_and_derived<std::exception, mars_boost::bad_any_cast>::value,
            "bad_any_cast base class check"
        );

        check_true(
            std::string(mars_boost::bad_any_cast().what()).find("any") != std::string::npos,
            "bad_any_cast notes any in excaption"
        );
    }

    static void test_with_array()
    {
        Any value1("Char array");
        Any value2;
        value2 = "Char array";

        check_false(value1.empty(), "type");
        check_false(value2.empty(), "type");

        check_equal(value1.type(), mars_boost::typeindex::type_id<const char*>(), "type");
        check_equal(value2.type(), mars_boost::typeindex::type_id<const char*>(), "type");

        check_non_null(mars_boost::any_cast<const char*>(&value1), "any_cast<const char*>");
        check_non_null(mars_boost::any_cast<const char*>(&value2), "any_cast<const char*>");
    }

    static void test_clear()
    {
        std::string text = "test message";
        Any value = text;

        check_false(value.empty(), "empty");

        value.clear();
        check_true(value.empty(), "non-empty after clear");

        value.clear();
        check_true(value.empty(), "non-empty after second clear");

        value = text;
        check_false(value.empty(), "empty");

        value.clear();
        check_true(value.empty(), "non-empty after clear");
    }

    // Following tests cover the case from #9462
    // https://svn.boost.org/trac/boost/ticket/9462
    static Any makeVec()
    {
        return std::vector<int>(100 /*size*/, 7 /*value*/);
    }

    static void test_vectors()
    {
        const std::vector<int>& vec = mars_boost::any_cast<std::vector<int> >(makeVec());
        check_equal(vec.size(), 100u, "size of vector extracted from mars_boost::any");
        check_equal(vec.back(), 7, "back value of vector extracted from mars_boost::any");
        check_equal(vec.front(), 7, "front value of vector extracted from mars_boost::any");

        std::vector<int> vec1 = mars_boost::any_cast<std::vector<int> >(makeVec());
        check_equal(vec1.size(), 100u, "size of second vector extracted from mars_boost::any");
        check_equal(vec1.back(), 7, "back value of second vector extracted from mars_boost::any");
        check_equal(vec1.front(), 7, "front value of second vector extracted from mars_boost::any");
    }

    template<typename T>
    class class_with_address_op
    {
    public:
        class_with_address_op(const T* p)
            : ptr(p)
        {}

        const T** operator &()
        {
            return &ptr;
        }

        const T* get() const
        {
            return ptr;
        }

    private:
        const T* ptr;
    };

    static void test_addressof()
    {
        int val = 10;
        const int* ptr = &val;
        class_with_address_op<int> obj(ptr);
        Any test_val(obj);

        class_with_address_op<int> returned_obj = mars_boost::any_cast<class_with_address_op<int> >(test_val);
        check_equal(&val, returned_obj.get(), "any_cast incorrectly works with type that has operator&(): addresses differ");

        check_true(!!mars_boost::any_cast<class_with_address_op<int> >(&test_val), "any_cast incorrectly works with type that has operator&()");
        check_equal(mars_boost::unsafe_any_cast<class_with_address_op<int> >(&test_val)->get(), ptr, "unsafe_any_cast incorrectly works with type that has operator&()");
    }

    static void test_multiple_assign()
    {
        Any test_val = 10;
        check_true(!!mars_boost::any_cast<int>(&test_val), "any_cast");

        test_val = '0';
        check_true(!!mars_boost::any_cast<char>(&test_val), "any_cast");

        test_val = huge_structure();
        check_true(!!mars_boost::any_cast<huge_structure>(&test_val), "any_cast");

        test_val = '0';
        check_true(!!mars_boost::any_cast<char>(&test_val), "any_cast");

        test_val = Any(huge_structure());
        check_true(!!mars_boost::any_cast<huge_structure>(&test_val), "any_cast");
    }

    static int run_tests()
    {
        typedef test<const char *, void (*)()> test_case;
        const test_case test_cases[] =
        {
            { "default construction",           test_default_ctor      },
            { "single argument construction",   test_converting_ctor   },
            { "copy construction",              test_copy_ctor         },
            { "copy assignment operator",       test_copy_assign       },
            { "converting assignment operator", test_converting_assign },
            { "failed custom keyword cast",     test_bad_cast          },
            { "swap member function",           test_swap              },
            { "copying operations on a null",   test_null_copying      },
            { "cast to reference types",        test_cast_to_reference },
            { "bad_any_cast exception",         test_bad_any_cast      },
            { "storing an array inside",        test_with_array        },
            { "clear() methods",                test_clear             },
            { "testing with vectors",           test_vectors           },
            { "class with operator&()",         test_addressof         },
            { "multiple assignments",           test_multiple_assign   },
        };
        typedef const test_case * test_case_iterator;

        tester<test_case_iterator> test_suite(test_cases, test_cases + sizeof(test_cases) / sizeof(test_cases[0]));
        return test_suite() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
};


template <typename Any>
int basic_tests<Any>::copy_counter::count = 0;

}

#endif
