#pragma once

#include <stdint.h>

#include <map>
#include <string>

#include "mmtls_ciphersuite.h"
#include "mmtls_psk.h"
#include "mmtls_string.h"
#include "mmtls_version.h"

namespace mmtls {

// The PSK generator interface. Server use this interface to
// 1. Generate new PSK.
// 2. Decrypt a PSK ticket and verify it. Verify process contains validating ticket content and check replay attack.
class PskGenerator {
 public:
    PskGenerator() {
    }
    virtual ~PskGenerator() {
    }

    // Generate a new PSK.
    virtual int GeneratePsk(PskType type,
                            const ProtocolVersion& version,
                            const CipherSuite& ciphersuite,
                            const mmtls::String& pre_shared_key,
                            const mmtls::String& mac_key,
                            const mmtls::String& mac_value,
                            uint32 ticket_lifetime,
                            uint32 client_gmt_time,
                            uint32 ecdh_key_version,
                            Psk*& psk) = 0;

    // Decrypt the psk ticket and verify the pre_shared_key.
    virtual int VerifyAndGetTicket(const Psk& psk, Psk::Ticket& ticket) = 0;
};

}  // namespace mmtls
