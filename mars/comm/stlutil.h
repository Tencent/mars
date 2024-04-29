//
// Created by gjt on 4/29/24.
//

#ifndef MARS_STLUTIL_H
#define MARS_STLUTIL_H

#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace stlutil {
template <typename C, typename T>
bool Exist(const C& c, const T& v) {
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}
}


#endif  // MARS_STLUTIL_H
