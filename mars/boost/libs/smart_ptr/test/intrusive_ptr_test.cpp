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
//  intrusive_ptr_test.cpp
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
#include <algorithm>
#include <functional>

//

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

//

namespace n_element_type
{

void f(X &)
{
}

void test()
{
    typedef mars_boost::intrusive_ptr<X>::element_type T;
    T t;
    f(t);
}

} // namespace n_element_type

namespace n_constructors
{

void default_constructor()
{
    mars_boost::intrusive_ptr<X> px;
    BOOST_TEST(px.get() == 0);
}

void pointer_constructor()
{
    {
        mars_boost::intrusive_ptr<X> px(0);
        BOOST_TEST(px.get() == 0);
    }

    {
        mars_boost::intrusive_ptr<X> px(0, false);
        BOOST_TEST(px.get() == 0);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        X * p = new X;
        BOOST_TEST(p->use_count() == 0);

        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> px(p);
        BOOST_TEST(px.get() == p);
        BOOST_TEST(px->use_count() == 1);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        X * p = new X;
        BOOST_TEST(p->use_count() == 0);

        BOOST_TEST( N::base::instances == 1 );

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
        using mars_boost::intrusive_ptr_add_ref;
#endif
        intrusive_ptr_add_ref(p);
        BOOST_TEST(p->use_count() == 1);

        mars_boost::intrusive_ptr<X> px(p, false);
        BOOST_TEST(px.get() == p);
        BOOST_TEST(px->use_count() == 1);
    }

    BOOST_TEST( N::base::instances == 0 );
}

void copy_constructor()
{
    {
        mars_boost::intrusive_ptr<X> px;
        mars_boost::intrusive_ptr<X> px2(px);
        BOOST_TEST(px2.get() == px.get());
    }

    {
        mars_boost::intrusive_ptr<Y> py;
        mars_boost::intrusive_ptr<X> px(py);
        BOOST_TEST(px.get() == py.get());
    }

    {
        mars_boost::intrusive_ptr<X> px(0);
        mars_boost::intrusive_ptr<X> px2(px);
        BOOST_TEST(px2.get() == px.get());
    }

    {
        mars_boost::intrusive_ptr<Y> py(0);
        mars_boost::intrusive_ptr<X> px(py);
        BOOST_TEST(px.get() == py.get());
    }

    {
        mars_boost::intrusive_ptr<X> px(0, false);
        mars_boost::intrusive_ptr<X> px2(px);
        BOOST_TEST(px2.get() == px.get());
    }

    {
        mars_boost::intrusive_ptr<Y> py(0, false);
        mars_boost::intrusive_ptr<X> px(py);
        BOOST_TEST(px.get() == py.get());
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px(new X);
        mars_boost::intrusive_ptr<X> px2(px);
        BOOST_TEST( px2.get() == px.get() );

        BOOST_TEST( N::base::instances == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<Y> py(new Y);
        mars_boost::intrusive_ptr<X> px(py);
        BOOST_TEST( px.get() == py.get() );

        BOOST_TEST( N::base::instances == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );
}

void test()
{
    default_constructor();
    pointer_constructor();
    copy_constructor();
}

} // namespace n_constructors

namespace n_destructor
{

void test()
{
    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px(new X);
        BOOST_TEST(px->use_count() == 1);

        BOOST_TEST( N::base::instances == 1 );

        {
            mars_boost::intrusive_ptr<X> px2(px);
            BOOST_TEST(px->use_count() == 2);
        }

        BOOST_TEST(px->use_count() == 1);
    }

    BOOST_TEST( N::base::instances == 0 );
}

} // namespace n_destructor

namespace n_assignment
{

void copy_assignment()
{
    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> p1;

        p1 = p1;

        BOOST_TEST(p1 == p1);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        mars_boost::intrusive_ptr<X> p2;

        p1 = p2;

        BOOST_TEST(p1 == p2);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        mars_boost::intrusive_ptr<X> p3(p1);

        p1 = p3;

        BOOST_TEST(p1 == p3);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        BOOST_TEST(N::base::instances == 0);

        mars_boost::intrusive_ptr<X> p4(new X);

        BOOST_TEST(N::base::instances == 1);

        p1 = p4;

        BOOST_TEST(N::base::instances == 1);

        BOOST_TEST(p1 == p4);

        BOOST_TEST(p1->use_count() == 2);

        p1 = p2;

        BOOST_TEST(p1 == p2);
        BOOST_TEST(N::base::instances == 1);

        p4 = p3;

        BOOST_TEST(p4 == p3);
        BOOST_TEST(N::base::instances == 0);
    }
}

void conversion_assignment()
{
    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> p1;

        mars_boost::intrusive_ptr<Y> p2;

        p1 = p2;

        BOOST_TEST(p1 == p2);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        BOOST_TEST(N::base::instances == 0);

        mars_boost::intrusive_ptr<Y> p4(new Y);

        BOOST_TEST(N::base::instances == 1);
        BOOST_TEST(p4->use_count() == 1);

        mars_boost::intrusive_ptr<X> p5(p4);
        BOOST_TEST(p4->use_count() == 2);

        p1 = p4;

        BOOST_TEST(N::base::instances == 1);

        BOOST_TEST(p1 == p4);

        BOOST_TEST(p1->use_count() == 3);
        BOOST_TEST(p4->use_count() == 3);

        p1 = p2;

        BOOST_TEST(p1 == p2);
        BOOST_TEST(N::base::instances == 1);
        BOOST_TEST(p4->use_count() == 2);

        p4 = p2;
        p5 = p2;

        BOOST_TEST(p4 == p2);
        BOOST_TEST(N::base::instances == 0);
    }
}

void pointer_assignment()
{
    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> p1;

        p1 = p1.get();

        BOOST_TEST(p1 == p1);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        mars_boost::intrusive_ptr<X> p2;

        p1 = p2.get();

        BOOST_TEST(p1 == p2);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        mars_boost::intrusive_ptr<X> p3(p1);

        p1 = p3.get();

        BOOST_TEST(p1 == p3);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        BOOST_TEST(N::base::instances == 0);

        mars_boost::intrusive_ptr<X> p4(new X);

        BOOST_TEST(N::base::instances == 1);

        p1 = p4.get();

        BOOST_TEST(N::base::instances == 1);

        BOOST_TEST(p1 == p4);

        BOOST_TEST(p1->use_count() == 2);

        p1 = p2.get();

        BOOST_TEST(p1 == p2);
        BOOST_TEST(N::base::instances == 1);

        p4 = p3.get();

        BOOST_TEST(p4 == p3);
        BOOST_TEST(N::base::instances == 0);
    }

    {
        mars_boost::intrusive_ptr<X> p1;

        mars_boost::intrusive_ptr<Y> p2;

        p1 = p2.get();

        BOOST_TEST(p1 == p2);
        BOOST_TEST(p1? false: true);
        BOOST_TEST(!p1);
        BOOST_TEST(p1.get() == 0);

        BOOST_TEST(N::base::instances == 0);

        mars_boost::intrusive_ptr<Y> p4(new Y);

        BOOST_TEST(N::base::instances == 1);
        BOOST_TEST(p4->use_count() == 1);

        mars_boost::intrusive_ptr<X> p5(p4);
        BOOST_TEST(p4->use_count() == 2);

        p1 = p4.get();

        BOOST_TEST(N::base::instances == 1);

        BOOST_TEST(p1 == p4);

        BOOST_TEST(p1->use_count() == 3);
        BOOST_TEST(p4->use_count() == 3);

        p1 = p2.get();

        BOOST_TEST(p1 == p2);
        BOOST_TEST(N::base::instances == 1);
        BOOST_TEST(p4->use_count() == 2);

        p4 = p2.get();
        p5 = p2.get();

        BOOST_TEST(p4 == p2);
        BOOST_TEST(N::base::instances == 0);
    }
}

void test()
{
    copy_assignment();
    conversion_assignment();
    pointer_assignment();
}

} // namespace n_assignment

namespace n_reset
{

void test()
{
    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px;
        BOOST_TEST( px.get() == 0 );

        px.reset();
        BOOST_TEST( px.get() == 0 );

        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );
        BOOST_TEST( N::base::instances == 1 );

        px.reset( p );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );

        px.reset();
        BOOST_TEST( px.get() == 0 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );
        BOOST_TEST( N::base::instances == 1 );

        px.reset( 0 );
        BOOST_TEST( px.get() == 0 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );
        BOOST_TEST( N::base::instances == 1 );

        px.reset( 0, false );
        BOOST_TEST( px.get() == 0 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );
        BOOST_TEST( N::base::instances == 1 );

        px.reset( 0, true );
        BOOST_TEST( px.get() == 0 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );

        BOOST_TEST( N::base::instances == 1 );

        mars_boost::intrusive_ptr<X> px;
        BOOST_TEST( px.get() == 0 );

        px.reset( p, true );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );

        BOOST_TEST( N::base::instances == 1 );

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
        using mars_boost::intrusive_ptr_add_ref;
#endif
        intrusive_ptr_add_ref( p );
        BOOST_TEST( p->use_count() == 1 );

        mars_boost::intrusive_ptr<X> px;
        BOOST_TEST( px.get() == 0 );

        px.reset( p, false );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );
        BOOST_TEST( px.get() != 0 );
        BOOST_TEST( px->use_count() == 1 );

        BOOST_TEST( N::base::instances == 1 );

        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );

        BOOST_TEST( N::base::instances == 2 );

        px.reset( p );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );

        BOOST_TEST( N::base::instances == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );
        BOOST_TEST( px.get() != 0 );
        BOOST_TEST( px->use_count() == 1 );

        BOOST_TEST( N::base::instances == 1 );

        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );

        BOOST_TEST( N::base::instances == 2 );

        px.reset( p, true );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );

        BOOST_TEST( N::base::instances == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px( new X );
        BOOST_TEST( px.get() != 0 );
        BOOST_TEST( px->use_count() == 1 );

        BOOST_TEST( N::base::instances == 1 );

        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
        using mars_boost::intrusive_ptr_add_ref;
#endif
        intrusive_ptr_add_ref( p );
        BOOST_TEST( p->use_count() == 1 );

        BOOST_TEST( N::base::instances == 2 );

        px.reset( p, false );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );

        BOOST_TEST( N::base::instances == 1 );
    }

    BOOST_TEST( N::base::instances == 0 );
}

} // namespace n_reset

namespace n_access
{

void test()
{
    {
        mars_boost::intrusive_ptr<X> px;
        BOOST_TEST(px? false: true);
        BOOST_TEST(!px);

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
        using mars_boost::get_pointer;
#endif

        BOOST_TEST(get_pointer(px) == px.get());
    }

    {
        mars_boost::intrusive_ptr<X> px(0);
        BOOST_TEST(px? false: true);
        BOOST_TEST(!px);

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
        using mars_boost::get_pointer;
#endif

        BOOST_TEST(get_pointer(px) == px.get());
    }

    {
        mars_boost::intrusive_ptr<X> px(new X);
        BOOST_TEST(px? true: false);
        BOOST_TEST(!!px);
        BOOST_TEST(&*px == px.get());
        BOOST_TEST(px.operator ->() == px.get());

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
        using mars_boost::get_pointer;
#endif

        BOOST_TEST(get_pointer(px) == px.get());
    }

    {
        mars_boost::intrusive_ptr<X> px;
        X* detached = px.detach();
        BOOST_TEST( px.get() == 0 );
        BOOST_TEST( detached == 0 );
    }

    {
        X * p = new X;
        BOOST_TEST( p->use_count() == 0 );

        mars_boost::intrusive_ptr<X> px( p );
        BOOST_TEST( px.get() == p );
        BOOST_TEST( px->use_count() == 1 );

        X * detached = px.detach();
        BOOST_TEST( px.get() == 0 );

        BOOST_TEST( detached == p );
        BOOST_TEST( detached->use_count() == 1 );

        delete detached;
    }
}

} // namespace n_access

namespace n_swap
{

void test()
{
    {
        mars_boost::intrusive_ptr<X> px;
        mars_boost::intrusive_ptr<X> px2;

        px.swap(px2);

        BOOST_TEST(px.get() == 0);
        BOOST_TEST(px2.get() == 0);

        using std::swap;
        swap(px, px2);

        BOOST_TEST(px.get() == 0);
        BOOST_TEST(px2.get() == 0);
    }

    {
        X * p = new X;
        mars_boost::intrusive_ptr<X> px;
        mars_boost::intrusive_ptr<X> px2(p);
        mars_boost::intrusive_ptr<X> px3(px2);

        px.swap(px2);

        BOOST_TEST(px.get() == p);
        BOOST_TEST(px->use_count() == 2);
        BOOST_TEST(px2.get() == 0);
        BOOST_TEST(px3.get() == p);
        BOOST_TEST(px3->use_count() == 2);

        using std::swap;
        swap(px, px2);

        BOOST_TEST(px.get() == 0);
        BOOST_TEST(px2.get() == p);
        BOOST_TEST(px2->use_count() == 2);
        BOOST_TEST(px3.get() == p);
        BOOST_TEST(px3->use_count() == 2);
    }

    {
        X * p1 = new X;
        X * p2 = new X;
        mars_boost::intrusive_ptr<X> px(p1);
        mars_boost::intrusive_ptr<X> px2(p2);
        mars_boost::intrusive_ptr<X> px3(px2);

        px.swap(px2);

        BOOST_TEST(px.get() == p2);
        BOOST_TEST(px->use_count() == 2);
        BOOST_TEST(px2.get() == p1);
        BOOST_TEST(px2->use_count() == 1);
        BOOST_TEST(px3.get() == p2);
        BOOST_TEST(px3->use_count() == 2);

        using std::swap;
        swap(px, px2);

        BOOST_TEST(px.get() == p1);
        BOOST_TEST(px->use_count() == 1);
        BOOST_TEST(px2.get() == p2);
        BOOST_TEST(px2->use_count() == 2);
        BOOST_TEST(px3.get() == p2);
        BOOST_TEST(px3->use_count() == 2);
    }
}

} // namespace n_swap

namespace n_comparison
{

template<class T, class U> void test2(mars_boost::intrusive_ptr<T> const & p, mars_boost::intrusive_ptr<U> const & q)
{
    BOOST_TEST((p == q) == (p.get() == q.get()));
    BOOST_TEST((p != q) == (p.get() != q.get()));
}

template<class T> void test3(mars_boost::intrusive_ptr<T> const & p, mars_boost::intrusive_ptr<T> const & q)
{
    BOOST_TEST((p == q) == (p.get() == q.get()));
    BOOST_TEST((p.get() == q) == (p.get() == q.get()));
    BOOST_TEST((p == q.get()) == (p.get() == q.get()));
    BOOST_TEST((p != q) == (p.get() != q.get()));
    BOOST_TEST((p.get() != q) == (p.get() != q.get()));
    BOOST_TEST((p != q.get()) == (p.get() != q.get()));

    // 'less' moved here as a g++ 2.9x parse error workaround
    std::less<T*> less;
    BOOST_TEST((p < q) == less(p.get(), q.get()));
}

void test()
{
    {
        mars_boost::intrusive_ptr<X> px;
        test3(px, px);

        mars_boost::intrusive_ptr<X> px2;
        test3(px, px2);

        mars_boost::intrusive_ptr<X> px3(px);
        test3(px3, px3);
        test3(px, px3);
    }

    {
        mars_boost::intrusive_ptr<X> px;

        mars_boost::intrusive_ptr<X> px2(new X);
        test3(px, px2);
        test3(px2, px2);

        mars_boost::intrusive_ptr<X> px3(new X);
        test3(px2, px3);

        mars_boost::intrusive_ptr<X> px4(px2);
        test3(px2, px4);
        test3(px4, px4);
    }

    {
        mars_boost::intrusive_ptr<X> px(new X);

        mars_boost::intrusive_ptr<Y> py(new Y);
        test2(px, py);

        mars_boost::intrusive_ptr<X> px2(py);
        test2(px2, py);
        test3(px, px2);
        test3(px2, px2);
    }
}

} // namespace n_comparison

namespace n_static_cast
{

void test()
{
    {
        mars_boost::intrusive_ptr<X> px(new Y);

        mars_boost::intrusive_ptr<Y> py = mars_boost::static_pointer_cast<Y>(px);
        BOOST_TEST(px.get() == py.get());
        BOOST_TEST(px->use_count() == 2);
        BOOST_TEST(py->use_count() == 2);

        mars_boost::intrusive_ptr<X> px2(py);
        BOOST_TEST(px2.get() == px.get());
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<Y> py = mars_boost::static_pointer_cast<Y>( mars_boost::intrusive_ptr<X>(new Y) );
        BOOST_TEST(py.get() != 0);
        BOOST_TEST(py->use_count() == 1);
    }

    BOOST_TEST( N::base::instances == 0 );
}

} // namespace n_static_cast

namespace n_const_cast
{

void test()
{
    {
        mars_boost::intrusive_ptr<X const> px;

        mars_boost::intrusive_ptr<X> px2 = mars_boost::const_pointer_cast<X>(px);
        BOOST_TEST(px2.get() == 0);
    }

    {
        mars_boost::intrusive_ptr<X> px2 = mars_boost::const_pointer_cast<X>( mars_boost::intrusive_ptr<X const>() );
        BOOST_TEST(px2.get() == 0);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X const> px(new X);

        mars_boost::intrusive_ptr<X> px2 = mars_boost::const_pointer_cast<X>(px);
        BOOST_TEST(px2.get() == px.get());
        BOOST_TEST(px2->use_count() == 2);
        BOOST_TEST(px->use_count() == 2);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px = mars_boost::const_pointer_cast<X>( mars_boost::intrusive_ptr<X const>(new X) );
        BOOST_TEST(px.get() != 0);
        BOOST_TEST(px->use_count() == 1);
    }

    BOOST_TEST( N::base::instances == 0 );
}

} // namespace n_const_cast

namespace n_dynamic_cast
{

void test()
{
    {
        mars_boost::intrusive_ptr<X> px;

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>(px);
        BOOST_TEST(py.get() == 0);
    }

    {
        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>( mars_boost::intrusive_ptr<X>() );
        BOOST_TEST(py.get() == 0);
    }

    {
        mars_boost::intrusive_ptr<X> px(static_cast<X*>(0));

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>(px);
        BOOST_TEST(py.get() == 0);
    }

    {
        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>( mars_boost::intrusive_ptr<X>(static_cast<X*>(0)) );
        BOOST_TEST(py.get() == 0);
    }

    {
        mars_boost::intrusive_ptr<X> px(new X);

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>(px);
        BOOST_TEST(py.get() == 0);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>( mars_boost::intrusive_ptr<X>(new X) );
        BOOST_TEST(py.get() == 0);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px(new Y);

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>(px);
        BOOST_TEST(py.get() == px.get());
        BOOST_TEST(py->use_count() == 2);
        BOOST_TEST(px->use_count() == 2);
    }

    BOOST_TEST( N::base::instances == 0 );

    {
        mars_boost::intrusive_ptr<X> px(new Y);

        mars_boost::intrusive_ptr<Y> py = mars_boost::dynamic_pointer_cast<Y>( mars_boost::intrusive_ptr<X>(new Y) );
        BOOST_TEST(py.get() != 0);
        BOOST_TEST(py->use_count() == 1);
    }

    BOOST_TEST( N::base::instances == 0 );
}

} // namespace n_dynamic_cast

namespace n_transitive
{

struct X: public N::base
{
    mars_boost::intrusive_ptr<X> next;
};

void test()
{
    mars_boost::intrusive_ptr<X> p(new X);
    p->next = mars_boost::intrusive_ptr<X>(new X);
    BOOST_TEST(!p->next->next);
    p = p->next;
    BOOST_TEST(!p->next);
}

} // namespace n_transitive

namespace n_report_1
{

class foo: public N::base
{ 
public: 

    foo(): m_self(this)
    {
    } 

    void suicide()
    {
        m_self = 0;
    }

private:

    mars_boost::intrusive_ptr<foo> m_self;
}; 

void test()
{
    foo * foo_ptr = new foo;
    foo_ptr->suicide();
} 

} // namespace n_report_1

int main()
{
    n_element_type::test();
    n_constructors::test();
    n_destructor::test();
    n_assignment::test();
    n_reset::test();
    n_access::test();
    n_swap::test();
    n_comparison::test();
    n_static_cast::test();
    n_const_cast::test();
    n_dynamic_cast::test();

    n_transitive::test();
    n_report_1::test();

    return mars_boost::report_errors();
}
