cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

include_directories(openssl/include)

add_subdirectory(comm comm)
add_subdirectory(boot boot)
add_subdirectory(boost boost)
add_subdirectory(app app)
add_subdirectory(baseevent baseevent)
add_subdirectory(xlog xlog)
add_subdirectory(sdt sdt)
add_subdirectory(stn stn)

# for zstd
option(ZSTD_BUILD_STATIC "BUILD STATIC LIBRARIES" ON)
option(ZSTD_BUILD_SHARED "BUILD SHARED LIBRARIES" OFF)
set(ZSTD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zstd")
set(LIBRARY_DIR ${ZSTD_SOURCE_DIR}/lib)
include(GNUInstallDirs)
add_subdirectory(zstd/build/cmake/lib zstd)

project (mars)

include(comm/CMakeUtils.txt)

include_directories(.)
include_directories(..)

set(SELF_LIBS_OUT ${CMAKE_SYSTEM_NAME}.out)

if(ANDROID)

    if(NATIVE_CALLBACK)
        message("common native callback")
        add_definitions(-DNATIVE_CALLBACK)
    endif()

    find_library(log-lib log)
    find_library(z-lib z)

    link_directories(app baseevent xlog sdt stn comm boost zstd)

    # marsxlog
    set(SELF_LIB_NAME marsxlog)
    file(GLOB SELF_SRC_FILES libraries/mars_android_sdk/jni/JNI_OnLoad.cc
            libraries/mars_xlog_sdk/jni/import.cc)
    add_library(${SELF_LIB_NAME} SHARED ${SELF_SRC_FILES})
    install(TARGETS ${SELF_LIB_NAME} LIBRARY DESTINATION ${SELF_LIBS_OUT} ARCHIVE DESTINATION ${SELF_LIBS_OUT})
    get_filename_component(EXPORT_XLOG_EXP_FILE libraries/mars_android_sdk/jni/export.exp ABSOLUTE)
    set(SELF_XLOG_LINKER_FLAG "-Wl,--gc-sections -Wl,--version-script='${EXPORT_XLOG_EXP_FILE}'") 
    if(ANDROID_ABI STREQUAL "x86_64" OR ANDROID_ABI STREQUAL "x86")
        set(SELF_XLOG_LINKER_FLAG "-Wl,--gc-sections -Wl,--version-script='${EXPORT_XLOG_EXP_FILE}' -Wl,--no-warn-shared-textrel") 
    endif()
    target_link_libraries(${SELF_LIB_NAME} "${SELF_XLOG_LINKER_FLAG}"
                            xlog
                            mars-boost
                            comm
                            libzstd_static
                            ${log-lib}
                            ${z-lib}
                            )
 
    # marsstn
    set(SELF_LIB_NAME marsstn)
    file(GLOB SELF_SRC_FILES libraries/mars_android_sdk/jni/*.cc)
    add_library(${SELF_LIB_NAME} SHARED ${SELF_SRC_FILES})
    install(TARGETS ${SELF_LIB_NAME} LIBRARY DESTINATION ${SELF_LIBS_OUT} ARCHIVE DESTINATION ${SELF_LIBS_OUT})
    link_directories(${SELF_LIBS_OUT})
    find_library(CRYPT_LIB crypto PATHS openssl/openssl_lib_android/${ANDROID_ABI} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    find_library(SSL_LIB ssl PATHS openssl/openssl_lib_android/${ANDROID_ABI} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections"
                        ${log-lib}
                        stn
                        sdt
                        app
                        baseevent
                        comm
                        boot
                        mars-boost
                        marsxlog
                        libzstd_static
                        ${SSL_LIB}
                        ${CRYPT_LIB})

elseif(OHOS)
message("start build ohos")
add_definitions(-DOHOS)
if(NATIVE_CALLBACK)
    message("common native callback")
    add_definitions(-DNATIVE_CALLBACK)
endif()

link_directories(app baseevent xlog sdt stn comm boost zstd)

# marsxlog
set(SELF_LIB_NAME marsxlog)
file(GLOB SELF_SRC_FILES ohoslibraries/mars_xlog_sdk/import.cc)
add_library(${SELF_LIB_NAME} SHARED ${SELF_SRC_FILES})
install(TARGETS ${SELF_LIB_NAME} LIBRARY DESTINATION ${SELF_LIBS_OUT} ARCHIVE DESTINATION ${SELF_LIBS_OUT})
get_filename_component(EXPORT_XLOG_EXP_FILE ohoslibraries/export.exp ABSOLUTE)
set(SELF_XLOG_LINKER_FLAG "-Wl,--gc-sections -Wl,--version-script='${EXPORT_XLOG_EXP_FILE}'") 
target_link_libraries(${SELF_LIB_NAME} "${SELF_XLOG_LINKER_FLAG}"
                        xlog
                        mars-boost
                        comm
                        libzstd_static
                        libace_napi.z.so
                        libhilog_ndk.z.so
                        libz.so
                        )
endif()
