//
// Created by astro zhou on 2020/11/13.
//

#ifndef MMNET_ANY_H
#define MMNET_ANY_H

#ifdef _MARS_CPP17_
#include <any>
#else
#include "boost/any.hpp"
#endif

namespace mars {
#ifdef _MARS_CPP17_
    using std::any;
    using std::any_cast;
#else
    using boost::any;
    using boost::any_cast;
#endif
}

#endif //MMNET_ANY_H
