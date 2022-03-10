# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef BOOST_PREPROCESSOR_SLOT_SLOT_HPP
# define BOOST_PREPROCESSOR_SLOT_SLOT_HPP
#
# include <third_party/boost/preprocessor/cat.hpp>
# include <third_party/boost/preprocessor/slot/detail/def.hpp>
#
# /* BOOST_PP_ASSIGN_SLOT */
#
# define BOOST_PP_ASSIGN_SLOT(i) BOOST_PP_CAT(BOOST_PP_ASSIGN_SLOT_, i)
#
# define BOOST_PP_ASSIGN_SLOT_1 <third_party/boost/preprocessor/slot/detail/slot1.hpp>
# define BOOST_PP_ASSIGN_SLOT_2 <third_party/boost/preprocessor/slot/detail/slot2.hpp>
# define BOOST_PP_ASSIGN_SLOT_3 <third_party/boost/preprocessor/slot/detail/slot3.hpp>
# define BOOST_PP_ASSIGN_SLOT_4 <third_party/boost/preprocessor/slot/detail/slot4.hpp>
# define BOOST_PP_ASSIGN_SLOT_5 <third_party/boost/preprocessor/slot/detail/slot5.hpp>
#
# /* BOOST_PP_SLOT */
#
# define BOOST_PP_SLOT(i) BOOST_PP_CAT(BOOST_PP_SLOT_, i)()
#
# endif
