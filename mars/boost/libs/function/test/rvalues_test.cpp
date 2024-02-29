// Copyright 2014 Antony Polukhin.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/function.hpp>
#include <boost/move/move.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <cstdlib>

#define BOOST_CHECK BOOST_TEST

class only_movable {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(only_movable)
    int value_;
    bool moved_;

public:
    only_movable(BOOST_RV_REF(only_movable) x)
        : value_(x.value_)
        , moved_(false)
    {
        x.moved_ = true;
    }

   only_movable& operator=(BOOST_RV_REF(only_movable) x) {
        value_ = x.value_;
        x.moved_ = true;
        moved_ = false;
        return *this;
   }

    explicit only_movable(int value = 0) : value_(value), moved_(false) {}
    int get_value() const {  return value_;  }
    bool is_moved() const { return moved_; }
};


int one(BOOST_RV_REF(only_movable) v) { return v.get_value(); }
only_movable two(BOOST_RV_REF(only_movable) t) {
    only_movable t1 = mars_boost::move(t);
    return BOOST_MOVE_RET(only_movable, t1);
}

only_movable two_sum(BOOST_RV_REF(only_movable) t1, BOOST_RV_REF(only_movable) t2) {
    only_movable ret(t1.get_value() + t2.get_value());
    return BOOST_MOVE_RET(only_movable, ret);
}

struct sum_struct {
    only_movable operator()(BOOST_RV_REF(only_movable) t1, BOOST_RV_REF(only_movable) t2) const {
        only_movable ret(t1.get_value() + t2.get_value());
        return BOOST_MOVE_RET(only_movable, ret);
    }
};

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
int three(std::string&&) { return 1; }
std::string&& four(std::string&& s) { return mars_boost::move(s); }
#endif

int main()
{
    using mars_boost::function;

    function <int(BOOST_RV_REF(only_movable))>  f1 = one;

    only_movable om1(1);
    BOOST_CHECK(f1(mars_boost::move(om1)) == 1);

    function <only_movable(BOOST_RV_REF(only_movable))>  f2 = two;

    only_movable om2(2);
    only_movable om2_2 = f2(mars_boost::move(om2));
    BOOST_CHECK(om2_2.get_value() == 2);
    BOOST_CHECK(om2.is_moved());

    {
        function <only_movable(BOOST_RV_REF(only_movable), BOOST_RV_REF(only_movable))>  f2_sum = two_sum;
        only_movable om1_sum(1), om2_sum(2);
        only_movable om2_sum_2 = f2_sum(mars_boost::move(om1_sum), mars_boost::move(om2_sum));
        BOOST_CHECK(om2_sum_2.get_value() == 3);
    }

    {
        sum_struct s;
        function <only_movable(BOOST_RV_REF(only_movable), BOOST_RV_REF(only_movable))>  f2_sum = s;
        only_movable om1_sum(1), om2_sum(2);
        only_movable om2_sum_2 = f2_sum(mars_boost::move(om1_sum), mars_boost::move(om2_sum));
        BOOST_CHECK(om2_sum_2.get_value() == 3);
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    function <int(std::string&&)>               f3 = three;
    function <std::string&& (std::string&& s)>  f4 = four;

    f3(std::string("Hello"));
    BOOST_CHECK(f4(std::string("world")) == "world");
#endif

    return mars_boost::report_errors();
}
