//
//  cpp11_pointer_cast_test.cpp - a test for boost/pointer_cast.hpp with std::shared_ptr and std::unique_ptr
//
//  Copyright (c) 2016 Karolin Varner
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/pointer_cast.hpp>

#include <boost/config.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/get_pointer.hpp>
#include <boost/shared_ptr.hpp>

#include <memory>
#include <utility>
#include <functional>

#if defined( BOOST_NO_CXX11_RVALUE_REFERENCES ) \
 || defined( BOOST_NO_CXX11_HDR_FUNCTIONAL ) \
 || defined( BOOST_NO_CXX11_HDR_UTILITY ) \
 || defined( BOOST_NO_CXX11_LAMBDAS ) \
 || defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

// We expect all the features or none of the features to be
// available, since we should be on C++11
int main() { return 0; }

#else

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
void check_all_copy_casts(const BasePtr &ptr)
{
#if !defined( BOOST_NO_RTTI )
   BOOST_TEST( check_dynamic_pointer_cast( ptr ) );
#endif
   BOOST_TEST( check_static_pointer_cast( ptr ) );
   BOOST_TEST( check_const_pointer_cast( ptr ) );
}


#if !defined( BOOST_NO_RTTI )

template <class BasePtr>
bool check_dynamic_moving_pointer_cast(std::function<BasePtr()> f)
{
   BasePtr smart1 = f(), smart2 = f();
   derived* expect1 = dynamic_cast<derived*>(mars_boost::get_pointer(smart1));
   derived_derived* expect2 = dynamic_cast<derived_derived*>(mars_boost::get_pointer(smart2));
   //Check that dynamic_pointer_cast versus dynamic_cast
   return
   //Correct cast with dynamic_pointer_cast
   mars_boost::get_pointer(mars_boost::dynamic_pointer_cast<derived>( std::move(smart1) )) == expect1
   &&
   //Incorrect cast with dynamic_pointer_cast
   mars_boost::get_pointer(mars_boost::dynamic_pointer_cast<derived_derived>( std::move(smart2) )) == expect2;
}

#endif

template <class BasePtr>
bool check_static_moving_pointer_cast(std::function<BasePtr()> f)
{
   BasePtr smart = f();
   base2 *expect = static_cast<base2*>(static_cast<derived*>(mars_boost::get_pointer(smart)));

   return
   //Cast base -> derived -> base2 using static_pointer_cast
   mars_boost::get_pointer(
            mars_boost::static_pointer_cast<base2>(
               mars_boost::static_pointer_cast<derived>( std::move(smart) ))) ==
   //Now the same with static_cast
   expect;
}

template <class BasePtr>
bool check_const_moving_pointer_cast(std::function<BasePtr()> f)
{
   BasePtr smart = f();
   const base *expect = const_cast<const base*>(const_cast<base*>(mars_boost::get_pointer(smart)));
   return
   //Unconst and const again using const_pointer_cast
   mars_boost::get_pointer(
      mars_boost::const_pointer_cast<const base>
         (mars_boost::const_pointer_cast<base>( std::move(smart) ))) ==
   //Now the same with const_cast
   expect;
}

template <class BasePtr>
void check_all_moving_casts(std::function<BasePtr()> f) {
#if !defined( BOOST_NO_RTTI )
   BOOST_TEST( check_dynamic_moving_pointer_cast( f ) );
#endif
   BOOST_TEST( check_static_moving_pointer_cast( f ) );
   BOOST_TEST( check_const_moving_pointer_cast( f ) );
}

}

int main()
{

   std::shared_ptr<base> std_shared(new derived);
   mars_boost::shared_ptr<base> boost_shared(new derived);
   base *plain = boost_shared.get();

   // plain & mars_boost::shared_ptr moving pointer_cast checks; there
   // is no specific handleing for those types at the moment; this
   // test just makes sure they won't break when std::move() is used
   // in generic code

   check_all_moving_casts<mars_boost::shared_ptr<base>>([&boost_shared]() {
       return boost_shared;
   });

   check_all_moving_casts<base*>([plain]() {
       return plain;
   });

   // std::shared_ptr casts

   check_all_copy_casts(std_shared);
   check_all_moving_casts<std::shared_ptr<base>>([&std_shared]() {
       return std_shared;
   });

   // std::unique_ptr casts

   check_all_moving_casts<std::unique_ptr<base>>([]() {
       return std::unique_ptr<base>(new derived);
   });

    return mars_boost::report_errors();
}
#endif
