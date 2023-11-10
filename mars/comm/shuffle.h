#pragma once
#include <cstdlib>
#include <iterator>
#include <utility>

#include "openssl/rand.h"

namespace mars {
namespace comm {
template <class RandomAccessIterator>
void random_shuffle(RandomAccessIterator first, RandomAccessIterator last) {
    typename std::iterator_traits<RandomAccessIterator>::difference_type i, n;
    n = (last - first);
    size_t seed = 0;
    for (i = n - 1; i > 0; --i) {
        RAND_bytes(reinterpret_cast<unsigned char*>(&seed), sizeof(seed));
        std::swap(first[i], first[seed % (i + 1)]);
    }
}

}  // namespace comm
}  // namespace mars
