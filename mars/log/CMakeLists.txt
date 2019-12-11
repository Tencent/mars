cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

project (xlog)


set(SELF_LIBS_OUT ${CMAKE_SYSTEM_NAME}.out)


include(../comm/CMakeUtils.txt)
include(../comm/CMakeExtraFlags.txt)

include_directories(.)
include_directories(src)
include_directories(..)
include_directories(../..)
include_directories(../comm)
include_directories(../crypt)
include_directories(../../..)
include_directories(../crypt/micro-ecc-master)


file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} src/*.cc src/*.h)
source_group(src FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})


file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} crypt/*.cc crypt/*.h)
source_group(crypt FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} crypt/micro-ecc-master/*.c crypt/micro-ecc-master/*.h)
source_group(crypt\\micro-ecc-master FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})



if(MSVC)
    add_definitions(/FI"../../comm/projdef.h")

    include_directories(../comm/windows)
    include_directories(../comm/windows/zlib)
    
elseif(ANDROID)
    file(GLOB SELF_ANDROID_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR}
            ../comm/xlogger/xloggerbase.c
            jni/*.cc
            ../mk_template/JNI_OnLoad.cpp)
        
    list(APPEND SELF_SRC_FILES ${SELF_ANDROID_SRC_FILES})

    get_filename_component(EXPORT_EXP_FILE jni/export.exp ABSOLUTE)
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${EXPORT_EXP_FILE}")


endif()

# add_library(${PROJECT_NAME} STATIC ${SELF_SRC_FILES})
# install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${SELF_LIBS_OUT})

BuildWithUnitTest("${PROJECT_NAME}" "${SELF_SRC_FILES}")


    
    
