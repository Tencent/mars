#include <boost/config.hpp>

#if defined(BOOST_MSVC)

#pragma warning(disable: 4786)  // identifier truncated in debug info
#pragma warning(disable: 4710)  // function not inlined
#pragma warning(disable: 4711)  // function selected for automatic inline expansion
#pragma warning(disable: 4514)  // unreferenced inline removed
#pragma warning(disable: 4355)  // 'this' : used in base member initializer list
#pragma warning(disable: 4511)  // copy constructor could not be generated
#pragma warning(disable: 4512)  // assignment operator could not be generated

#if (BOOST_MSVC >= 1310)
#pragma warning(disable: 4675)  // resolved overload found with Koenig lookup
#endif

#endif

//
//  intrusive_ptr_move_test.cpp
//
//  Copyright (c) 2002-2005 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/core/lightweight_test.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/config.hpp>
#include <utility>

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

namespace N
{

class base
{
private:

    mutable mars_boost::detail::atomic_count use_count_;

    base(base const &);
    base & operator=(base const &);

protected:

    base(): use_count_(0)
    {
        ++instances;
    }

    virtual ~base()
    {
        --instances;
    }

public:

    static long instances;

    long use_count() const
    {
        return use_count_;
    }

#if !defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)

    inline friend void intrusive_ptr_add_ref(base const * p)
    {
        ++p->use_count_;
    }

    inline friend void intrusive_ptr_release(base const * p)
    {
        if(--p->use_count_ == 0) delete p;
    }

#else

    void add_ref() const
    {
        ++use_count_;
    }

    void release() const
    {
        if(--use_count_ == 0) delete this;
    }

#endif
};

long base::instances = 0;

} // namespace N

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{

inline void intrusive_ptr_add_ref(N::base const * p)
{
    p->add_ref();
}

inline void intrusive_ptr_release(N::base const * p)
{
    p->release();
}

} // namespace mars_boost

#endif

//

struct X: public virtual N::base
{
};

struct Y: public X
{
};

int main()
{
    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> p( new X );
        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> p2( std::move( p ) );
        BOOST_TEST( N::base::instances == 1 );
        BOOST_TEST( p.get() == 0 );

        p2.reset();
        BOOST_TEST( N::base::instances == 0 );
    }

    {
        mars_boost::intrusive_ptr<Y> p( new Y );
        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> p2( std::move( p ) );
        BOOST_TEST( N::base::instances == 1 );
        BOOST_TEST( p.get() == 0 );

        p2.reset();
        BOOST_TEST( N::base::instances == 0 );
    }

    {
        mars_boost::intrusive_ptr<X> p( new X );
        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> p2;
        p2 = std::move( p );
        BOOST_TEST( N::base::instances == 1 );
        BOOST_TEST( p.get() == 0 );

        p2.reset();
        BOOST_TEST( N::base::instances == 0 );
    }

    {
        mars_boost::intrusive_ptr<X> p( new X );
        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> p2( new X );
        BOOST_TEST( N::base::instances == 2 );
        p2 = std::move( p );
        BOOST_TEST( N::base::instances == 1 );
        BOOST_TEST( p.get() == 0 );

        p2.reset();
        BOOST_TEST( N::base::instances == 0 );
    }

    {
        mars_boost::intrusive_ptr<Y> p( new Y );
        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> p2;
        p2 = std::move( p );
        BOOST_TEST( N::base::instances == 1 );
        BOOST_TEST( p.get() == 0 );

        p2.reset();
        BOOST_TEST( N::base::instances == 0 );
    }

    {
        mars_boost::intrusive_ptr<Y> p( new Y );
        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> p2( new X );
        BOOST_TEST( N::base::instances == 2 );
        p2 = std::move( p );
        BOOST_TEST( N::base::instances == 1 );
        BOOST_TEST( p.get() == 0 );

        p2.reset();
        BOOST_TEST( N::base::instances == 0 );
    }

    {
        mars_boost::intrusive_ptr<X> px( new Y );

        X * px2 = px.get();

        mars_boost::intrusive_ptr<Y> py = mars_boost::static_pointer_cast<Y>( std::move( px ) );
        BOOST_TEST( py.get() == px2 );
        BOOST_TEST( px.get() == 0 );
        BOOST_TEST( py->use_count() == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X const> px( new X );

        X const * px2 = px.get();

        mars_boost::intrusive_ptr<X> px3 = mars_boost::const_pointer_cast<X>( std::move( px ) );
        BOOST_TEST( px3.get() == px2 );
        BOOST_TEST( px.get() == 0 );
        BOOST_TEST( px3->use_count() == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new Y );

        X * px2 = px.get();

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>( std::move( px ) );
        BOOST_TEST( py.get() == px2 );
        BOOST_TEST( px.get() == 0 );
        BOOST_TEST( py->use_count() == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );

        X * px2 = px.get();

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>( std::move( px ) );
        BOOST_TEST( py.get() == 0 );
        BOOST_TEST( px.get() == px2 );
        BOOST_TEST( px->use_count() == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    return mars_boost::report_errors();
}

#else // defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

int main()
{
    return 0;
}

#endif
