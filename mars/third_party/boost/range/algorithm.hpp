///////////////////////////////////////////////////////////////////////////////
/// \file algorithm.hpp
///   Includes the range-based versions of the algorithms in the
///   C++ standard header file <algorithm>
//
/////////////////////////////////////////////////////////////////////////////

// Copyright 2009 Neil Groves.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Acknowledgements:
// This code uses combinations of ideas, techniques and code snippets
// from: Thorsten Ottosen, Eric Niebler, Jeremy Siek,
// and Vladimir Prus'
//
// The original mutating algorithms that served as the first version
// were originally written by Vladimir Prus'
// <ghost@cs.msu.su> code from Boost Wiki

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BOOST_RANGE_ALGORITHM_HPP_INCLUDED_01012009
#define BOOST_RANGE_ALGORITHM_HPP_INCLUDED_01012009

#include <third_party/boost/range/concepts.hpp>
#include <third_party/boost/range/iterator_range.hpp>
#include <third_party/boost/range/difference_type.hpp>
#include <third_party/boost/range/detail/range_return.hpp>
#include <third_party/boost/iterator/iterator_traits.hpp>
#include <third_party/boost/next_prior.hpp>
#include <algorithm>

// Non-mutating algorithms
#include <third_party/boost/range/algorithm/adjacent_find.hpp>
#include <third_party/boost/range/algorithm/count.hpp>
#include <third_party/boost/range/algorithm/count_if.hpp>
#include <third_party/boost/range/algorithm/equal.hpp>
#include <third_party/boost/range/algorithm/for_each.hpp>
#include <third_party/boost/range/algorithm/find.hpp>
#include <third_party/boost/range/algorithm/find_end.hpp>
#include <third_party/boost/range/algorithm/find_first_of.hpp>
#include <third_party/boost/range/algorithm/find_if.hpp>
#include <third_party/boost/range/algorithm/lexicographical_compare.hpp>
#include <third_party/boost/range/algorithm/mismatch.hpp>
#include <third_party/boost/range/algorithm/search.hpp>
#include <third_party/boost/range/algorithm/search_n.hpp>

// Mutating algorithms
#include <third_party/boost/range/algorithm/copy.hpp>
#include <third_party/boost/range/algorithm/copy_backward.hpp>
#include <third_party/boost/range/algorithm/fill.hpp>
#include <third_party/boost/range/algorithm/fill_n.hpp>
#include <third_party/boost/range/algorithm/generate.hpp>
#include <third_party/boost/range/algorithm/inplace_merge.hpp>
#include <third_party/boost/range/algorithm/merge.hpp>
#include <third_party/boost/range/algorithm/nth_element.hpp>
#include <third_party/boost/range/algorithm/partial_sort.hpp>
#include <third_party/boost/range/algorithm/partial_sort_copy.hpp>
#include <third_party/boost/range/algorithm/partition.hpp>
#include <third_party/boost/range/algorithm/random_shuffle.hpp>
#include <third_party/boost/range/algorithm/remove.hpp>
#include <third_party/boost/range/algorithm/remove_copy.hpp>
#include <third_party/boost/range/algorithm/remove_copy_if.hpp>
#include <third_party/boost/range/algorithm/remove_if.hpp>
#include <third_party/boost/range/algorithm/replace.hpp>
#include <third_party/boost/range/algorithm/replace_copy.hpp>
#include <third_party/boost/range/algorithm/replace_copy_if.hpp>
#include <third_party/boost/range/algorithm/replace_if.hpp>
#include <third_party/boost/range/algorithm/reverse.hpp>
#include <third_party/boost/range/algorithm/reverse_copy.hpp>
#include <third_party/boost/range/algorithm/rotate.hpp>
#include <third_party/boost/range/algorithm/rotate_copy.hpp>
#include <third_party/boost/range/algorithm/sort.hpp>
#include <third_party/boost/range/algorithm/stable_partition.hpp>
#include <third_party/boost/range/algorithm/stable_sort.hpp>
#include <third_party/boost/range/algorithm/transform.hpp>
#include <third_party/boost/range/algorithm/unique.hpp>
#include <third_party/boost/range/algorithm/unique_copy.hpp>

// Binary search
#include <third_party/boost/range/algorithm/binary_search.hpp>
#include <third_party/boost/range/algorithm/equal_range.hpp>
#include <third_party/boost/range/algorithm/lower_bound.hpp>
#include <third_party/boost/range/algorithm/upper_bound.hpp>

// Set operations of sorted ranges
#include <third_party/boost/range/algorithm/set_algorithm.hpp>

// Heap operations
#include <third_party/boost/range/algorithm/heap_algorithm.hpp>

// Minimum and Maximum
#include <third_party/boost/range/algorithm/max_element.hpp>
#include <third_party/boost/range/algorithm/min_element.hpp>

// Permutations
#include <third_party/boost/range/algorithm/permutation.hpp>

#endif // include guard

