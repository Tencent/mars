# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.16)

project(boost_context VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${boost_context_SOURCE_DIR}/cmake)

# Build features

## Binary format

if(WIN32)
  set(_default_binfmt pe)
elseif(APPLE)
  set(_default_binfmt mach-o)
else()
  set(_default_binfmt elf)
endif()

set(BOOST_CONTEXT_BINARY_FORMAT "${_default_binfmt}" CACHE STRING "Boost.Context binary format (elf, mach-o, pe, xcoff)")
set_property(CACHE BOOST_CONTEXT_BINARY_FORMAT PROPERTY STRINGS elf mach-o pe xcoff)

unset(_default_binfmt)

## ABI

math(EXPR _bits "${CMAKE_SIZEOF_VOID_P}*8")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  set(_default_abi aapcs)
elseif(WIN32)
  set(_default_abi ms)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
  if(_bits EQUAL 32)
    set(_default_abi o32)
  else()
    set(_default_abi n64)
  endif()
else()
  set(_default_abi sysv)
endif()

set(BOOST_CONTEXT_ABI "${_default_abi}" CACHE STRING "Boost.Context ABI (aapcs, eabi, ms, n32, n64, o32, o64, sysv, x32)")
set_property(CACHE BOOST_CONTEXT_ABI PROPERTY STRINGS aapcs eabi ms n32 n64 o32 o64 sysv x32)

unset(_default_abi)

## Arch-and-model

set(_all_archs arm arm64 loongarch64 mips32 mips64 ppc32 ppc64 riscv64 s390x i386 x86_64 combined)

# Try at start to auto determine arch from CMake.
if(CMAKE_SYSTEM_PROCESSOR IN_LIST _all_archs)
  set(_default_arch ${CMAKE_SYSTEM_PROCESSOR})
elseif(_bits EQUAL 32)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]")
    set(_default_arch arm)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    set(_default_arch mips32)
  else()
    set(_default_arch i386)
  endif()
else()
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR
    CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]") # armv8
    set(_default_arch arm64)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    set(_default_arch mips64)
  else()
    set(_default_arch x86_64)
  endif()
endif()

set(BOOST_CONTEXT_ARCHITECTURE "${_default_arch}" CACHE STRING "Boost.Context architecture (arm, arm64, loongarch64, mips32, mips64, ppc32, ppc64, riscv64, s390x, i386, x86_64, combined)")
set_property(CACHE BOOST_CONTEXT_ARCHITECTURE PROPERTY STRINGS ${_all_archs})

unset(_all_archs)
unset(_bits)
unset(_default_arch)

## Assembler type

if(MSVC)
  if(BOOST_CONTEXT_ARCHITECTURE STREQUAL arm64 OR BOOST_CONTEXT_ARCHITECTURE STREQUAL arm)
    set(_default_asm armasm)
  else()
    set(_default_asm masm)
  endif()
else()
  set(_default_asm gas)
endif()

set(BOOST_CONTEXT_ASSEMBLER "${_default_asm}" CACHE STRING "Boost.Context assembler (masm, gas, armasm)")
set_property(CACHE BOOST_CONTEXT_ASSEMBLER PROPERTY STRINGS masm gas armasm)

unset(_default_asm)

## Assembler source suffix

if(BOOST_CONTEXT_BINARY_FORMAT STREQUAL pe)
  set(_default_ext .asm)
elseif(BOOST_CONTEXT_ASSEMBLER STREQUAL gas)
  set(_default_ext .S)
else()
  set(_default_ext .asm)
endif()

set(BOOST_CONTEXT_ASM_SUFFIX "${_default_ext}" CACHE STRING "Boost.Context assembler source suffix (.asm, .S)")
set_property(CACHE BOOST_CONTEXT_ASM_SUFFIX PROPERTY STRINGS .asm .S)

unset(_default_ext)

## Implementation

set(_default_impl fcontext)

set(BOOST_CONTEXT_IMPLEMENTATION "${_default_impl}" CACHE STRING "Boost.Context implementation (fcontext, ucontext, winfib)")
set_property(CACHE BOOST_CONTEXT_IMPLEMENTATION PROPERTY STRINGS fcontext ucontext winfib)

unset(_default_impl)

#

message(STATUS "Boost.Context: "
  "architecture ${BOOST_CONTEXT_ARCHITECTURE}, "
  "binary format ${BOOST_CONTEXT_BINARY_FORMAT}, "
  "ABI ${BOOST_CONTEXT_ABI}, "
  "assembler ${BOOST_CONTEXT_ASSEMBLER}, "
  "suffix ${BOOST_CONTEXT_ASM_SUFFIX}, "
  "implementation ${BOOST_CONTEXT_IMPLEMENTATION}")

# Enable the right assembler

if(BOOST_CONTEXT_IMPLEMENTATION STREQUAL "fcontext")
  if(BOOST_CONTEXT_ASSEMBLER STREQUAL gas)
    if(CMAKE_CXX_PLATFORM_ID MATCHES "Cygwin")
      enable_language(ASM-ATT)
    else()
      enable_language(ASM)
    endif()
  elseif(BOOST_CONTEXT_ASSEMBLER STREQUAL armasm)
    enable_language(ASM_ARMASM)
  else()
    enable_language(ASM_MASM)
  endif()
endif()

# Choose .asm sources

if(BOOST_CONTEXT_BINARY_FORMAT STREQUAL mach-o)
  set(BOOST_CONTEXT_BINARY_FORMAT macho)
endif()

set(_asm_suffix ${BOOST_CONTEXT_ARCHITECTURE}_${BOOST_CONTEXT_ABI}_${BOOST_CONTEXT_BINARY_FORMAT}_${BOOST_CONTEXT_ASSEMBLER}${BOOST_CONTEXT_ASM_SUFFIX})

set(ASM_SOURCES
  src/asm/make_${_asm_suffix}
  src/asm/jump_${_asm_suffix}
  src/asm/ontop_${_asm_suffix}
)

unset(_asm_suffix)

#

if(BOOST_CONTEXT_IMPLEMENTATION STREQUAL "fcontext")

  set(IMPL_SOURCES ${ASM_SOURCES})

  if(BOOST_CONTEXT_ASSEMBLER STREQUAL masm AND BOOST_CONTEXT_ARCHITECTURE STREQUAL i386)
      set_source_files_properties(${ASM_SOURCES} PROPERTIES COMPILE_FLAGS "/safeseh")
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_property(SOURCE ${ASM_SOURCES} APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp")
  endif()
else()
  set(IMPL_SOURCES
    src/continuation.cpp
    src/fiber.cpp
  )
endif()

if(WIN32 AND NOT CMAKE_CXX_PLATFORM_ID MATCHES "Cygwin")
  set(STACK_TRAITS_SOURCES
    src/windows/stack_traits.cpp
  )
else()
  set(STACK_TRAITS_SOURCES
    src/posix/stack_traits.cpp
  )
endif()

add_library(boost_context
  ${IMPL_SOURCES}
  ${STACK_TRAITS_SOURCES}
)

add_library(Boost::context ALIAS boost_context)

target_include_directories(boost_context PUBLIC include)

target_link_libraries(boost_context
  PUBLIC
    Boost::assert
    Boost::config
    Boost::core
    Boost::mp11
    Boost::pool
    Boost::predef
    Boost::smart_ptr
)

# ATTENTION. Some assemblers are picky to get --defsym,KEY=VALUE pairs where '=' is non-optional!
# Be sure all defines have the '=' character. Overkill solution: always put '=' throughout this module.

target_compile_definitions(boost_context
  PUBLIC BOOST_CONTEXT_NO_LIB=
  PRIVATE BOOST_CONTEXT_SOURCE=
)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(boost_context PUBLIC BOOST_CONTEXT_DYN_LINK= BOOST_CONTEXT_EXPORT=EXPORT)
else()
  target_compile_definitions(boost_context PUBLIC BOOST_CONTEXT_STATIC_LINK= BOOST_CONTEXT_EXPORT=)
endif()

if(BOOST_CONTEXT_IMPLEMENTATION STREQUAL "ucontext")
  target_compile_definitions(boost_context PUBLIC BOOST_USE_UCONTEXT=)
endif()

if(BOOST_CONTEXT_IMPLEMENTATION STREQUAL "winfib")
  target_compile_definitions(boost_context PUBLIC BOOST_USE_WINFIB=)
endif()

if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")

  add_subdirectory(test)

endif()
