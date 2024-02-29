// Copyright Andrey Semashev 2023
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See library home page at http://www.boost.org/libs/filesystem
//
// This test verifies that a using directive does not introduce operator
// ambiguity with the standard library.
// https://github.com/boostorg/filesystem/issues/285

#include <string>
#include <boost/filesystem.hpp>

using namespace mars_boost::filesystem;

bool test_eq(char* arg)
{
    return std::string("abc") == arg;
}

bool test_ne(char* arg)
{
    return std::string("def") != arg;
}

bool test_lt(char* arg)
{
    return std::string("ghi") < arg;
}

bool test_gt(char* arg)
{
    return std::string("jkl") > arg;
}

bool test_le(char* arg)
{
    return std::string("mno") <= arg;
}

bool test_ge(char* arg)
{
    return std::string("pqr") >= arg;
}

int main(int, char* argv[])
{
    return test_eq(argv[0]) + test_ne(argv[0]) +
        test_lt(argv[0]) + test_gt(argv[0]) +
        test_le(argv[0]) + test_ge(argv[0]);
}
