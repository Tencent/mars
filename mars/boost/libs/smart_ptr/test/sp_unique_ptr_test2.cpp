// Copyright 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>
#include <boost/config/pragma_message.hpp>
#include <memory>
#include <utility>

#if defined( BOOST_NO_CXX11_SMART_PTR )

BOOST_PRAGMA_MESSAGE("Skipping test because BOOST_NO_CXX11_SMART_PTR is defined")
int main() {}

#elif defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

BOOST_PRAGMA_MESSAGE("Skipping test because BOOST_NO_CXX11_RVALUE_REFERENCES is defined")
int main() {}

#elif defined(BOOST_MSVC) && BOOST_MSVC < 1700

BOOST_PRAGMA_MESSAGE("Skipping test because msvc-10.0 unique_ptr doesn't support move-only deleters")
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
        else
        {
            BOOST_ERROR( "YD::operator()(0) called" );
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
        std::unique_ptr<Y, YD> p( new Y );
        BOOST_TEST( Y::instances == 1 );

        mars_boost::shared_ptr<Y> p2( std::move( p ) );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.get_deleter().moved_ );

        p2.reset();
        BOOST_TEST( Y::instances == 0 );
    }

    {
        std::unique_ptr<Y, YD> p( new Y );
        BOOST_TEST( Y::instances == 1 );

        mars_boost::shared_ptr<void> p2( std::move( p ) );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.get_deleter().moved_ );

        p2.reset();
        BOOST_TEST( Y::instances == 0 );
    }

    {
        std::unique_ptr<Y, YD> p( new Y );
        BOOST_TEST( Y::instances == 1 );

        mars_boost::shared_ptr<Y> p2;
        p2 = std::move( p );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.get_deleter().moved_ );

        p2.reset();
        BOOST_TEST( Y::instances == 0 );
    }

    {
        std::unique_ptr<Y, YD> p( new Y );
        BOOST_TEST( Y::instances == 1 );

        mars_boost::shared_ptr<void> p2( new int(0) );
        p2 = std::move( p );

        BOOST_TEST( Y::instances == 1 );
        BOOST_TEST( p.get() == 0 );
        BOOST_TEST( p.get_deleter().moved_ );

        p2.reset();
        BOOST_TEST( Y::instances == 0 );
    }

    return mars_boost::report_errors();
}

#endif
