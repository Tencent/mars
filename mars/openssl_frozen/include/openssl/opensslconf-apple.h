/*
 * Building OpenSSL for the different architectures of all iOS and tvOS devices requires different settings.
 * In order to be able to use assembly code on all devices, the choice was made to keep optimal settings for all
 * devices and use this intermediate header file to use the proper opensslconf.h file for each architecture.

 * See also https://github.com/x2on/OpenSSL-for-iPhone/issues/126 and referenced pull requests
 */

#include <TargetConditionals.h>

#if TARGET_OS_IOS && TARGET_OS_IPHONE
# include <openssl/opensslconf_ios_arm64.h>
#elif TARGET_OS_IOS && TARGET_OS_SIMULATOR && TARGET_CPU_X86_64
# include <openssl/opensslconf_ios_sim_x86_64.h>
#elif TARGET_OS_WATCH && TARGET_OS_IPHONE && TARGET_CPU_ARM
# include <openssl/opensslconf_watchos_armv7k.h>
#elif TARGET_OS_WATCH && TARGET_OS_IPHONE && TARGET_CPU_ARM64
# include <openssl/opensslconf_watchos_arm64_32.h>
#elif TARGET_OS_WATCH && TARGET_OS_SIMULATOR && TARGET_CPU_X86
# include <openssl/opensslconf_watchos_sim_i386.h>
#elif TARGET_OS_OSX && TARGET_CPU_X86_64
# include <openssl/opensslconf_macos_x86_64.h>
#elif TARGET_OS_OSX && TARGET_CPU_ARM64
# include <openssl/opensslconf_macos_arm64.h>
#else
# error Unable to determine target or target not included in OpenSSL build
#endif
