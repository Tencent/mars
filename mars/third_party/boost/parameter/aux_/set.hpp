// Copyright Daniel Wallin 2006. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_SET_060912_HPP
# define BOOST_PARAMETER_SET_060912_HPP

# include <third_party/boost/detail/workaround.hpp>

# if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
#  include <third_party/boost/mpl/insert.hpp>
#  include <third_party/boost/mpl/set/set0.hpp>
#  include <third_party/boost/mpl/has_key.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

typedef mpl::set0<> set0;

template <class Set, class K>
struct insert_
{
    typedef typename mpl::insert<Set, K>::type type;
};

template <class Set, class K>
struct has_key_
{
    typedef typename mpl::has_key<Set, K>::type type;
};

}}} // namespace mars_boost::parameter::aux

# else

#  include <third_party/boost/mpl/list.hpp>
#  include <third_party/boost/mpl/end.hpp>
#  include <third_party/boost/mpl/find.hpp>
#  include <third_party/boost/mpl/not.hpp>
#  include <third_party/boost/mpl/push_front.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace parameter { namespace aux {

typedef mpl::list0<> set0;

template <class Set, class K>
struct insert_
{
    typedef typename mpl::push_front<Set, K>::type type;
};

template <class Set, class K>
struct has_key_
{
    typedef typename mpl::find<Set, K>::type iter;
    typedef mpl::not_<
        is_same<iter, typename mpl::end<Set>::type> 
    > type;
};

}}} // namespace mars_boost::parameter::aux

# endif


#endif // BOOST_PARAMETER_SET_060912_HPP

