/*
 * Building OpenSSL for the different architectures of all iOS and tvOS devices requires different settings.
 * In order to be able to use assembly code on all devices, the choice was made to keep optimal settings for all
 * devices and use this intermediate header file to use the proper opensslconf.h file for each architecture.

 * See also https://github.com/x2on/OpenSSL-for-iPhone/issues/126 and referenced pull requests
 */

// see https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-160
#if defined(_WIN64)
#include <openssl/opensslconf_windows-x86_64.h>
#else
#include <openssl/opensslconf_windows-x86.h>
#endif