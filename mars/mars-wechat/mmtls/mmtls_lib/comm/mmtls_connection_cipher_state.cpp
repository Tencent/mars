#include "mmtls_connection_cipher_state.h"

#include <algorithm>

#include "mmtls_aead_crypter_aes_gcm.h"

namespace mmtls {

ConnectionCipherState* ConnectionCipherState::Create(ProtocolVersion version,
                                                     ConnectionSide side,
                                                     bool is_our_side,
                                                     const CipherSuite& suite,
                                                     ConnectionKeys& conn_keys) {
    int ret = 0;
    ConnectionCipherState* cipher_state = NULL;

    if (suite.cipher_algo() == "AES_128_GCM" && suite.mac_algo() == "AEAD") {
        if (side == CONN_SIDE_CLIENT) {
            AeadCrypter* aead_crypter = NULL;

            if (is_our_side) {
                // client side write record
                aead_crypter = new AesGcmCrypter(suite.nonce_len(), suite.cipher_key_len(), 16, true);

                ret = aead_crypter->SetIvBySwap(conn_keys.client_write_iv());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set client write iv failed");

                ret = aead_crypter->SetKeyBySwap(conn_keys.client_write_key());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set client write key failed");
            } else {
                // client side read record
                aead_crypter = new AesGcmCrypter(suite.nonce_len(), suite.cipher_key_len(), 16, false);

                ret = aead_crypter->SetIvBySwap(conn_keys.server_write_iv());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set client read iv fialed");

                ret = aead_crypter->SetKeyBySwap(conn_keys.server_write_key());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set client read key failed");
            }

            // create connection cipher state
            cipher_state = new ConnectionCipherState(version, side, aead_crypter);
        } else if (side == CONN_SIDE_SERVER) {
            AeadCrypter* aead_crypter = NULL;

            if (is_our_side) {
                // server side write record
                aead_crypter = new AesGcmCrypter(suite.nonce_len(), suite.cipher_key_len(), 16, true);

                ret = aead_crypter->SetIvBySwap(conn_keys.server_write_iv());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set server write iv fialed");

                ret = aead_crypter->SetKeyBySwap(conn_keys.server_write_key());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set server write key failed");
            } else {
                // server side read record
                aead_crypter = new AesGcmCrypter(suite.nonce_len(), suite.cipher_key_len(), 16, false);

                ret = aead_crypter->SetIvBySwap(conn_keys.client_write_iv());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set server read iv fialed");

                ret = aead_crypter->SetKeyBySwap(conn_keys.client_write_key());
                MMTLS_CHECK_EQ(ret, OK, NULL, "set server read key failed");
            }

            // create connection cipher state
            cipher_state = new ConnectionCipherState(version, side, aead_crypter);
        }
    }

    return cipher_state;
}

ConnectionCipherState::~ConnectionCipherState() {
    if (aead_crypter_) {
        delete aead_crypter_;
        aead_crypter_ = NULL;
    }
}

// Format aead nonce
void ConnectionCipherState::FormatNonce(uint64 seq, mmtls::String& nonce) {
    // See https://tlswg.github.io/tls13-spec/#record-payload-protection
    nonce.clear();

    const mmtls::String& iv = aead_crypter_->GetIv();
    nonce.resize(std::max(0, static_cast<int>(iv.size()) - 8), '\0');

    DataWriter writer(nonce);
    writer.Write(seq);

    for (size_t i = 0; i < iv.size(); ++i) {
        nonce[i] ^= iv[i];
    }
}

// Format aead aad
void ConnectionCipherState::FormatAad(uint64 seq, byte msg_type, uint16 msg_len, mmtls::String& aad) {
    aad.clear();

    DataWriter writer(aad);
    writer.Write(seq);
    writer.Write(msg_type);
    writer.Write(version_.version());
    writer.Write(msg_len);
}

}  // namespace mmtls
