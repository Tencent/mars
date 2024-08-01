#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <string>
namespace mmtls {

#ifdef uint8
#undef uint8
#endif
typedef uint8_t byte;

#ifdef uint16
#undef uint16
#endif
typedef uint16_t uint16;

#ifdef uint32
#undef uint32
#endif
typedef uint32_t uint32;

#ifdef uint64
#undef uint64
#endif
typedef uint64_t uint64;

#ifdef __UNITEST_BUILD__
#undef private
#define private public

#undef protected
#define protected public
#endif

};  // namespace mmtls
