#include "shuffle.h"
#include <iterator>
#include <utility>
#include "mars/openssl/include/openssl/rand.h"

namespace mars{
namespace comm{

size_t shuffle_rand(size_t v){
    size_t seed = 0;
    RAND_bytes(reinterpret_cast<unsigned char *>(&seed), sizeof(seed));
    return seed % v;
}

template <class RandomAccessIterator>
void random_shuffle (RandomAccessIterator first, RandomAccessIterator last){
    typename std::iterator_traits<RandomAccessIterator>::difference_type i, n;
    n = (last-first);
    for (i=n-1; i>0; --i) {
        std::swap(first[i], first[shuffle_rand(i+1)]);
    }
}

}}