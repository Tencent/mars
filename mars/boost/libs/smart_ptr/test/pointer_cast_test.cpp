//
//  pointer_cast_test.cpp - a test for boost/pointer_cast.hpp
//
//  Copyright (c) 2005 Ion Gaztanaga
//  Copyright (c) 2005 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/config.hpp>

#include <boost/pointer_cast.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/get_pointer.hpp>

#include <boost/core/lightweight_test.hpp>

namespace
{

// Let's create these inheritance relationship:
//
//    base   base2
//      |     |
//      derived
//         |
//  derived_derived
//

class base
{
   public:
   virtual ~base(){}
   int filler [5];
};

class base2
{
public:

    virtual ~base2(){}
    int filler [5];
};

class derived
   : public base, public base2 
{
    int filler [5];
};

class derived_derived
   : public derived
{
    int filler [5];
};

// And now some simple check functions

#if !defined( BOOST_NO_RTTI )

template <class BasePtr>
bool check_dynamic_pointer_cast(const BasePtr &ptr)
{
   //Check that dynamic_pointer_cast versus dynamic_cast
   return
   //Correct cast with dynamic_pointer_cast
   mars_boost::get_pointer(mars_boost::dynamic_pointer_cast<derived>(ptr)) ==
      //Correct cast with dynamic_cast
      dynamic_cast<derived*>(mars_boost::get_pointer(ptr)) 
   &&
   //Incorrect cast with dynamic_pointer_cast
   mars_boost::get_pointer(mars_boost::dynamic_pointer_cast<derived_derived>(ptr)) ==
      //Incorrect cast with dynamic_cast
      dynamic_cast<derived_derived*>(mars_boost::get_pointer(ptr));
}

#endif

template <class BasePtr>
bool check_static_pointer_cast(const BasePtr &ptr)
{
   return
   //Cast base -> derived -> base2 using static_pointer_cast
   mars_boost::get_pointer(
            mars_boost::static_pointer_cast<base2>(
               mars_boost::static_pointer_cast<derived>(ptr))) ==
   //Now the same with static_cast
   static_cast<base2*>(static_cast<derived*>(mars_boost::get_pointer(ptr)));
}

template <class BasePtr>
bool check_const_pointer_cast(const BasePtr &ptr)
{
   return   
   //Unconst and const again using const_pointer_cast
   mars_boost::get_pointer(
      mars_boost::const_pointer_cast<const base>
         (mars_boost::const_pointer_cast<base>(ptr))) ==
   //Now the same with const_cast
   const_cast<const base*>(const_cast<base*>(mars_boost::get_pointer(ptr)));
}

template <class BasePtr>
void check_all_casts(const BasePtr &ptr)
{
#if !defined( BOOST_NO_RTTI )
    BOOST_TEST( check_dynamic_pointer_cast( ptr ) );
#endif
    BOOST_TEST( check_static_pointer_cast( ptr ) );
    BOOST_TEST( check_const_pointer_cast( ptr ) );
}

}

int main()
{
    mars_boost::shared_ptr<base> boost_shared(new derived);
    base *plain = boost_shared.get();

    check_all_casts(boost_shared);
    check_all_casts(plain);
    
    return mars_boost::report_errors();
}
