// Boost.Signals2 library

// Copyright Frank Mori Hess 2009.
//
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS2_SLOT_HPP
#define BOOST_SIGNALS2_SLOT_HPP

#include <third_party/boost/bind.hpp>
#include <third_party/boost/config.hpp>
#include <third_party/boost/function.hpp>
#include <third_party/boost/mpl/identity.hpp>
#include <third_party/boost/ref.hpp>
#include <third_party/boost/signals2/detail/signals_common.hpp>
#include <third_party/boost/signals2/detail/signals_common_macros.hpp>
#include <third_party/boost/signals2/detail/tracked_objects_visitor.hpp>
#include <third_party/boost/signals2/slot_base.hpp>
#include <third_party/boost/visit_each.hpp>
#include <third_party/boost/weak_ptr.hpp>

#ifdef BOOST_NO_CXX11_VARIADIC_TEMPLATES
#include <third_party/boost/signals2/preprocessed_slot.hpp>
#else
#include <third_party/boost/signals2/variadic_slot.hpp>
#endif

#endif // BOOST_SIGNALS2_SLOT_HPP
