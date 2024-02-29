# Copyright 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

include(BoostTest OPTIONAL RESULT_VARIABLE HAVE_BOOST_TEST)

if(NOT HAVE_BOOST_TEST)
  return()
endif()

set(BOOST_TEST_LINK_LIBRARIES Boost::signals2 Boost::included_unit_test_framework)

boost_test(TYPE run SOURCES connection_test.cpp)
boost_test(TYPE run SOURCES dead_slot_test.cpp)
boost_test(TYPE run SOURCES deadlock_regression_test.cpp)
boost_test(TYPE run SOURCES deconstruct_test.cpp)
boost_test(TYPE run SOURCES deletion_test.cpp)
boost_test(TYPE run SOURCES ordering_test.cpp)
boost_test(TYPE run SOURCES regression_test.cpp)
boost_test(TYPE run SOURCES shared_connection_block_test.cpp)
boost_test(TYPE run SOURCES signal_n_test.cpp)
boost_test(TYPE run SOURCES signal_test.cpp)
boost_test(TYPE run SOURCES signal_type_test.cpp)
boost_test(TYPE run SOURCES slot_compile_test.cpp)
boost_test(TYPE run SOURCES trackable_test.cpp)
boost_test(TYPE run SOURCES track_test.cpp)

list(APPEND BOOST_TEST_LINK_LIBRARIES Boost::thread)

boost_test(TYPE run SOURCES mutex_test.cpp)
boost_test(TYPE run SOURCES threading_models_test.cpp)
