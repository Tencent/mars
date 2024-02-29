// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_PACK_INSERT_TAGGED_HPP
#define BOOST_PARAMETER_AUX_PACK_INSERT_TAGGED_HPP

#include <boost/parameter/aux_/set.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

    // Inserts Tagged::key_type into the UserArgs set.
    // Extra indirection to lazily evaluate Tagged::key_type.
    template <typename UsedArgs, typename Tagged>
    struct insert_tagged
      : ::mars_boost::parameter::aux::insert_<UsedArgs,typename Tagged::key_type>
    {
    };
}}} // namespace mars_boost::parameter::aux

#endif  // include guard

