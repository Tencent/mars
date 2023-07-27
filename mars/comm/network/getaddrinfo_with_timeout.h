#ifndef getaddrinfo_with_timeout_h
#define getaddrinfo_with_timeout_h

#include <netdb.h>

namespace mars {
namespace comm {

int getaddrinfo_with_timeout(const char* node,
                             const char* service,
                             const struct ::addrinfo* hints,
                             struct ::addrinfo** res,
                             bool& _is_timeout,
                             unsigned long long _timeout_msec);

}
}  // namespace mars

#endif /* getaddrinfo_with_timeout_h */
