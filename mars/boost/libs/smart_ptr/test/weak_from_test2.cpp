
//  weak_from_test2.cpp
//
//  Tests weak_from in a destructor
//
//  Copyright 2014, 2015, 2019 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

class X: public mars_boost::enable_shared_from
{
private:

    mars_boost::weak_ptr<X> px_;

public:

    X()
    {
        mars_boost::weak_ptr<X> p1 = weak_from( this );
        BOOST_TEST( p1._empty() );
        BOOST_TEST( p1.expired() );
    }

    void check()
    {
        mars_boost::weak_ptr<X> p2 = weak_from( this );
        BOOST_TEST( !p2.expired() );

        BOOST_TEST( p2.lock().get() == this );

        px_ = p2;
    }

    ~X()
    {
        mars_boost::weak_ptr<X> p3 = weak_from( this );
        BOOST_TEST( p3.expired() );

        BOOST_TEST( !(px_ < p3) && !(p3 < px_) );
    }
};

int main()
{
    {
        mars_boost::shared_ptr< X > px( new X );
        px->check();
    }

    return mars_boost::report_errors();
}
