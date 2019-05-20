cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

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

    get_filename_component(EXPORT_XLOG_EXP_FILE log/jni/export.exp ABSOLUTE)

    if(ANDROID_ABI STREQUAL "arm64-v8a")
         target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections"
                                ${log-lib}
                                ${z-lib}
                                xlog
                                mars-boost
                                comm)

    else()
        target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections -Wl,--version-script=\"${EXPORT_XLOG_EXP_FILE}\""
                                ${log-lib}
                                ${z-lib}
                                xlog
                                mars-boost
                                comm)
    endif()


    set(SELF_LIB_NAME marsstn)
    get_filename_component(EXPORT_EXP_FILE libraries/mars_android_sdk/jni/export.exp ABSOLUTE)

    file(GLOB SELF_SRC_FILES libraries/mars_android_sdk/jni/*.cc)

    add_library(${SELF_LIB_NAME} SHARED ${SELF_SRC_FILES})


    install(TARGETS ${SELF_LIB_NAME} LIBRARY DESTINATION ${SELF_LIBS_OUT} ARCHIVE DESTINATION ${SELF_LIBS_OUT})

    link_directories(${SELF_LIBS_OUT})

    if(ANDROID_ABI STREQUAL "arm64-v8a")
        target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections"
                            ${log-lib}
                            stn
                            sdt
                            app
                            baseevent
                            comm
                            mars-boost
                            marsxlog)
    else()
        target_link_libraries(${SELF_LIB_NAME} "-Wl,--gc-sections -Wl,--version-script=\"${EXPORT_EXP_FILE}\""
                            ${log-lib}
                            stn
                            sdt
                            app
                            baseevent
                            comm
                            mars-boost
                            marsxlog)
    endif()
elseif(APPLE)

endif()





