/*
 * Building OpenSSL for the different architectures of all iOS and tvOS devices requires different settings.
 * In order to be able to use assembly code on all devices, the choice was made to keep optimal settings for all
 * devices and use this intermediate header file to use the proper opensslconf.h file for each architecture.

 * See also https://github.com/x2on/OpenSSL-for-iPhone/issues/126 and referenced pull requests
 */

#ifdef __APPLE__
#include <openssl/opensslconf-apple.h>
#elif defined(__ANDROID__)
#include <openssl/opensslconf-android.h>
#elif defined(WIN32)
#include <openssl/opensslconf-windows.h>
#else
#error unsupported platform
#endif
