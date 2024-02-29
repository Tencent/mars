// Copyright 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>
#include <boost/config/pragma_message.hpp>
#include <memory>
#include <utility>

#if defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

BOOST_PRAGMA_MESSAGE("Skipping test because BOOST_NO_CXX11_RVALUE_REFERENCES is defined")
int main() {}

#else

struct Y
{
    static int instances;

    bool deleted_;

    Y(): deleted_( false )
    {
        ++instances;
    }

    ~Y()
    {
        BOOST_TEST( deleted_ );
        --instances;
    }

private:

    Y( Y const & );
    Y & operator=( Y const & );
};

int Y::instances = 0;

struct YD
{
    bool moved_;

    YD(): moved_( false )
    {
    }

    YD( YD&& r ): moved_( false )
    {
        r.moved_ = true;
    }

    void operator()( Y* p ) const
    {
        BOOST_TEST( !moved_ );

        if( p )
        {
            p->deleted_ = true;
            delete p;
        }
    }

private:

    YD( YD const & );
    YD & operator=( YD const & );
};

int main()
{
    BOOST_TEST( Y::instances == 0 );

    {
        YD del;
        mars_boost::shared_ptr<Y> p( new Y, std::move( del ) );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( del.moved_ );

        p.reset( new Y, YD() );

        BOOST_TEST( Y::instances == 1 );

        p = mars_boost::shared_ptr<Y>( new Y, YD() );

        BOOST_TEST( Y::instances == 1 );

        YD del2;
        p.reset( new Y, std::move( del2 ) );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( del2.moved_ );

        p.reset();
        BOOST_TEST( Y::instances == 0 );
    }

    {
        YD del;
        mars_boost::shared_ptr<Y> p( new Y, std::move( del ), std::allocator<Y>() );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( del.moved_ );

        p.reset( new Y, YD(), std::allocator<Y>() );

        BOOST_TEST( Y::instances == 1 );

        p = mars_boost::shared_ptr<Y>( new Y, YD(), std::allocator<Y>() );

        BOOST_TEST( Y::instances == 1 );

        YD del2;
        p.reset( new Y, std::move( del2 ), std::allocator<Y>() );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( del2.moved_ );

        p.reset();
        BOOST_TEST( Y::instances == 0 );
    }

#if !defined( BOOST_NO_CXX11_NULLPTR )

    {
        mars_boost::shared_ptr<Y> p( nullptr, YD() );

        YD del;
        p = mars_boost::shared_ptr<Y>( nullptr, std::move( del ) );

        BOOST_TEST( del.moved_ );
    }

    {
        mars_boost::shared_ptr<Y> p( nullptr, YD(), std::allocator<Y>() );

        YD del;
        p = mars_boost::shared_ptr<Y>( nullptr, std::move( del ), std::allocator<Y>() );

        BOOST_TEST( del.moved_ );
    }

#endif

    return mars_boost::report_errors();
}

#endif
