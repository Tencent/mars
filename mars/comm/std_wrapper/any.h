//
// Created by astro zhou on 2020/11/13.
//

#ifndef MMNET_ANY_H
#define MMNET_ANY_H

#if __cplusplus >= 201703L
#include <any>
#else
#include "boost/any.hpp"
#endif

namespace mars {
#if __cplusplus >= 201703L
    using std::any;
    using std::any_cast;
#else
    using boost::any;
    using boost::any_cast;
#endif
}

#endif //MMNET_ANY_H
