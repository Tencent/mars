#ifndef MARS_FILESYSTEM_MACROS_H_
#define MARS_FILESYSTEM_MACROS_H_

#define MARS_FILESYSTEM_INLINE inline
#define MARS_FILESYSTEM_API /* __attribute__((visibility("default"))) */

#if __cplusplus < 201103L
#define noexcept /* noexcept */
#endif

// #if __cplusplus >= 201103L
// #define CPP11GOOD
// #endif

#endif