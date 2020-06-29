cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

include_directories(openssl/include)

add_subdirectory(comm comm)
add_subdirectory(boost boost)
add_subdirectory(app app)
add_subdirectory(baseevent baseevent)
add_subdirectory(log xlog)
add_subdirectory(sdt sdt)
add_subdirectory(stn stn)


project (mars)

include(comm/CMakeUtils.txt)

include_directories(.)
include_directories(..)

set(SELF_LIBS_OUT ${CMAKE_SYSTEM_NAME}.out)

if(ANDROID)

    find_library(log-lib log)
    find_library(z-lib z)

    link_directories(app baseevent log sdt stn comm boost)

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
                            ${log-lib}
                            ${z-lib}
                            )
 
    set(SELF_LIB_NAME marsstn)

    file(GLOB SELF_SRC_FILES libraries/mars_android_sdk/jni/*.cc)

    add_library(${SELF_LIB_NAME} SHARED ${SELF_SRC_FILES})

    install(TARGETS ${SELF_LIB_NAME} LIBRARY DESTINATION ${SELF_LIBS_OUT} ARCHIVE DESTINATION ${SELF_LIBS_OUT})

    link_directories(${SELF_LIBS_OUT})

if(NOT MSVC)
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
endif()
    find_library(CRYPT_LIB crypto PATHS openssl/openssl_lib_android/${ANDROID_ABI} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    find_library(SSL_LIB ssl PATHS openssl/openssl_lib_android/${ANDROID_ABI} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

    target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections"
                        ${log-lib}
                        stn
                        sdt
                        app
                        baseevent
                        comm
                        mars-boost
                        marsxlog
                        ${SSL_LIB}
                        ${CRYPT_LIB})

elseif(APPLE)

endif()





