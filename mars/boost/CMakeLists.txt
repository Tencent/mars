cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

project (mars-boost)

include(../comm/CMakeUtils.txt)

include_directories(.)
include_directories(..)

add_definitions(-DBOOST_NO_EXCEPTIONS)

set(SELF_SRC_FILES
        libs/atomic/src/lock_pool.cpp
        libs/filesystem/src/codecvt_error_category.cpp
        libs/filesystem/src/directory.cpp
        libs/filesystem/src/operations.cpp
        libs/filesystem/src/exception.cpp
        libs/filesystem/src/path.cpp
        libs/filesystem/src/path_traits.cpp
        libs/filesystem/src/portability.cpp
        libs/filesystem/src/unique_path.cpp
        libs/filesystem/src/utf8_codecvt_facet.cpp
        libs/filesystem/src/windows_file_codecvt.cpp
        libs/iostreams/src/file_descriptor.cpp
        libs/iostreams/src/mapped_file.cpp
        libs/smart_ptr/extras/src/sp_collector.cpp
        libs/smart_ptr/extras/src/sp_debug_hooks.cpp
        libs/system/src/error_code.cpp
        libs/thread/src/future.cpp)
        
 
if(MSVC)
    include_directories(../comm/windows)
    list(APPEND SELF_SRC_FILES
            libs/thread/src/win32/thread.cpp
            libs/thread/src/win32/tss_dll.cpp
            libs/thread/src/win32/tss_pe.cpp)
    
endif()


add_library(${PROJECT_NAME} STATIC ${SELF_SRC_FILES})

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${CMAKE_SYSTEM_NAME}.out)
    
    

