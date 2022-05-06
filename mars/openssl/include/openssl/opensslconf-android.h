/*
 * Building OpenSSL for the different architectures of all iOS and tvOS devices requires different settings.
 * In order to be able to use assembly code on all devices, the choice was made to keep optimal settings for all
 * devices and use this intermediate header file to use the proper opensslconf.h file for each architecture.

 * See also https://github.com/x2on/OpenSSL-for-iPhone/issues/126 and referenced pull requests
 */


#if defined(__arm__)
#include <openssl/opensslconf_android-arm.h>
#elif defined(__aarch64__)
#include <openssl/opensslconf_android-arm64.h>
#elif defined(__i386__)
#include <openssl/opensslconf_android-x86_64.h>
#elif defined(__x86_64__)
#include <openssl/opensslconf_android64-x86_64.h>
#else
# error Unable to determine arch not included in OpenSSL build
#endif 
