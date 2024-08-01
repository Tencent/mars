#pragma once

#include "mmtls_aead_crypter.h"
#include "mmtls_ciphersuite.h"
#include "mmtls_connection_keys.h"
#include "mmtls_constants.h"
#include "mmtls_version.h"

namespace mmtls {

// The cipher state of a connection.
// There are 4 cipher state: no cipher, SS base, ES base, master secret base.
// This class is refer to
// https://github.com/randombit/botan/blob/6a51315c4bea471a5093b0ce621643b7012a7d8c/src/lib/tls/tls_record.h
class ConnectionCipherState {
 public:
    // The keys and ivs in conn_keys will be swapped into cipher state.
    static ConnectionCipherState* Create(ProtocolVersion version,
                                         ConnectionSide side,
                                         bool is_our_side,
                                         const CipherSuite& suite,
                                         ConnectionKeys& conn_keys);

    ConnectionCipherState(ProtocolVersion version, ConnectionSide side, AeadCrypter* aead_crypter)
    : version_(version), aead_crypter_(aead_crypter) {
    }

    ~ConnectionCipherState();

    // Format aead nonce
    // See https://tlswg.github.io/tls13-spec/#rfc.section.4.8.2 for aead nonce details.
    // @param seq, the message sequence
    // @return nonce, the formated nocne
    void FormatNonce(uint64 seq, mmtls::String& nonce);

    // Format aead aad
    // @param seq, the message sequence
    // @param msg_type, the record type
    // @param msg_len, the length of record's payload
    // @return aad, the formated aad
    void FormatAad(uint64 seq, byte msg_type, uint16 msg_len, mmtls::String& aad);

    // Get connection protocol version.
    const ProtocolVersion& version() const {
        return version_;
    }

    // The encrypter based on the current cipher state.
    AeadCrypter* aead_crypter() const {
        return aead_crypter_;
    }

 private:
    ProtocolVersion version_;
    AeadCrypter* aead_crypter_;
};

}  // namespace mmtls
