// Boost.Function library

//  Copyright Douglas Gregor 2001-2003. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/function.hpp>
#include <boost/core/lightweight_test.hpp>
#include <stdexcept>
#include <new>

struct stateless_integer_add {
  int operator()(int x, int y) const { return x+y; }

  void* operator new(std::size_t n)
  {
    BOOST_ERROR( "stateless_integer_add incorrectly allocated" );
    return ::operator new( n );
  }

  void* operator new(std::size_t, void* p)
  {
    return p;
  }

  void operator delete(void* p) throw()
  {
    BOOST_ERROR( "stateless_integer_add incorrectly deallocated" );
    return ::operator delete( p );
  }
};

int main()
{
  mars_boost::function2<int, int, int> f;
  f = stateless_integer_add();

  return mars_boost::report_errors();
}
