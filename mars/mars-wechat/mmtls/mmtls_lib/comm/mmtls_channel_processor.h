#pragma once

#include <map>
#include <string>

#include "mmtls_ciphersuite.h"
#include "mmtls_connection_keys.h"
#include "mmtls_constants.h"
#include "mmtls_crypto_util.h"

namespace mmtls {

class ConnectionCipherState;

// Handles handshake operations.
class ChannelProcessor {
 public:
    ChannelProcessor(){};

    virtual ~ChannelProcessor(){};
};

}  // namespace mmtls
