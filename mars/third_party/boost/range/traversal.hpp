// Boost.Range library
//
//  Copyright Neil Groves 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef BOOST_RANGE_TRAVERSAL_HPP
#define BOOST_RANGE_TRAVERSAL_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <third_party/boost/range/config.hpp>
#include <third_party/boost/range/iterator.hpp>
#include <third_party/boost/iterator/iterator_traits.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    template<typename SinglePassRange>
    struct range_traversal
        : iterator_traversal<typename range_iterator<SinglePassRange>::type>
    {
    };
}

#endif
