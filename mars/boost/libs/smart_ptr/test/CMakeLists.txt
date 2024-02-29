# Copyright 2018, 2019 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

include(BoostTestJamfile OPTIONAL RESULT_VARIABLE HAVE_BOOST_TEST)

if(HAVE_BOOST_TEST)

# for lw_thread_test.cpp
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

boost_test_jamfile(FILE Jamfile LINK_LIBRARIES Boost::smart_ptr Boost::core Boost::align Boost::atomic Boost::container_hash Boost::bind Threads::Threads)

endif()
