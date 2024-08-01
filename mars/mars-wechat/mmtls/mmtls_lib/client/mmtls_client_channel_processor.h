#pragma once

#include <vector>

#include "mmtls_channel_processor.h"
#include "mmtls_client_handshake_state.h"
#include "mmtls_handshake_messages.h"
#include "mmtls_handshake_state.h"
#include "mmtls_heartbeat_messages.h"

namespace mmtls {

// Handles client handshake operations.
class ClientChannelProcessor : public ChannelProcessor {
 public:
    virtual ~ClientChannelProcessor() {
    }

    int CreateClientHello(ClientHandShakeState& state, ClientHello*& client_hello);

    int CreateEncryptedExtensions(ClientHandShakeState& state, EncryptedExtensions*& encrypted_extensions);

    int CreateClientFinished(ClientHandShakeState& state, Finished*& client_finished);

    int CreateClientHeartbeat(ClientHandShakeState& state, const mmtls::String& payload, mmtls::String* heartbeat_str);

    int ProcessServerHello(ClientHandShakeState& state, ServerHello* server_hello);

    int ProcessCertificateVerify(ClientHandShakeState& state, CertificateVerify* certificate_verify);

    int ProcessNewSessionTicket(ClientHandShakeState& state, NewSessionTicket* new_session_ticket);

    int ProcessServerFinished(ClientHandShakeState& state, Finished* server_finished);
};

}  // namespace mmtls
