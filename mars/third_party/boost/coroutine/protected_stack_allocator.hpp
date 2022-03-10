
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <third_party/boost/config.hpp>

#if defined(BOOST_WINDOWS)
# include <third_party/boost/coroutine/windows/protected_stack_allocator.hpp>
#else
# include <third_party/boost/coroutine/posix/protected_stack_allocator.hpp>
#endif
