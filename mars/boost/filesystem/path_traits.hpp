//  filesystem path_traits.hpp  --------------------------------------------------------//

//  Copyright Beman Dawes 2009
//  Copyright Andrey Semashev 2022

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#ifndef BOOST_FILESYSTEM_PATH_TRAITS_HPP
#define BOOST_FILESYSTEM_PATH_TRAITS_HPP

#include <boost/filesystem/config.hpp>

#if !defined(BOOST_FILESYSTEM_DEPRECATED) && !defined(BOOST_FILESYSTEM_ALLOW_DEPRECATED)
#include <boost/config/header_deprecated.hpp>
BOOST_HEADER_DEPRECATED("your own implementation")
#endif

#if !defined(BOOST_FILESYSTEM_NO_DEPRECATED)

#include <boost/filesystem/detail/header.hpp> // must be the last #include
#include <boost/filesystem/detail/path_traits.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace filesystem {

namespace path_traits = mars_boost::filesystem::detail::path_traits;

} // namespace filesystem
} // namespace mars_boost

#include <boost/filesystem/detail/footer.hpp>

#endif // !defined(BOOST_FILESYSTEM_NO_DEPRECATED)

#endif // BOOST_FILESYSTEM_PATH_TRAITS_HPP
