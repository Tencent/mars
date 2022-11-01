#pragma once
#include <cstdlib>

namespace mars{
namespace comm{
size_t shuffle_rand(size_t v);

template <class RandomAccessIterator>
void random_shuffle (RandomAccessIterator first, RandomAccessIterator last);

}}
