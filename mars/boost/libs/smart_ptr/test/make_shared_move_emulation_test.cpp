// make_shared_move_emulation_test.cpp - a test of make_shared
//   semi-perfect forwarding of constructor arguments when using a C++03
//   compiler with move emulation.
//   Note the "semi": it means moving temporaries (real r-values) doesn't work.
//
// Copyright 2016 Giel van Schijndel
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/core/lightweight_test.hpp>
#include <boost/make_shared.hpp>
#include <boost/move/core.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/shared_ptr.hpp>

class movearg
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(movearg)
public:
    movearg()
    {}
    movearg(BOOST_RV_REF(movearg))
    {}
    movearg& operator=(BOOST_RV_REF(movearg))
    {
        return *this;
    }
};

class ByVal
{
public:
    ByVal(movearg) {}
};

class ByRef
{
public:
    enum constructor_id
    {
        move_constructor,
        const_ref_constructor
    };

    ByRef(BOOST_RV_REF(movearg)): constructed_by_(move_constructor)
    {}
    ByRef(const movearg &arg): constructed_by_(const_ref_constructor)
    {}

    constructor_id constructed_by_;
};

int main()
{
    {
        movearg a;
        mars_boost::shared_ptr< ByVal > x = mars_boost::make_shared< ByVal >(mars_boost::move(a));
    }
    {
        movearg a;
        mars_boost::shared_ptr< ByRef > x = mars_boost::make_shared< ByRef >(mars_boost::move(a));
        BOOST_TEST( x->constructed_by_ == ByRef::move_constructor);
    }
#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
    {
        mars_boost::shared_ptr< ByVal > x = mars_boost::make_shared< ByVal >(movearg());
        mars_boost::shared_ptr< ByRef > y = mars_boost::make_shared< ByRef >(movearg());
        BOOST_TEST( y->constructed_by_ == ByRef::move_constructor);
    }
#endif // !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
    {
        const movearg ca;
        mars_boost::shared_ptr< ByRef > x = mars_boost::make_shared< ByRef >(ca);
        BOOST_TEST( x->constructed_by_ == ByRef::const_ref_constructor);
    }

    return mars_boost::report_errors();
}
