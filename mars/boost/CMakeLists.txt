cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

project (mars-boost)

include(../comm/CMakeUtils.txt)

include_directories(.)
include_directories(..)

add_definitions(-DBOOST_NO_EXCEPTIONS)

set(SELF_SRC_FILES
        libs/atomic/src/lockpool.cpp
        libs/date_time/src/gregorian/date_generators.cpp
        libs/date_time/src/gregorian/gregorian_types.cpp
        libs/date_time/src/gregorian/greg_month.cpp
        libs/date_time/src/gregorian/greg_weekday.cpp
        libs/date_time/src/posix_time/posix_time_types.cpp
        libs/exception/src/clone_current_exception_non_intrusive.cpp
        libs/filesystem/src/codecvt_error_category.cpp
        libs/filesystem/src/operations.cpp
        libs/filesystem/src/path.cpp
        libs/filesystem/src/path_traits.cpp
        libs/filesystem/src/portability.cpp
        libs/filesystem/src/unique_path.cpp
        libs/filesystem/src/utf8_codecvt_facet.cpp
        libs/filesystem/src/windows_file_codecvt.cpp
        libs/iostreams/src/file_descriptor.cpp
        libs/iostreams/src/mapped_file.cpp
        libs/smart_ptr/src/sp_collector.cpp
        libs/smart_ptr/src/sp_debug_hooks.cpp
        libs/system/src/error_code.cpp
        libs/thread/src/future.cpp)
        
 
if(MSVC)
    include_directories(../comm/windows)
    list(APPEND SELF_SRC_FILES
            libs/thread/src/win32/thread.cpp
            libs/thread/src/win32/tss_dll.cpp
            libs/thread/src/win32/tss_pe.cpp)
    
endif()

if(ANDROID)
    file(GLOB SELF_ANDROID_SRC_FILE
            libs/coroutine/src/*.cpp
            libs/coroutine/src/detail/*.cpp
            libs/coroutine/src/posix/*.cpp
            libs/context/src/*.cpp
            libs/context/src/posix/*.cpp)

    list(APPEND SELF_SRC_FILES ${SELF_ANDROID_SRC_FILE})
    enable_language(ASM)
    
    if(ANDROID_ABI MATCHES "^armeabi(-v7a)?$")
        list(APPEND SELF_SRC_FILES
                libs/context/src/asm/jump_arm_aapcs_elf_gas.S
                libs/context/src/asm/make_arm_aapcs_elf_gas.S)
    elseif(ANDROID_ABI STREQUAL arm64-v8a)
        list(APPEND SELF_SRC_FILES
                libs/context/src/asm/jump_arm64_aapcs_elf_gas.S
                libs/context/src/asm/make_arm64_aapcs_elf_gas.S)
    elseif(ANDROID_ABI STREQUAL x86)
        list(APPEND SELF_SRC_FILES
                libs/context/src/asm/jump_i386_sysv_elf_gas.S
                libs/context/src/asm/make_i386_sysv_elf_gas.S)
    elseif(ANDROID_ABI STREQUAL x86_64)
        list(APPEND SELF_SRC_FILES
                libs/context/src/asm/jump_x86_64_sysv_elf_gas.S
                libs/context/src/asm/make_x86_64_sysv_elf_gas.S)
    endif()

endif()


add_library(${PROJECT_NAME} STATIC ${SELF_SRC_FILES})

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${CMAKE_SYSTEM_NAME}.out)
    
    

