#include <boost/config.hpp>

#if defined(BOOST_MSVC)

#pragma warning(disable: 4786)  // identifier truncated in debug info
#pragma warning(disable: 4710)  // function not inlined
#pragma warning(disable: 4711)  // function selected for automatic inline expansion
#pragma warning(disable: 4514)  // unreferenced inline removed
#pragma warning(disable: 4355)  // 'this' : used in base member initializer list

#if (BOOST_MSVC >= 1310)
#pragma warning(disable: 4675)  // resolved overload found with Koenig lookup
#endif

#endif

//
//  weak_ptr_test.cpp
//
//  Copyright (c) 2002-2005 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/core/lightweight_test.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <map>
#include <vector>

//

namespace n_element_type
{

void f(int &)
{
}

void test()
{
    typedef mars_boost::weak_ptr<int>::element_type T;
    T t;
    f(t);
}

} // namespace n_element_type

class incomplete;

mars_boost::shared_ptr<incomplete> create_incomplete();

struct X
{
    int dummy;
};

struct Y
{
    int dummy2;
};

struct Z: public X, public virtual Y
{
};

namespace n_constructors
{

void default_constructor()
{
    {
        mars_boost::weak_ptr<int> wp;
        BOOST_TEST(wp.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<void> wp;
        BOOST_TEST(wp.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<incomplete> wp;
        BOOST_TEST(wp.use_count() == 0);
    }
}

void shared_ptr_constructor()
{
    {
        mars_boost::shared_ptr<int> sp;

        mars_boost::weak_ptr<int> wp(sp);
        BOOST_TEST(wp.use_count() == sp.use_count());

        mars_boost::weak_ptr<void> wp2(sp);
        BOOST_TEST(wp2.use_count() == sp.use_count());
    }

    {
        mars_boost::shared_ptr<int> sp(static_cast<int*>(0));

        {
            mars_boost::weak_ptr<int> wp(sp);
            BOOST_TEST(wp.use_count() == sp.use_count());
            BOOST_TEST(wp.use_count() == 1);
            mars_boost::shared_ptr<int> sp2(wp);
            BOOST_TEST(wp.use_count() == 2);
            BOOST_TEST(!(sp < sp2 || sp2 < sp));
        }

        {
            mars_boost::weak_ptr<void> wp(sp);
            BOOST_TEST(wp.use_count() == sp.use_count());
            BOOST_TEST(wp.use_count() == 1);
            mars_boost::shared_ptr<void> sp2(wp);
            BOOST_TEST(wp.use_count() == 2);
            BOOST_TEST(!(sp < sp2 || sp2 < sp));
        }
    }

    {
        mars_boost::shared_ptr<int> sp(new int);

        {
            mars_boost::weak_ptr<int> wp(sp);
            BOOST_TEST(wp.use_count() == sp.use_count());
            BOOST_TEST(wp.use_count() == 1);
            mars_boost::shared_ptr<int> sp2(wp);
            BOOST_TEST(wp.use_count() == 2);
            BOOST_TEST(!(sp < sp2 || sp2 < sp));
        }

        {
            mars_boost::weak_ptr<void> wp(sp);
            BOOST_TEST(wp.use_count() == sp.use_count());
            BOOST_TEST(wp.use_count() == 1);
            mars_boost::shared_ptr<void> sp2(wp);
            BOOST_TEST(wp.use_count() == 2);
            BOOST_TEST(!(sp < sp2 || sp2 < sp));
        }
    }

    {
        mars_boost::shared_ptr<void> sp;

        mars_boost::weak_ptr<void> wp(sp);
        BOOST_TEST(wp.use_count() == sp.use_count());
    }

    {
        mars_boost::shared_ptr<void> sp(static_cast<int*>(0));

        mars_boost::weak_ptr<void> wp(sp);
        BOOST_TEST(wp.use_count() == sp.use_count());
        BOOST_TEST(wp.use_count() == 1);
        mars_boost::shared_ptr<void> sp2(wp);
        BOOST_TEST(wp.use_count() == 2);
        BOOST_TEST(!(sp < sp2 || sp2 < sp));
    }

    {
        mars_boost::shared_ptr<void> sp(new int);

        mars_boost::weak_ptr<void> wp(sp);
        BOOST_TEST(wp.use_count() == sp.use_count());
        BOOST_TEST(wp.use_count() == 1);
        mars_boost::shared_ptr<void> sp2(wp);
        BOOST_TEST(wp.use_count() == 2);
        BOOST_TEST(!(sp < sp2 || sp2 < sp));
    }

    {
        mars_boost::shared_ptr<incomplete> sp;

        mars_boost::weak_ptr<incomplete> wp(sp);
        BOOST_TEST(wp.use_count() == sp.use_count());

        mars_boost::weak_ptr<void> wp2(sp);
        BOOST_TEST(wp2.use_count() == sp.use_count());
    }

    {
        mars_boost::shared_ptr<incomplete> sp = create_incomplete();

        {
            mars_boost::weak_ptr<incomplete> wp(sp);
            BOOST_TEST(wp.use_count() == sp.use_count());
            BOOST_TEST(wp.use_count() == 1);
            mars_boost::shared_ptr<incomplete> sp2(wp);
            BOOST_TEST(wp.use_count() == 2);
            BOOST_TEST(!(sp < sp2 || sp2 < sp));
        }

        {
            mars_boost::weak_ptr<void> wp(sp);
            BOOST_TEST(wp.use_count() == sp.use_count());
            BOOST_TEST(wp.use_count() == 1);
            mars_boost::shared_ptr<void> sp2(wp);
            BOOST_TEST(wp.use_count() == 2);
            BOOST_TEST(!(sp < sp2 || sp2 < sp));
        }
    }

    {
        mars_boost::shared_ptr<void> sp = create_incomplete();

        mars_boost::weak_ptr<void> wp(sp);
        BOOST_TEST(wp.use_count() == sp.use_count());
        BOOST_TEST(wp.use_count() == 1);
        mars_boost::shared_ptr<void> sp2(wp);
        BOOST_TEST(wp.use_count() == 2);
        BOOST_TEST(!(sp < sp2 || sp2 < sp));
    }
}

void copy_constructor()
{
    {
        mars_boost::weak_ptr<int> wp;
        mars_boost::weak_ptr<int> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<void> wp;
        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<incomplete> wp;
        mars_boost::weak_ptr<incomplete> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 0);
    }

    {
        mars_boost::shared_ptr<int> sp(static_cast<int*>(0));
        mars_boost::weak_ptr<int> wp(sp);

        mars_boost::weak_ptr<int> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<int> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<int> sp(new int);
        mars_boost::weak_ptr<int> wp(sp);

        mars_boost::weak_ptr<int> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<int> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<void> sp(static_cast<int*>(0));
        mars_boost::weak_ptr<void> wp(sp);

        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<void> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<void> sp(new int);
        mars_boost::weak_ptr<void> wp(sp);

        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<void> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<incomplete> sp = create_incomplete();
        mars_boost::weak_ptr<incomplete> wp(sp);

        mars_boost::weak_ptr<incomplete> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<incomplete> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }
}

void conversion_constructor()
{
    {
        mars_boost::weak_ptr<int> wp;
        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<incomplete> wp;
        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<Z> wp;

        mars_boost::weak_ptr<X> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 0);

        mars_boost::weak_ptr<Y> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
    }

    {
        mars_boost::shared_ptr<int> sp(static_cast<int*>(0));
        mars_boost::weak_ptr<int> wp(sp);

        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<void> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<int> sp(new int);
        mars_boost::weak_ptr<int> wp(sp);

        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<void> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<incomplete> sp = create_incomplete();
        mars_boost::weak_ptr<incomplete> wp(sp);

        mars_boost::weak_ptr<void> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<void> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<Z> sp(static_cast<Z*>(0));
        mars_boost::weak_ptr<Z> wp(sp);

        mars_boost::weak_ptr<X> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<X> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<Z> sp(static_cast<Z*>(0));
        mars_boost::weak_ptr<Z> wp(sp);

        mars_boost::weak_ptr<Y> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<Y> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<Z> sp(new Z);
        mars_boost::weak_ptr<Z> wp(sp);

        mars_boost::weak_ptr<X> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<X> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }

    {
        mars_boost::shared_ptr<Z> sp(new Z);
        mars_boost::weak_ptr<Z> wp(sp);

        mars_boost::weak_ptr<Y> wp2(wp);
        BOOST_TEST(wp2.use_count() == wp.use_count());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        sp.reset();
        BOOST_TEST(!(wp < wp2 || wp2 < wp));

        mars_boost::weak_ptr<Y> wp3(wp);
        BOOST_TEST(wp3.use_count() == wp.use_count());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));
    }
}

void test()
{
    default_constructor();
    shared_ptr_constructor();
    copy_constructor();
    conversion_constructor();
}

} // namespace n_constructors

namespace n_assignment
{

template<class T> void copy_assignment(mars_boost::shared_ptr<T> & sp)
{
    BOOST_TEST(sp.unique());

    mars_boost::weak_ptr<T> p1;

    p1 = p1;
    BOOST_TEST(p1.use_count() == 0);

    mars_boost::weak_ptr<T> p2;

    p1 = p2;
    BOOST_TEST(p1.use_count() == 0);

    mars_boost::weak_ptr<T> p3(p1);

    p1 = p3;
    BOOST_TEST(p1.use_count() == 0);

    mars_boost::weak_ptr<T> p4(sp);

    p4 = p4;
    BOOST_TEST(p4.use_count() == 1);

    p1 = p4;
    BOOST_TEST(p1.use_count() == 1);

    p4 = p2;
    BOOST_TEST(p4.use_count() == 0);

    sp.reset();

    p1 = p1;
    BOOST_TEST(p1.use_count() == 0);

    p4 = p1;
    BOOST_TEST(p4.use_count() == 0);
}

void conversion_assignment()
{
    {
        mars_boost::weak_ptr<void> p1;

        mars_boost::weak_ptr<incomplete> p2;

        p1 = p2;
        BOOST_TEST(p1.use_count() == 0);

        mars_boost::shared_ptr<incomplete> sp = create_incomplete();
        mars_boost::weak_ptr<incomplete> p3(sp);

        p1 = p3;
        BOOST_TEST(p1.use_count() == 1);

        sp.reset();

        p1 = p3;
        BOOST_TEST(p1.use_count() == 0);

        p1 = p2;
        BOOST_TEST(p1.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<X> p1;

        mars_boost::weak_ptr<Z> p2;

        p1 = p2;
        BOOST_TEST(p1.use_count() == 0);

        mars_boost::shared_ptr<Z> sp(new Z);
        mars_boost::weak_ptr<Z> p3(sp);

        p1 = p3;
        BOOST_TEST(p1.use_count() == 1);

        sp.reset();

        p1 = p3;
        BOOST_TEST(p1.use_count() == 0);

        p1 = p2;
        BOOST_TEST(p1.use_count() == 0);
    }

    {
        mars_boost::weak_ptr<Y> p1;

        mars_boost::weak_ptr<Z> p2;

        p1 = p2;
        BOOST_TEST(p1.use_count() == 0);

        mars_boost::shared_ptr<Z> sp(new Z);
        mars_boost::weak_ptr<Z> p3(sp);

        p1 = p3;
        BOOST_TEST(p1.use_count() == 1);

        sp.reset();

        p1 = p3;
        BOOST_TEST(p1.use_count() == 0);

        p1 = p2;
        BOOST_TEST(p1.use_count() == 0);
    }
}

template<class T, class U> void shared_ptr_assignment(mars_boost::shared_ptr<U> & sp, T * = 0)
{
    BOOST_TEST(sp.unique());

    mars_boost::weak_ptr<T> p1;
    mars_boost::weak_ptr<T> p2(p1);
    mars_boost::weak_ptr<T> p3(sp);
    mars_boost::weak_ptr<T> p4(p3);

    p1 = sp;
    BOOST_TEST(p1.use_count() == 1);

    p2 = sp;
    BOOST_TEST(p2.use_count() == 1);

    p3 = sp;
    BOOST_TEST(p3.use_count() == 1);

    p4 = sp;
    BOOST_TEST(p4.use_count() == 1);

    sp.reset();

    BOOST_TEST(p1.use_count() == 0);
    BOOST_TEST(p2.use_count() == 0);
    BOOST_TEST(p3.use_count() == 0);
    BOOST_TEST(p4.use_count() == 0);

    p1 = sp;
}

void test()
{
    {
        mars_boost::shared_ptr<int> p( new int );
        copy_assignment( p );
    }

    {
        mars_boost::shared_ptr<X> p( new X );
        copy_assignment( p );
    }

    {
        mars_boost::shared_ptr<void> p( new int );
        copy_assignment( p );
    }

    {
        mars_boost::shared_ptr<incomplete> p = create_incomplete();
        copy_assignment( p );
    }

    conversion_assignment();

    {
        mars_boost::shared_ptr<int> p( new int );
        shared_ptr_assignment<int>( p );
    }

    {
        mars_boost::shared_ptr<int> p( new int );
        shared_ptr_assignment<void>( p );
    }

    {
        mars_boost::shared_ptr<X> p( new X );
        shared_ptr_assignment<X>( p );
    }

    {
        mars_boost::shared_ptr<X> p( new X );
        shared_ptr_assignment<void>( p );
    }

    {
        mars_boost::shared_ptr<void> p( new int );
        shared_ptr_assignment<void>( p );
    }

    {
        mars_boost::shared_ptr<incomplete> p = create_incomplete();
        shared_ptr_assignment<incomplete>( p );
    }

    {
        mars_boost::shared_ptr<incomplete> p = create_incomplete();
        shared_ptr_assignment<void>( p );
    }
}

} // namespace n_assignment

namespace n_reset
{

template<class T, class U> void test2( mars_boost::shared_ptr<U> & sp, T * = 0 )
{
    BOOST_TEST(sp.unique());

    mars_boost::weak_ptr<T> p1;
    mars_boost::weak_ptr<T> p2(p1);
    mars_boost::weak_ptr<T> p3(sp);
    mars_boost::weak_ptr<T> p4(p3);
    mars_boost::weak_ptr<T> p5(sp);
    mars_boost::weak_ptr<T> p6(p5);

    p1.reset();
    BOOST_TEST(p1.use_count() == 0);

    p2.reset();
    BOOST_TEST(p2.use_count() == 0);

    p3.reset();
    BOOST_TEST(p3.use_count() == 0);

    p4.reset();
    BOOST_TEST(p4.use_count() == 0);

    sp.reset();

    p5.reset();
    BOOST_TEST(p5.use_count() == 0);

    p6.reset();
    BOOST_TEST(p6.use_count() == 0);
}

void test()
{
    {
        mars_boost::shared_ptr<int> p( new int );
        test2<int>( p );
    }

    {
        mars_boost::shared_ptr<int> p( new int );
        test2<void>( p );
    }

    {
        mars_boost::shared_ptr<X> p( new X );
        test2<X>( p );
    }

    {
        mars_boost::shared_ptr<X> p( new X );
        test2<void>( p );
    }

    {
        mars_boost::shared_ptr<void> p( new int );
        test2<void>( p );
    }

    {
        mars_boost::shared_ptr<incomplete> p = create_incomplete();
        test2<incomplete>( p );
    }

    {
        mars_boost::shared_ptr<incomplete> p = create_incomplete();
        test2<void>( p );
    }
}

} // namespace n_reset

namespace n_use_count
{

void test()
{
    {
        mars_boost::weak_ptr<X> wp;
        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp.expired());

        mars_boost::weak_ptr<X> wp2;
        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp.expired());

        mars_boost::weak_ptr<X> wp3(wp);
        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp.expired());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(wp3.expired());
    }

    {
        mars_boost::shared_ptr<X> sp(static_cast<X*>(0));

        mars_boost::weak_ptr<X> wp(sp);
        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(!wp.expired());

        mars_boost::weak_ptr<X> wp2(sp);
        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(!wp.expired());

        mars_boost::weak_ptr<X> wp3(wp);
        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(!wp.expired());
        BOOST_TEST(wp3.use_count() == 1);
        BOOST_TEST(!wp3.expired());

        mars_boost::shared_ptr<X> sp2(sp);

        BOOST_TEST(wp.use_count() == 2);
        BOOST_TEST(!wp.expired());
        BOOST_TEST(wp2.use_count() == 2);
        BOOST_TEST(!wp2.expired());
        BOOST_TEST(wp3.use_count() == 2);
        BOOST_TEST(!wp3.expired());

        mars_boost::shared_ptr<void> sp3(sp);

        BOOST_TEST(wp.use_count() == 3);
        BOOST_TEST(!wp.expired());
        BOOST_TEST(wp2.use_count() == 3);
        BOOST_TEST(!wp2.expired());
        BOOST_TEST(wp3.use_count() == 3);
        BOOST_TEST(!wp3.expired());

        sp.reset();

        BOOST_TEST(wp.use_count() == 2);
        BOOST_TEST(!wp.expired());
        BOOST_TEST(wp2.use_count() == 2);
        BOOST_TEST(!wp2.expired());
        BOOST_TEST(wp3.use_count() == 2);
        BOOST_TEST(!wp3.expired());

        sp2.reset();

        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(!wp.expired());
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!wp2.expired());
        BOOST_TEST(wp3.use_count() == 1);
        BOOST_TEST(!wp3.expired());

        sp3.reset();

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp.expired());
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(wp2.expired());
        BOOST_TEST(wp3.use_count() == 0);
        BOOST_TEST(wp3.expired());
    }
}

} // namespace n_use_count

namespace n_swap
{

void test()
{
    {
        mars_boost::weak_ptr<X> wp;
        mars_boost::weak_ptr<X> wp2;

        wp.swap(wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 0);

        using std::swap;
        swap(wp, wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 0);
    }

    {
        mars_boost::shared_ptr<X> sp(new X);
        mars_boost::weak_ptr<X> wp;
        mars_boost::weak_ptr<X> wp2(sp);
        mars_boost::weak_ptr<X> wp3(sp);

        wp.swap(wp2);

        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));

        using std::swap;
        swap(wp, wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));

        sp.reset();

        wp.swap(wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));

        swap(wp, wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));
    }

    {
        mars_boost::shared_ptr<X> sp(new X);
        mars_boost::shared_ptr<X> sp2(new X);
        mars_boost::weak_ptr<X> wp(sp);
        mars_boost::weak_ptr<X> wp2(sp2);
        mars_boost::weak_ptr<X> wp3(sp2);

        wp.swap(wp2);

        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));

        using std::swap;
        swap(wp, wp2);

        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));

        sp.reset();

        wp.swap(wp2);

        BOOST_TEST(wp.use_count() == 1);
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));

        swap(wp, wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 1);
        BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));

        sp2.reset();

        wp.swap(wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(!(wp < wp3 || wp3 < wp));

        swap(wp, wp2);

        BOOST_TEST(wp.use_count() == 0);
        BOOST_TEST(wp2.use_count() == 0);
        BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));
    }
}

} // namespace n_swap

namespace n_comparison
{

void test()
{
    {
        mars_boost::weak_ptr<X> wp;
        BOOST_TEST(!(wp < wp));

        mars_boost::weak_ptr<X> wp2;
        BOOST_TEST(!(wp < wp2 && wp2 < wp));

        mars_boost::weak_ptr<X> wp3(wp);
        BOOST_TEST(!(wp3 < wp3));
        BOOST_TEST(!(wp < wp3 && wp3 < wp));
    }

    {
        mars_boost::shared_ptr<X> sp(new X);

        mars_boost::weak_ptr<X> wp(sp);
        BOOST_TEST(!(wp < wp));

        mars_boost::weak_ptr<X> wp2;
        BOOST_TEST(wp < wp2 || wp2 < wp);
        BOOST_TEST(!(wp < wp2 && wp2 < wp));

        bool b1 = wp < wp2;
        bool b2 = wp2 < wp;

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(wp2 < wp3 || wp3 < wp2);
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(wp4 < wp3 || wp3 < wp4);
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }

        sp.reset();

        BOOST_TEST(b1 == (wp < wp2));
        BOOST_TEST(b2 == (wp2 < wp));

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(wp2 < wp3 || wp3 < wp2);
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(wp4 < wp3 || wp3 < wp4);
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }
    }

    {
        mars_boost::shared_ptr<X> sp(new X);
        mars_boost::shared_ptr<X> sp2(new X);

        mars_boost::weak_ptr<X> wp(sp);
        mars_boost::weak_ptr<X> wp2(sp2);

        BOOST_TEST(wp < wp2 || wp2 < wp);
        BOOST_TEST(!(wp < wp2 && wp2 < wp));

        bool b1 = wp < wp2;
        bool b2 = wp2 < wp;

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(wp2 < wp3 || wp3 < wp2);
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(wp4 < wp3 || wp3 < wp4);
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }

        sp.reset();

        BOOST_TEST(b1 == (wp < wp2));
        BOOST_TEST(b2 == (wp2 < wp));

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(wp2 < wp3 || wp3 < wp2);
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(wp4 < wp3 || wp3 < wp4);
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }

        sp2.reset();

        BOOST_TEST(b1 == (wp < wp2));
        BOOST_TEST(b2 == (wp2 < wp));

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(wp2 < wp3 || wp3 < wp2);
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(wp4 < wp3 || wp3 < wp4);
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }
    }

    {
        mars_boost::shared_ptr<X> sp(new X);
        mars_boost::shared_ptr<X> sp2(sp);

        mars_boost::weak_ptr<X> wp(sp);
        mars_boost::weak_ptr<X> wp2(sp2);

        BOOST_TEST(!(wp < wp2 || wp2 < wp));
        BOOST_TEST(!(wp < wp2 && wp2 < wp));

        bool b1 = wp < wp2;
        bool b2 = wp2 < wp;

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(!(wp4 < wp3 || wp3 < wp4));
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }

        sp.reset();
        sp2.reset();

        BOOST_TEST(b1 == (wp < wp2));
        BOOST_TEST(b2 == (wp2 < wp));

        {
            mars_boost::weak_ptr<X> wp3(wp);

            BOOST_TEST(!(wp < wp3 || wp3 < wp));
            BOOST_TEST(!(wp < wp3 && wp3 < wp));

            BOOST_TEST(!(wp2 < wp3 || wp3 < wp2));
            BOOST_TEST(!(wp2 < wp3 && wp3 < wp2));

            mars_boost::weak_ptr<X> wp4(wp2);

            BOOST_TEST(!(wp4 < wp3 || wp3 < wp4));
            BOOST_TEST(!(wp4 < wp3 && wp3 < wp4));
        }
    }

    {
        mars_boost::shared_ptr<X> spx(new X);
        mars_boost::shared_ptr<Y> spy(new Y);
        mars_boost::shared_ptr<Z> spz(new Z);

        mars_boost::weak_ptr<X> px(spx);
        mars_boost::weak_ptr<Y> py(spy);
        mars_boost::weak_ptr<Z> pz(spz);

        BOOST_TEST(px < py || py < px);
        BOOST_TEST(px < pz || pz < px);
        BOOST_TEST(py < pz || pz < py);

        BOOST_TEST(!(px < py && py < px));
        BOOST_TEST(!(px < pz && pz < px));
        BOOST_TEST(!(py < pz && pz < py));

        mars_boost::weak_ptr<void> pvx(px);
        BOOST_TEST(!(pvx < pvx));

        mars_boost::weak_ptr<void> pvy(py);
        BOOST_TEST(!(pvy < pvy));

        mars_boost::weak_ptr<void> pvz(pz);
        BOOST_TEST(!(pvz < pvz));

        BOOST_TEST(pvx < pvy || pvy < pvx);
        BOOST_TEST(pvx < pvz || pvz < pvx);
        BOOST_TEST(pvy < pvz || pvz < pvy);

        BOOST_TEST(!(pvx < pvy && pvy < pvx));
        BOOST_TEST(!(pvx < pvz && pvz < pvx));
        BOOST_TEST(!(pvy < pvz && pvz < pvy));

        spx.reset();
        spy.reset();
        spz.reset();

        BOOST_TEST(px < py || py < px);
        BOOST_TEST(px < pz || pz < px);
        BOOST_TEST(py < pz || pz < py);

        BOOST_TEST(!(px < py && py < px));
        BOOST_TEST(!(px < pz && pz < px));
        BOOST_TEST(!(py < pz && pz < py));

        BOOST_TEST(!(pvx < pvx));
        BOOST_TEST(!(pvy < pvy));
        BOOST_TEST(!(pvz < pvz));

        BOOST_TEST(pvx < pvy || pvy < pvx);
        BOOST_TEST(pvx < pvz || pvz < pvx);
        BOOST_TEST(pvy < pvz || pvz < pvy);

        BOOST_TEST(!(pvx < pvy && pvy < pvx));
        BOOST_TEST(!(pvx < pvz && pvz < pvx));
        BOOST_TEST(!(pvy < pvz && pvz < pvy));
    }

    {
        mars_boost::shared_ptr<Z> spz(new Z);
        mars_boost::shared_ptr<X> spx(spz);

        mars_boost::weak_ptr<Z> pz(spz);
        mars_boost::weak_ptr<X> px(spx);
        mars_boost::weak_ptr<Y> py(spz);

        BOOST_TEST(!(px < px));
        BOOST_TEST(!(py < py));

        BOOST_TEST(!(px < py || py < px));
        BOOST_TEST(!(px < pz || pz < px));
        BOOST_TEST(!(py < pz || pz < py));

        mars_boost::weak_ptr<void> pvx(px);
        mars_boost::weak_ptr<void> pvy(py);
        mars_boost::weak_ptr<void> pvz(pz);

        BOOST_TEST(!(pvx < pvy || pvy < pvx));
        BOOST_TEST(!(pvx < pvz || pvz < pvx));
        BOOST_TEST(!(pvy < pvz || pvz < pvy));

        spx.reset();
        spz.reset();

        BOOST_TEST(!(px < px));
        BOOST_TEST(!(py < py));

        BOOST_TEST(!(px < py || py < px));
        BOOST_TEST(!(px < pz || pz < px));
        BOOST_TEST(!(py < pz || pz < py));

        BOOST_TEST(!(pvx < pvy || pvy < pvx));
        BOOST_TEST(!(pvx < pvz || pvz < pvx));
        BOOST_TEST(!(pvy < pvz || pvz < pvy));
    }
}

} // namespace n_comparison

namespace n_lock
{

void test()
{
}

} // namespace n_lock

namespace n_map
{

void test()
{
    std::vector< mars_boost::shared_ptr<int> > vi;

    {
        mars_boost::shared_ptr<int> pi1(new int);
        mars_boost::shared_ptr<int> pi2(new int);
        mars_boost::shared_ptr<int> pi3(new int);

        vi.push_back(pi1);
        vi.push_back(pi1);
        vi.push_back(pi1);
        vi.push_back(pi2);
        vi.push_back(pi1);
        vi.push_back(pi2);
        vi.push_back(pi1);
        vi.push_back(pi3);
        vi.push_back(pi3);
        vi.push_back(pi2);
        vi.push_back(pi1);
    }

    std::vector< mars_boost::shared_ptr<X> > vx;

    {
        mars_boost::shared_ptr<X> px1(new X);
        mars_boost::shared_ptr<X> px2(new X);
        mars_boost::shared_ptr<X> px3(new X);

        vx.push_back(px2);
        vx.push_back(px2);
        vx.push_back(px1);
        vx.push_back(px2);
        vx.push_back(px1);
        vx.push_back(px1);
        vx.push_back(px1);
        vx.push_back(px2);
        vx.push_back(px1);
        vx.push_back(px3);
        vx.push_back(px2);
    }

    std::map< mars_boost::weak_ptr<void>, long > m;

    {
        for(std::vector< mars_boost::shared_ptr<int> >::iterator i = vi.begin(); i != vi.end(); ++i)
        {
            ++m[*i];
        }
    }

    {
        for(std::vector< mars_boost::shared_ptr<X> >::iterator i = vx.begin(); i != vx.end(); ++i)
        {
            ++m[*i];
        }
    }

    {
        for(std::map< mars_boost::weak_ptr<void>, long >::iterator i = m.begin(); i != m.end(); ++i)
        {
            BOOST_TEST(i->first.use_count() == i->second);
        }
    }
}

} // namespace n_map

int main()
{
    n_element_type::test();
    n_constructors::test();
    n_assignment::test();
    n_reset::test();
    n_use_count::test();
    n_swap::test();
    n_comparison::test();
    n_lock::test();

    n_map::test();

    return mars_boost::report_errors();
}

class incomplete
{
};

mars_boost::shared_ptr<incomplete> create_incomplete()
{
    mars_boost::shared_ptr<incomplete> px(new incomplete);
    return px;
}
