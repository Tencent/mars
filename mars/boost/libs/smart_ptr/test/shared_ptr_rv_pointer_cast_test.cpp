//
//  shared_ptr_rv_pointer_cast_test.cpp
//
//  Copyright (c) 2016 Chris Glover
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

struct X
{};

struct Y: public X
{};

struct U
{
    virtual ~U() {}
};

struct V: public U
{};

struct W : public U
{};

int main()
{
    {
        mars_boost::shared_ptr<X> px(new Y);

        mars_boost::shared_ptr<Y> py1 = mars_boost::static_pointer_cast<Y>(px);
        mars_boost::shared_ptr<Y> py2 = mars_boost::static_pointer_cast<Y>(std::move(px));
        BOOST_TEST(!px);
        BOOST_TEST(px.use_count() == 0);
        BOOST_TEST(py1.get() == py2.get());
        BOOST_TEST(!(py1 < py2 || py2 < py1));
        BOOST_TEST(py1.use_count() == 2);
        BOOST_TEST(py2.use_count() == 2);
    }

    {
        mars_boost::shared_ptr<int const volatile> px(new int);

        mars_boost::shared_ptr<int> px2 = mars_boost::const_pointer_cast<int>(px);
        mars_boost::shared_ptr<int> px3 = mars_boost::const_pointer_cast<int>(std::move(px));
        BOOST_TEST(!px);
        BOOST_TEST(px.use_count() == 0);
        BOOST_TEST(px2.get() == px3.get());
        BOOST_TEST(!(px2 < px3 || px2 < px3));
        BOOST_TEST(px2.use_count() == 2);
        BOOST_TEST(px3.use_count() == 2);
    }

    {
        mars_boost::shared_ptr<char> pv(reinterpret_cast<char*>(new Y));

        mars_boost::shared_ptr<Y> py1 = mars_boost::reinterpret_pointer_cast<Y>(pv);
        mars_boost::shared_ptr<Y> py2 = mars_boost::reinterpret_pointer_cast<Y>(std::move(pv));
        BOOST_TEST(!pv);
        BOOST_TEST(pv.use_count() == 0);
        BOOST_TEST(py1.get() == py2.get());
        BOOST_TEST(!(py1 < py2 || py2 < py1));
        BOOST_TEST(py1.use_count() == 2);
        BOOST_TEST(py2.use_count() == 2);
    }

#if !defined( BOOST_NO_RTTI )
    {
        mars_boost::shared_ptr<U> pu(new V);

        mars_boost::shared_ptr<V> pv1 = mars_boost::dynamic_pointer_cast<V>(pu);
        mars_boost::shared_ptr<V> pv2 = mars_boost::dynamic_pointer_cast<V>(std::move(pu));
        BOOST_TEST(!pu);
        BOOST_TEST(pu.use_count() == 0);
        BOOST_TEST(pv1.get() == pv2.get());
        BOOST_TEST(!(pv1 < pv2 || pv2 < pv1));
        BOOST_TEST(pv1.use_count() == 2);
        BOOST_TEST(pv2.use_count() == 2);
    }

    {
        mars_boost::shared_ptr<U> pu(new V);
        mars_boost::shared_ptr<W> pw = mars_boost::dynamic_pointer_cast<W>(std::move(pu));
        BOOST_TEST(!pw);
        BOOST_TEST(pu);
    }
#endif // !defined( BOOST_NO_RTTI )

    return mars_boost::report_errors();
}

#else // !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

int main()
{
    return 0;
}

#endif // !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
