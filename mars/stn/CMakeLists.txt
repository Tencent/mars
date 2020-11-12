cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")


project (stn)

include(../comm/CMakeUtils.txt)
include(../comm/CMakeExtraFlags.txt)

include_directories(.)
include_directories(src)
include_directories(..)
include_directories(../..)
include_directories(../comm)
include_directories(../../..)
include_directories(../openssl/include)

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} src/*.cc src/*.h)
source_group(src FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.cc *.h)
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

if(MIN_VERSION)
    list(FILTER SELF_SRC_FILES EXCLUDE REGEX "src/.*longlink.*.cc$")
    list(REMOVE_ITEM SELF_SRC_FILES "src/signalling_keeper.cc" "src/timing_sync.cc" "src/net_check_logic.cc" "src/smart_heartbeat.cc")
endif()

if(MSVC)
    add_definitions(/FI"../../comm/projdef.h")
    include_directories(../comm/windows)

elseif(ANDROID)
    if(NOT CPP_CALL_BACK)
        file(GLOB SELF_ANDROID_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} jni/*.cc)
    endif()
        
    list(APPEND SELF_SRC_FILES ${SELF_ANDROID_SRC_FILES})
endif()

BuildWithUnitTest("${PROJECT_NAME}" "${SELF_SRC_FILES}")
