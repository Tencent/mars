#include "mmtls_client_channel_processor.h"

#include <string>
#include <vector>

#include "mmtls_constants.h"
#include "mmtls_handshake_messages.h"
#include "mmtls_string.h"
#include "mmtls_utils.h"

namespace mmtls {

int ClientChannelProcessor::CreateClientHello(ClientHandShakeState& state, ClientHello*& client_hello) {
    MMTLS_CHECK_TRUE(NULL != state.crypto_util(), ERR_UNEXPECT_CHECK_FAIL, "crypto util is null");
    mmtls::String random;
    std::vector<CipherSuite> ciphersuites;
    client_hello = NULL;

    AutoPointHolder<ClientHello> holder(client_hello);

    uint32 client_gmt_time = NowTime();

    int ret = state.crypto_util()->GenerateRandom(FIXED_RANDOM_SIZE, random);
    MMTLS_CHECK_EQ(OK, ret, ret, "generate client random fail");
    MMTLSLOG_DEBUG("debug: client random %s", SafeStrToHex(random).c_str());

    switch (state.mode()) {
        case HS_MODE_ONE_RTT_ECDH:
            // Select ciphersuites and create ClientHello.
            ciphersuites.push_back(CipherSuite::GetByCode(CipherSuite::TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256));
            if (NULL != state.refresh_psk()) {
                // Add Psk ciphersuite for disaster recovery.
                ciphersuites.push_back(CipherSuite::GetByCode(CipherSuite::TLS_PSK_WITH_AES_128_GCM_SHA256));
            }
            client_hello = new ClientHello(state.protocol_version(), ciphersuites, random, client_gmt_time);

            // Create ClientKeyShare.
            {
                std::vector<ClientKeyShare::ClientKeyOffer> offers;
                std::vector<KeyPair> ephemeral_client_ecdh_keys;
                for (size_t j = 0; j < state.static_server_ecdh_keys().size(); ++j) {
                    const KeyPair& static_key_pair = state.static_server_ecdh_keys()[j];

                    KeyPair ephemeral_key_pair;
                    int ret = state.crypto_util()->GenEcdhKeyPair(static_key_pair.nid(), ephemeral_key_pair);
                    MMTLS_CHECK_EQ(OK, ret, ret, "generate ecdh key pair fail");

                    ephemeral_key_pair.set_version(static_key_pair.version());
                    ephemeral_client_ecdh_keys.push_back(ephemeral_key_pair);

                    ClientKeyShare::ClientKeyOffer offer(ephemeral_key_pair.version(), ephemeral_key_pair.public_key());
                    offers.push_back(offer);
                }
                state.set_ephemeral_client_ecdh_keys(ephemeral_client_ecdh_keys);

                std::vector<uint32> certificate_versions;
                for (size_t j = 0; j < state.ecdsa_keys().size(); ++j) {
                    const uint32 certificate_version = state.ecdsa_keys()[j].version();
                    certificate_versions.push_back(certificate_version);
                    MMTLSLOG_DEBUG("debug: ecdsa version %d", certificate_version);
                }

                client_hello->extensions()->Add(new ClientKeyShare(offers, certificate_versions));
            }

            if (state.refresh_psk() != NULL) {
                // Create PreSharedKeyExtension.
                client_hello->extensions()->Add(new PreSharedKeyExtension(state.refresh_psk()->psk()->SafeClone()));
            }

            break;

        case HS_MODE_ZERO_RTT_PSK:
        case HS_MODE_ONE_RTT_PSK:
            MMTLS_CHECK_TRUE(NULL != state.access_psk(), ERR_UNEXPECT_CHECK_FAIL, "no access psk");

            // Select ciphersuites and create ClientHello.
            ciphersuites.push_back(CipherSuite::GetByCode(CipherSuite::TLS_PSK_WITH_AES_128_GCM_SHA256));
            client_hello = new ClientHello(state.protocol_version(), ciphersuites, random, client_gmt_time);

            // Create PreSharedKeyExtension.
            client_hello->extensions()->Add(new PreSharedKeyExtension(state.access_psk()->psk()->SafeClone()));

            if (HS_MODE_ZERO_RTT_PSK == state.mode()) {
                // Set static secret for computing early data cipher state.
                state.set_static_secret(state.access_psk()->pre_shared_key());
            }

            break;

        default:
            MMTLSLOG_ERR("unknown handshake mode");
            return ERR_UNEXPECT_CHECK_FAIL;
    }

    holder.Disable();

    return OK;
}

int ClientChannelProcessor::CreateEncryptedExtensions(ClientHandShakeState& state,
                                                      EncryptedExtensions*& encrypted_extensions) {
    // Create EncryptedExtensions.
    encrypted_extensions = new EncryptedExtensions();
    encrypted_extensions->extensions()->Add(new EarlyEncryptData(NowTime()));

    return OK;
}

int ClientChannelProcessor::CreateClientHeartbeat(ClientHandShakeState& state,
                                                  const mmtls::String& payload,
                                                  mmtls::String* heartbeat_str) {
    MMTLS_CHECK_TRUE(NULL != heartbeat_str, ERR_UNEXPECT_CHECK_FAIL, "heartbeat str is null");
    MMTLS_CHECK_TRUE(NULL != state.crypto_util(), ERR_UNEXPECT_CHECK_FAIL, "crypto util is null");

    mmtls::String random;
    int ret = state.crypto_util()->GenerateRandom(FIXED_RANDOM_SIZE, random);
    MMTLS_CHECK_EQ(OK, ret, ret, "generate heartbeat random fail");

    Heartbeat heartbeat(HEARTBEAT_REQUEST, payload, random);

    DataWriter writer;
    ret = heartbeat.Serialize(writer);
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize heartbeat message fail");

    *heartbeat_str = writer.Buffer();

    return OK;
}

int ClientChannelProcessor::CreateClientFinished(ClientHandShakeState& state, Finished*& client_finished) {
    // Create client finished.
    mmtls::String verify_data;

    int ret = state.ComputeVerifyData(CONN_SIDE_CLIENT, verify_data);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute verify data for client finished fail");

    client_finished = new Finished(verify_data);

    return OK;
}

int ClientChannelProcessor::ProcessServerHello(ClientHandShakeState& state, ServerHello* server_hello) {
    MMTLS_CHECK_TRUE(NULL != state.crypto_util(), ERR_UNEXPECT_CHECK_FAIL, "crypto util is null");
    MMTLS_CHECK_TRUE(server_hello != NULL, ERR_UNEXPECT_CHECK_FAIL, "server hello is null");

    ClientHello* client_hello = state.Get<ClientHello>();
    MMTLS_CHECK_TRUE(client_hello != NULL, ERR_UNEXPECT_CHECK_FAIL, "client hello is null");

    // Check if server mmtls version is the same as client's.
    MMTLS_CHECK_EQ(client_hello->version().version(),
                   server_hello->version().version(),
                   ERR_INVALID_SERVER_HELLO,
                   "server hello version is not match client's");

    // Check if server chosen ciphersuite is valid.
    bool is_valid = false;
    for (size_t i = 0; i < client_hello->ciphersuites().size(); ++i) {
        if (server_hello->ciphersuite() == client_hello->ciphersuites()[i]) {
            is_valid = true;
            break;
        }
    }
    MMTLS_CHECK_TRUE(is_valid, ERR_INVALID_SERVER_HELLO, "server chosen ciphersuite is invalid");

    MMTLSLOG_DEBUG("debug: server random %s", SafeStrToHex(server_hello->random()).c_str());

    // Set static and ephemeral secrets.
    if (HS_MODE_ONE_RTT_ECDH == state.mode()) {
        if (server_hello->ciphersuite().ciphersuite_code() == CipherSuite::TLS_PSK_WITH_AES_128_GCM_SHA256) {
            MMTLSLOG_DEBUG("debug: ecdh handshake: server select refresh psk");

            // We don't check server returned psk because we only send one psk.
            // PreSharedKeyExtension* ext = server_hello->extensions()->Get<PreSharedKeyExtension>();
            // MMTLS_CHECK_TRUE(ext != NULL, ERR_INVALID_SERVER_HELLO, "server hello doesn't have pre shared key
            // extension"); MMTLS_CHECK_TRUE(ext->psk() != NULL, ERR_INVALID_SERVER_HELLO, "server hello's pre shared
            // key extension doesn't have psk"); MMTLS_CHECK_TRUE(ext->psk()->type() == PSK_REFRESH,
            // ERR_INVALID_SERVER_HELLO, "server hello's pre shared key extension psk is not refresh psk");
            MMTLS_CHECK_TRUE(state.refresh_psk() != NULL, ERR_UNEXPECT_CHECK_FAIL, "no refresh psk for psk handshake");

            state.set_static_secret(state.refresh_psk()->pre_shared_key());
            state.set_ephemeral_secret(state.refresh_psk()->pre_shared_key());

        } else if (server_hello->ciphersuite().ciphersuite_code()
                   == CipherSuite::TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256) {
            MMTLSLOG_DEBUG("debug: ecdh handshake: server select ecdh");
            ServerKeyShare* server_key_share = server_hello->extensions()->Get<ServerKeyShare>();
            MMTLS_CHECK_TRUE(server_key_share != NULL,
                             ERR_INVALID_SERVER_HELLO,
                             "server hello doesn't have server key share");

            bool match_ecdh_version = false;
            for (size_t i = 0; i < state.ephemeral_client_ecdh_keys().size(); ++i) {
                const KeyPair& ephemeral_key_pair = state.ephemeral_client_ecdh_keys()[i];

                if (ephemeral_key_pair.version() == server_key_share->version()) {
                    match_ecdh_version = true;

                    mmtls::String ephemeral_secret;

                    int ret = state.crypto_util()->ComputeDh(ephemeral_key_pair.nid(),
                                                             server_key_share->public_value(),
                                                             ephemeral_key_pair.private_key(),
                                                             ephemeral_secret);
                    MMTLS_CHECK_EQ(OK, ret, ret, "compute ephemeral secret fail");

                    // For ecdh full handshake, static secret is the same as ephemeral secret.
                    state.set_ephemeral_secret(ephemeral_secret);
                    state.set_static_secret(ephemeral_secret);

                    break;
                }
            }
            MMTLS_CHECK_TRUE(match_ecdh_version, ERR_INVALID_SERVER_HELLO, "server hello ecdh key version not match");
        }

        CertRegion* cert_region = server_hello->extensions()->Get<CertRegion>();
        if (cert_region != NULL) {
            state.set_cert_region(cert_region->cert_region());
            MMTLSLOG_DEBUG("found cert region %u", cert_region->cert_region());

            const uint32 certificate_version = cert_region->certificate_version();
            int ret = state.set_ecdsa_key(certificate_version);
            MMTLS_CHECK_EQ(OK, ret, ret, "set ecdsa key failed, version %u", certificate_version);
        } else {
            MMTLSLOG_ERR("not found cert region extension");
        }

    } else if (HS_MODE_ZERO_RTT_PSK == state.mode() || HS_MODE_ONE_RTT_PSK == state.mode()) {
        if (server_hello->ciphersuite().ciphersuite_code() == CipherSuite::TLS_PSK_WITH_AES_128_GCM_SHA256) {
            // We don't check server returned psk because we only send one psk.
            // PreSharedKeyExtension* ext = server_hello->extensions()->Get<PreSharedKeyExtension>();
            // MMTLS_CHECK_TRUE(ext != NULL, ERR_INVALID_SERVER_HELLO, "server hello doesn't have pre shared key
            // extension"); MMTLS_CHECK_TRUE(ext->psk() != NULL, ERR_INVALID_SERVER_HELLO, "server hello's pre shared
            // key extension doesn't have psk"); MMTLS_CHECK_TRUE(ext->psk()->type() == PSK_ACCESS,
            // ERR_INVALID_SERVER_HELLO, "server hello's pre shared key extension psk is not access psk");
            MMTLS_CHECK_TRUE(state.access_psk() != NULL, ERR_UNEXPECT_CHECK_FAIL, "no access psk for psk handshake");

            state.set_static_secret(state.access_psk()->pre_shared_key());
            state.set_ephemeral_secret(state.access_psk()->pre_shared_key());
        }
    } else {
        MMTLSLOG_ERR("unknown handshake mode");
        return ERR_UNEXPECT_CHECK_FAIL;
    }

    // Change to the server selected cipher suite.
    if (state.cipher_suite().ciphersuite_code() != server_hello->ciphersuite().ciphersuite_code()) {
        state.set_cipher_suite(server_hello->ciphersuite().ciphersuite_code());
        MMTLSLOG_DEBUG("debug: change to server selected cipher suite %s", state.cipher_suite().ToString().c_str());
    }

    return OK;
}

int ClientChannelProcessor::ProcessCertificateVerify(ClientHandShakeState& state,
                                                     CertificateVerify* certificate_verify) {
    MMTLS_CHECK_TRUE(NULL != state.crypto_util(), ERR_UNEXPECT_CHECK_FAIL, "crypto util is null");
    MMTLS_CHECK_TRUE(certificate_verify != NULL, ERR_UNEXPECT_CHECK_FAIL, "certificate_verify is null");
    MMTLS_CHECK_EQ(HS_MODE_ONE_RTT_ECDH,
                   state.mode(),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "process certificate verify in wrong handshake mode");

    if (state.cipher_suite().ciphersuite_code() == CipherSuite::TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256) {
        int ret = state.VerifyCertificate(state.ecdsa_key().public_key(), certificate_verify->signature());
        MMTLS_CHECK_EQ(OK, ret, ret, "verify certificate verify signature fail");
    } else if (state.cipher_suite().ciphersuite_code() == CipherSuite::TLS_PSK_WITH_AES_128_GCM_SHA256) {
        MMTLSLOG_DEBUG("debug: refresh psk mac key %s, mac value %s",
                       SafeStrToHex(certificate_verify->signature()).c_str(),
                       SafeStrToHex(state.refresh_psk()->psk()->mac_value()).c_str());

        MMTLS_CHECK_TRUE(NULL != state.refresh_psk(), ERR_UNEXPECT_CHECK_FAIL, "refresh psk is null");
        mmtls::String mac;
        int ret = state.crypto_util()->MessageAuthCode(certificate_verify->signature(),
                                                       state.refresh_psk()->pre_shared_key(),
                                                       mac);
        MMTLS_CHECK_EQ(OK, ret, ret, "calculate refresh psk pre_shared_key mac fail");
        MMTLS_CHECK_EQ(mac,
                       state.refresh_psk()->psk()->mac_value(),
                       ERR_RECEIVE_VERIFY_DATA_INVALID,
                       "verify refresh psk pre_shard_key mac fail");
    } else {
        MMTLSLOG_ERR("unknown cipher suite");
        return ERR_UNEXPECT_CHECK_FAIL;
    }

    return OK;
}

int ClientChannelProcessor::ProcessNewSessionTicket(ClientHandShakeState& state, NewSessionTicket* new_session_ticket) {
    MMTLS_CHECK_TRUE(new_session_ticket != NULL, ERR_UNEXPECT_CHECK_FAIL, "new_session_ticket is null");
    MMTLS_CHECK_LE(new_session_ticket->psks().size(),
                   2U,
                   ERR_UNEXPECT_CHECK_FAIL,
                   "psk num  %zu over",
                   new_session_ticket->psks().size());
    for (size_t i = 0; i < new_session_ticket->psks().size(); ++i) {
        Psk* psk = new_session_ticket->psks()[i];
        MMTLS_CHECK_TRUE(psk != NULL, ERR_UNEXPECT_CHECK_FAIL, "new session ticket psk is null");

        mmtls::String secret;
        int ret = state.ComputeResumptionSecret(psk->type(), secret);
        MMTLS_CHECK_EQ(OK, ret, ret, "compute resumption secret fail, psk type %d", psk->type());

        ClientPsk* client_psk = new ClientPsk(new Psk(*psk), NowTime() + psk->ticket_lifetime_hint(), secret);
        if (PSK_ACCESS == psk->type()) {
            state.set_new_access_psk(client_psk);
        } else {
            state.set_new_refresh_psk(client_psk);
        }
    }

    return OK;
}

int ClientChannelProcessor::ProcessServerFinished(ClientHandShakeState& state, Finished* server_finished) {
    MMTLS_CHECK_TRUE(server_finished != NULL, ERR_UNEXPECT_CHECK_FAIL, "server_finished is null");
    mmtls::String verify_data;
    int ret = state.ComputeVerifyData(CONN_SIDE_SERVER, verify_data);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute server finshed verify data fail");

    MMTLS_CHECK_TRUE(verify_data.safe_eq(server_finished->verify_data()),
                     ERR_RECEIVE_VERIFY_DATA_INVALID,
                     "server finished is invalid");

    return OK;
}

}  // namespace mmtls
