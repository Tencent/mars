#pragma once

#include "mmtls_audit.h"
#include "mmtls_ciphersuite.h"
#include "mmtls_crypto_util.h"
#include "mmtls_handshake_messages.h"
#include "mmtls_version.h"

namespace mmtls {

class ConnectionKeys;
class ConnectionCipherState;
class CryptoUtil;

enum enCriticalAudit {
    CRITICAL_AUDIT_STATIC_SECRET = 1,
    CRITICAL_AUDIT_EPHEMERAL_SECRET = 2,
    CRITICAL_AUDIT_MASTER_SECRET = 3,

};

typedef Audit<enCriticalAudit, uint64> CriticalAudit;

// Stores handshake related data.
class HandShakeState {
 public:
    // The information for a handshake message.
    struct HandShakeMsgInfo {
        HandShakeType type;
        HandShakeMessage* msg;
        ConnectionSide side;
    };

    typedef std::vector<HandShakeMsgInfo> HandShakeMessages;

    // Get the first handshake message that match the type.
    template <typename T>
    T* Get() {
        HandShakeType type = T::StaticType();
        for (HandShakeMessages::iterator it = handshake_messages_.begin(); it != handshake_messages_.end(); ++it) {
            if (it->type == type) {
                return dynamic_cast<T*>(it->msg);
            }
        }
        return NULL;
    }

    // Get the first handshake message that match the type and connection side.
    template <typename T>
    T* Get(ConnectionSide side) {
        HandShakeType type = T::StaticType();
        for (HandShakeMessages::iterator it = handshake_messages_.begin(); it != handshake_messages_.end(); ++it) {
            if (it->type == type && it->side == side) {
                return dynamic_cast<T*>(it->msg);
            }
        }
        return NULL;
    }

    template <typename T>
    bool Has() const {
        return (Get<T>() != NULL);
    }

    HandShakeState(ConnectionSide con_side)
    : stage_(STAGE_NONE)
    , is_next_handshake_loop_(false)
    , mode_(HS_MODE_NONE)
    , protocol_version_(ProtocolVersion::TLS_V13)
    , crypto_util_(NULL)
    , con_side_(con_side)
    , init_time_(NowUTime())
    , active_time_(0)
    , close_time_(0)
    , set_flags_(0)
    , send_to_peer_cnt_(0)
    , client_id_("unsetid") {
    }

    virtual ~HandShakeState();

    StageAudit& stage_audit() {
        return stage_audit_;
    }
    CriticalAudit& critical_audit() {
        return critical_audit_;
    }

    // Add a handshake message. The msg will be deleted if HandShakeState is disposed.
    // The serialized_msg is to avoid serialize message twice.
    // The side specify who generate the handshake message.
    int Add(HandShakeMessage* msg, const mmtls::String& serialized_msg, ConnectionSide side);

    // Derialize a handshake message from network order binary data
    // The deserialized msg won't be add to internal state.
    // msg should not be null.
    int DeserializeMsg(DataReader& reader, HandShakeMessage** msg);

    // Get the hash of current handshake bytes.
    const mmtls::String& handshake_hash();
    // Update the hash of current handshake bytes.
    int UpdateHandShakeHash();

    ConnectionSide con_side() const {
        return con_side_;
    }

    HandShakeStage stage() const {
        return stage_;
    }

    void SetNextStage(HandShakeStage stage, bool is_next_handshake_loop);
    bool IsNextHandShakeLoop() const {
        return is_next_handshake_loop_;
    }

    HandShakeMode mode() const {
        return mode_;
    }
    void SetMode(HandShakeMode mode) {
        mode_ = mode;
    }

    const ProtocolVersion& protocol_version() const {
        return protocol_version_;
    }
    void set_protocol_version(const ProtocolVersion& version) {
        protocol_version_ = version;
    }

    const CipherSuite& cipher_suite() const {
        return cipher_suite_;
    }
    void set_cipher_suite(uint16 ciphersuite_code);

    const mmtls::String& static_secret() const {
        return static_secret_;
    }
    void set_static_secret(const mmtls::String& static_secret) {
        static_secret_ = static_secret;
        if (static_secret.size() > 0) {
            critical_audit_.set_flag(CRITICAL_AUDIT_STATIC_SECRET);
        }
    }

    const mmtls::String& ephemeral_secret() const {
        return ephemeral_secret_;
    }
    void set_ephemeral_secret(const mmtls::String& ephemeral_secret) {
        ephemeral_secret_ = ephemeral_secret;
        if (ephemeral_secret.size() > 0) {
            critical_audit_.set_flag(CRITICAL_AUDIT_EPHEMERAL_SECRET);
        }
    }

    // ComputeMasterSecret should be called before calling this getter.
    const mmtls::String& master_secret() const {
        return master_secret_;
    }

    // Compute master secret
    // See https://tlswg.github.io/tls13-spec/#rfc.section.7.1
    int ComputeMasterSecret();

    // Compute connection keys.
    // See https://tlswg.github.io/tls13-spec/#rfc.section.7.2
    int ComputeStageConnKey(const mmtls::String& secret_key, const mmtls::String& lable, ConnectionKeys*& conn_keys);

    // Derive mac key and compute mac value for psk.
    int ComputePskHmac(const mmtls::String& message, mmtls::String& mac_key, mmtls::String& mac_value);

    // Compute a connection cipher state.
    // The keys and ivs in conn_keys will be swapped into cipher_state.
    int ComputeStageConnCipherState(ConnectionSide conn_side,
                                    bool is_our_side,
                                    ConnectionKeys& conn_keys,
                                    ConnectionCipherState*& cipher_state);

    // Compute verification data used in Finished.
    int ComputeVerifyData(ConnectionSide side, mmtls::String& verify_data);

    // Sign certificate verify for handshake hash.
    int SignCertificate(const mmtls::String& pri_key, mmtls::String& signature);

    // Verify certificate verify for handshake hash.
    int VerifyCertificate(const mmtls::String& pub_key, const mmtls::String& signature);

    // Compute resumption secret
    int ComputeResumptionSecret(PskType type, mmtls::String& resumption_secret);

    // Compute url signature
    int ComputeFallBackUrlSignature(const mmtls::String& pri_key, const mmtls::String& url, mmtls::String& signature);

    // Verify url signature
    int VerifyFallBackUrlSignature(const mmtls::String& pub_key,
                                   const mmtls::String& url,
                                   const mmtls::String& signature);

    CryptoUtil* crypto_util() {
        return crypto_util_;
    }

    // Session is established. Clear unused material to ensure security.
    virtual void ActiveSession();
    virtual void Close();
    virtual void ClearHandshakeMessages();

    void attach_crypto_util(CryptoUtil* crypto_util);

    uint64 init_time() const {
        return init_time_;
    }

    uint64 active_time() const {
        return active_time_;
    }

    uint64 close_time() const {
        return close_time_;
    }

    uint32 send_to_peer_cnt() const {
        return send_to_peer_cnt_;
    }
    void inc_send_to_peer_cnt() {
        send_to_peer_cnt_++;
    }

    void set_client_id(const mmtls::String& random);
    const char* client_id() const {
        return client_id_.c_str();
    }
    mmtls::String& log_impt() {
        return log_impt_;
    }

 protected:
    // Disallow copying.
    HandShakeState(const HandShakeState&);
    HandShakeType& operator=(const HandShakeState&);

    // Received and sent handshake messages.
    HandShakeMessages handshake_messages_;

    // The serialized handshake message bytes. Except Finished.
    mmtls::String handshake_bytes_;
    mmtls::String handshake_hash_;
    std::unique_ptr<CryptoUtil::Hash> handshake_hash_obj_;

    // The current handshake stage.
    HandShakeStage stage_;
    bool is_next_handshake_loop_;

    // The current chosen handshake mode.
    HandShakeMode mode_;

    ProtocolVersion protocol_version_;
    CipherSuite cipher_suite_;

    // The secrets for computing connection keys.
    mmtls::String static_secret_;
    mmtls::String ephemeral_secret_;
    mmtls::String master_secret_;

    CryptoUtil* crypto_util_;
    ConnectionSide con_side_;

    uint64 init_time_;
    uint64 active_time_;
    uint64 close_time_;

    uint32 set_flags_;

    StageAudit stage_audit_;
    CriticalAudit critical_audit_;
    uint32 send_to_peer_cnt_;
    mmtls::String client_id_;
    mmtls::String log_impt_;
};

}  // namespace mmtls
