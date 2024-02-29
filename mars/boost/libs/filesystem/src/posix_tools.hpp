//  posix_tools.hpp  -------------------------------------------------------------------//

//  Copyright 2021 Andrey Semashev

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home page at http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------//

#ifndef BOOST_FILESYSTEM_SRC_POSIX_TOOLS_HPP_
#define BOOST_FILESYSTEM_SRC_POSIX_TOOLS_HPP_

#include "platform_config.hpp"
#include <cerrno>
#include <boost/filesystem/config.hpp>
#ifdef BOOST_HAS_UNISTD_H
#include <unistd.h>
#endif

#include <boost/filesystem/detail/header.hpp> // must be the last #include

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace filesystem {
namespace detail {

//! Platform-specific parameters for directory iterator construction
struct directory_iterator_params
{
#if defined(BOOST_FILESYSTEM_HAS_POSIX_AT_APIS)
    //! File descriptor of the base directory relative to which to interpret relative paths
    int basedir_fd;
#endif
#if defined(BOOST_FILESYSTEM_HAS_FDOPENDIR_NOFOLLOW)
    //! File descriptor of the directory over which the iterator iterates
    int iterator_fd;
#endif
};

/*!
 * Closes a file descriptor and returns the result, similar to close(2). Unlike close(2), guarantees that the file descriptor is closed even if EINTR error happens.
 *
 * Some systems don't close the file descriptor in case if the thread is interrupted by a signal and close(2) returns EINTR.
 * Other (most) systems do close the file descriptor even when when close(2) returns EINTR, and attempting to close it
 * again could close a different file descriptor that was opened by a different thread. This function hides this difference in behavior.
 *
 * Future POSIX standards will likely fix this by introducing posix_close (see https://www.austingroupbugs.net/view.php?id=529)
 * and prohibiting returning EINTR from close(2), but we still have to support older systems where this new behavior is not available and close(2)
 * behaves differently between systems.
 */
inline int close_fd(int fd)
{
#if defined(hpux) || defined(_hpux) || defined(__hpux)
    int res;
    while (true)
    {
        res = ::close(fd);
        if (BOOST_UNLIKELY(res < 0))
        {
            int err = errno;
            if (err == EINTR)
                continue;
        }

        break;
    }

    return res;
#else
    return ::close(fd);
#endif
}

} // namespace detail
} // namespace filesystem
} // namespace mars_boost

#include <boost/filesystem/detail/footer.hpp>

#endif // BOOST_FILESYSTEM_SRC_POSIX_TOOLS_HPP_
