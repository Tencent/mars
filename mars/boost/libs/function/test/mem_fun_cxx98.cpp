// Function library

// Copyright (C) 2001-2003 Douglas Gregor

// Use, modification and distribution is subject to the Boost Software 
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt) 

// For more information, see http://www.boost.org/

    
#include <boost/function.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <functional>

struct Y {
    Y(int y = 0) : y_(y) {}
    bool operator==(const Y& rhs) const { return y_ == rhs.y_; }
private:
    int y_;
    };

struct X {
  int foo(int);
  Y& foo2(Y&) const;
};
int X::foo(int x) { return -x; }
Y& X::foo2(Y& x) const { return x; }

int main()
{
    mars_boost::function<int (X*, int)> f;
    mars_boost::function<Y& (X*, Y&)> f2;
	Y y1;
	
    f = &X::foo;
    f2 = &X::foo2;

    X x;
    BOOST_TEST(f(&x, 5) == -5);
    BOOST_TEST(f2(&x, mars_boost::ref(y1)) == y1);

    return ::mars_boost::report_errors();
}
