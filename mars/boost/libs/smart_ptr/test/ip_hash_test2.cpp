// Copyright 2011, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/intrusive_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>
#include <functional>

#if defined(BOOST_NO_CXX11_HDR_FUNCTIONAL)

int main() {}

#else

class base
{
private:

    int use_count_;

    base(base const &);
    base & operator=(base const &);

protected:

    base(): use_count_(0)
    {
    }

    virtual ~base()
    {
    }

public:

    long use_count() const
    {
        return use_count_;
    }

    inline friend void intrusive_ptr_add_ref(base * p)
    {
        ++p->use_count_;
    }

    inline friend void intrusive_ptr_release(base * p)
    {
        if(--p->use_count_ == 0) delete p;
    }
};

struct X: public base
{
};

int main()
{
    mars_boost::intrusive_ptr<X> p1, p2( new X );

    BOOST_TEST_EQ( std::hash< mars_boost::intrusive_ptr<X> >()( p1 ), std::hash< X* >()( p1.get() ) );
    BOOST_TEST_EQ( std::hash< mars_boost::intrusive_ptr<X> >()( p2 ), std::hash< X* >()( p2.get() ) );

    return mars_boost::report_errors();
}

#endif // #if defined(BOOST_NO_CXX11_HDR_FUNCTIONAL)
