#include "mmtls_handshake_state.h"

#include <algorithm>
#include <sstream>

#include "mmtls_connection_cipher_state.h"
#include "mmtls_connection_keys.h"
#include "mmtls_constants.h"
#include "mmtls_openssl_crypto_util.h"

namespace mmtls {

HandShakeState::~HandShakeState() {
    ClearHandshakeMessages();

    if (crypto_util_) {
        delete crypto_util_;
        crypto_util_ = NULL;
    }
}

int HandShakeState::Add(HandShakeMessage* msg, const mmtls::String& serialized_msg, ConnectionSide side) {
    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "handshake message is null");
    if (msg->GetType() != FINISHED) {
        handshake_hash_.clear();
        if (handshake_hash_obj_.get()) {
            int ret = OK;
            if (!handshake_bytes_.empty()) {
                ret = handshake_hash_obj_->Update((const byte*)handshake_bytes_.data(), handshake_bytes_.size());
                MMTLS_CHECK_EQ(OK, ret, ret, "update hs hash fail.ret %d", ret);

                handshake_bytes_.clear();
            }
            ret = handshake_hash_obj_->Update((const byte*)serialized_msg.data(), serialized_msg.size());
            MMTLS_CHECK_EQ(OK, ret, ret, "update hs hash fail.ret %d", ret);

        } else {
            handshake_bytes_ += serialized_msg;
        }
    }

    HandShakeMsgInfo info;
    info.msg = msg;
    info.type = msg->GetType();
    info.side = side;
    handshake_messages_.push_back(info);
    return OK;
}

int HandShakeState::DeserializeMsg(DataReader& reader, HandShakeMessage** msg) {
    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "handshake message pointer is null");
    HandShakeType type = HANDSHAKE_NONE;

    int ret = HandShakeMessage::DetectHandShakeType(reader, type);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type");

    *msg = NULL;
    switch (type) {
        case CLIENT_HELLO:
            *msg = new ClientHello();
            break;
        case SERVER_HELLO:
            *msg = new ServerHello();
            break;
        case ENCRYPTED_EXTENSIONS:
            *msg = new EncryptedExtensions();
            break;
        case NEW_SESSION_TICKET:
            *msg = new NewSessionTicket();
            break;
        case CERTIFICATE_VERIFY:
            *msg = new CertificateVerify();
            break;
        case FINISHED:
            *msg = new Finished();
            break;
        default:
            *msg = NULL;
    }

    MMTLS_CHECK_TRUE(*msg, ERR_DECODE_ERROR, "unknown handshake message type");

    ret = (*msg)->Deserialize(reader);
    if (OK != ret) {
        delete (*msg);
        *msg = NULL;
    }
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "deserialize handshake message");

    return OK;
}

const mmtls::String& HandShakeState::handshake_hash() {
    return handshake_hash_;
}

int HandShakeState::UpdateHandShakeHash() {
    if (handshake_hash_.empty()) {
        MMTLS_CHECK_TRUE(NULL != crypto_util_, ERR_UNEXPECT_CHECK_FAIL, "crypto util should not be null");
        MMTLS_CHECK_TRUE(NULL != handshake_hash_obj_.get(), ERR_UNEXPECT_CHECK_FAIL, "handshake hash object is null");

        // update the old first
        if (!handshake_bytes_.empty()) {
            int ret = handshake_hash_obj_->Update((const byte*)handshake_bytes_.data(), handshake_bytes_.size());
            MMTLS_CHECK_EQ(OK, ret, ret, "update hs hash fail.ret %d", ret);
            handshake_bytes_.clear();
        }

        // clone new hash
        std::unique_ptr<CryptoUtil::Hash> new_hash(handshake_hash_obj_->Clone());
        MMTLS_CHECK_TRUE(new_hash.get(), ERR_UNEXPECT_CHECK_FAIL, "clone handshake hash obj fail");

        // final new one
        int ret = handshake_hash_obj_->Final(handshake_hash_);
        MMTLS_CHECK_EQ(OK, ret, ret, "compute handshake hash fail");

        handshake_hash_obj_.reset(new_hash.release());
    }

    MMTLS_CHECK_GT(handshake_hash_.size(), 0U, ERR_UNEXPECT_CHECK_FAIL, "handshake hash is empty");

    return OK;
}

int HandShakeState::ComputeMasterSecret() {
    // ECDH key agreement algorithm ensure that static secret and ephemeral secret are pseudorandom string, so we
    // skipped HKDF-Extract, So if new key agreement algorithm is added, we should make sure the static secret and
    // ephemeral secret are pseudorandom string too. See rfc5869 3.3 part.

    // See https://tlswg.github.io/tls13-spec/#key-schedule

    MMTLS_CHECK_EQ(
        OK,
        critical_audit_.CheckHas(CriticalAudit(2, CRITICAL_AUDIT_STATIC_SECRET, CRITICAL_AUDIT_EPHEMERAL_SECRET)),
        ERR_UNEXPECT_CHECK_FAIL,
        "audit fail");

    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash fail");

    if (static_secret_ == ephemeral_secret_) {
        // SS equal to ES, use one HKDF-Expand to derive master secret.

        mmtls::String label(EXPAND_LABLE);
        label.append(handshake_hash());
        ret = crypto_util_->HkdfExpand(static_secret_, label, FIXED_SECRET_SIZE, master_secret_);
        MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand secret fail");
        MMTLS_CHECK_EQ(FIXED_SECRET_SIZE, master_secret_.size(), ERR_UNEXPECT_CHECK_FAIL, "master secret size invalid");
    } else {
        // SS not equal to ES, follow TLS1.3 standard to derive master secret.

        mmtls::String mSS, mES;

        mmtls::String label(EXPAND_SS_LABLE);
        label.append(handshake_hash());

        ret = crypto_util_->HkdfExpand(static_secret_, label, FIXED_SECRET_SIZE, mSS);
        MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand static secret fail");

        label.assign(EXPAND_ES_LABLE);
        label.append(handshake_hash());

        ret = crypto_util_->HkdfExpand(ephemeral_secret_, label, FIXED_SECRET_SIZE, mES);
        MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand ephemeral secret fail");

        ret = crypto_util_->HkdfExtract(mSS, mES, master_secret_);
        MMTLS_CHECK_EQ(OK, ret, ret, "hkdf extract master secret fail");
        MMTLS_CHECK_EQ(FIXED_SECRET_SIZE, master_secret_.size(), ERR_UNEXPECT_CHECK_FAIL, "master secret size invalid");
    }

    critical_audit_.set_flag(CRITICAL_AUDIT_MASTER_SECRET);

    return OK;
}

int HandShakeState::ComputePskHmac(const mmtls::String& message, mmtls::String& mac_key, mmtls::String& mac_value) {
    MMTLS_CHECK_EQ(OK,
                   critical_audit_.CheckHas(CRITICAL_AUDIT_EPHEMERAL_SECRET),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "audit fail");

    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash for computing psk mac fail");

    mmtls::String label;
    ret = crypto_util_->GenerateRandom(FIXED_RANDOM_SIZE, label);
    MMTLS_CHECK_EQ(OK, ret, ret, "generate random fail");

    label += EXPAND_REFRESH_PSK_MAC_LABEL;
    label += handshake_hash();

    mac_key.clear();
    ret = crypto_util_->HkdfExpand(ephemeral_secret_, label, FIXED_SECRET_SIZE, mac_key);
    MMTLS_CHECK_EQ(OK, ret, ret, "hkdf derive psk mac key fail");
    MMTLS_CHECK_EQ(FIXED_SECRET_SIZE, mac_key.size(), ERR_UNEXPECT_CHECK_FAIL, "mac key size invalid");

    ret = crypto_util_->MessageAuthCode(mac_key, message, mac_value);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute mac value fail");
    MMTLS_CHECK_GT(mac_value.size(), 0U, ERR_UNEXPECT_CHECK_FAIL, "mac value is empty");

    return OK;
}

int HandShakeState::ComputeStageConnCipherState(ConnectionSide conn_side,
                                                bool is_our_side,
                                                ConnectionKeys& conn_keys,
                                                ConnectionCipherState*& cipher_state) {
    cipher_state = ConnectionCipherState::Create(protocol_version_, conn_side, is_our_side, cipher_suite_, conn_keys);

    return OK;
}

int HandShakeState::ComputeStageConnKey(const mmtls::String& secret,
                                        const mmtls::String& lable,
                                        ConnectionKeys*& conn_keys) {
    MMTLS_CHECK_TRUE(secret.size() > 0, ERR_UNEXPECT_CHECK_FAIL, "secret is empty");

    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash for derving connection key fail");

    MMTLSLOG_DEBUG("debug: compute conn keys: secret %s label %s handshake_hash %s",
                   SafeStrToHex(secret).c_str(),
                   lable.c_str(),
                   SafeStrToHex(handshake_hash()).c_str());

    mmtls::String expand_label(lable);
    expand_label += handshake_hash();

    if ((mode() == HS_MODE_ZERO_RTT_PSK)
        && (strcmp(lable.c_str(), CON_KEY_LABLE_EARLY_DATA) == 0
            || strcmp(lable.c_str(), CON_KEY_LABLE_HANDSHAKE) == 0)) {
        // In 0-RTT mode
        // We only need client_write_key/iv for early data encryption and server_write_key/iv for handshake encryption.
        // Here we set client_write_key/iv and server_write_key/iv to be the same to avoid empty key/iv.
        size_t len = cipher_suite_.cipher_key_len() + cipher_suite_.nonce_len();

        mmtls::String key_block;
        ret = crypto_util_->HkdfExpand(secret, expand_label, len, key_block);
        MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand connection key fail");
        MMTLS_CHECK_EQ(len, key_block.size(), ERR_UNEXPECT_CHECK_FAIL, "expanded key block size invalid");

        conn_keys = new ConnectionKeys(key_block.c_str(),
                                       cipher_suite_.cipher_key_len(),
                                       key_block.c_str(),
                                       cipher_suite_.cipher_key_len(),
                                       key_block.c_str() + cipher_suite_.cipher_key_len(),
                                       cipher_suite_.nonce_len(),
                                       key_block.c_str() + cipher_suite_.cipher_key_len(),
                                       cipher_suite_.nonce_len());

    } else {
        size_t len = 2 * (cipher_suite_.cipher_key_len() + cipher_suite_.nonce_len());

        mmtls::String key_block;
        ret = crypto_util_->HkdfExpand(secret, expand_label, len, key_block);
        MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand connection key fail");
        MMTLS_CHECK_EQ(len, key_block.size(), ERR_UNEXPECT_CHECK_FAIL, "expanded key block size invalid");

        conn_keys =
            new ConnectionKeys(key_block.c_str(),
                               cipher_suite_.cipher_key_len(),
                               key_block.c_str() + cipher_suite_.cipher_key_len(),
                               cipher_suite_.cipher_key_len(),
                               key_block.c_str() + 2 * cipher_suite_.cipher_key_len(),
                               cipher_suite_.nonce_len(),
                               key_block.c_str() + 2 * cipher_suite_.cipher_key_len() + cipher_suite_.nonce_len(),
                               cipher_suite_.nonce_len());
    }

    return OK;
}

int HandShakeState::ComputeVerifyData(ConnectionSide side, mmtls::String& verify_data) {
    // See https://tlswg.github.io/tls13-spec/#finished

    MMTLS_CHECK_EQ(OK, critical_audit_.CheckHas(CRITICAL_AUDIT_STATIC_SECRET), ERR_UNEXPECT_CHECK_FAIL, "audit fail");

    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash for computing finished verify data fail");

    mmtls::String finished_secret;

    ret = crypto_util_->HkdfExpand(static_secret_,
                                   (side == CONN_SIDE_CLIENT ? "client finished" : "server finished"),
                                   FIXED_SECRET_SIZE,
                                   finished_secret);
    MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand finished secret");
    MMTLS_CHECK_EQ(FIXED_SECRET_SIZE, finished_secret.size(), ERR_UNEXPECT_CHECK_FAIL, "finished secret size invalid");

    ret = crypto_util_->MessageAuthCode(finished_secret, handshake_hash(), verify_data);
    MMTLS_CHECK_EQ(OK, ret, ret, "calculate message auth code");
    MMTLS_CHECK_GT(verify_data.size(), 0U, ERR_UNEXPECT_CHECK_FAIL, "finished verify data is empty");

    return OK;
}

int HandShakeState::SignCertificate(const mmtls::String& pri_key, mmtls::String& signature) {
    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash for computing finished verify data fail");

    ret = crypto_util_->SignMessage(pri_key, handshake_hash(), signature);
    MMTLS_CHECK_EQ(OK, ret, ret, "sign certificate fail");
    return OK;
}

int HandShakeState::VerifyCertificate(const mmtls::String& pub_key, const mmtls::String& signature) {
    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash for computing finished verify data fail");

    ret = crypto_util_->VerifyMessage(pub_key, signature, handshake_hash());
    MMTLS_CHECK_EQ(OK, ret, ret, "verify certificate fail");
    return OK;
}

int HandShakeState::ComputeResumptionSecret(PskType type, mmtls::String& resumption_secret) {
    // See https://tlswg.github.io/tls13-spec/#key-schedule
    // We send NewSessionTicket before ServerFinished, so we use ephemeral secret to derive resumption secret.

    MMTLS_CHECK_EQ(OK,
                   critical_audit_.CheckHas(CRITICAL_AUDIT_EPHEMERAL_SECRET),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "audit fail");

    int ret = UpdateHandShakeHash();
    MMTLS_CHECK_EQ(OK, ret, ret, "update handshake hash for computing resumption secret fail");

    mmtls::String label(type == PSK_ACCESS ? EXPAND_ACCESS_PSK_SECRET_LABEL : EXPAND_REFRESH_PSK_SECRET_LABEL);
    label += handshake_hash();

    ret = crypto_util_->HkdfExpand(ephemeral_secret_, label, FIXED_SECRET_SIZE, resumption_secret);
    MMTLS_CHECK_EQ(OK, ret, ret, "hkdf expand resumption secret fail");
    MMTLS_CHECK_EQ(FIXED_SECRET_SIZE,
                   resumption_secret.size(),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "resumption secret size invalid");

    MMTLSLOG_DEBUG(
        "debug: compute resumption_secret, psk type %d, ephemeral_secret %s, label %s, handshake_hash %s, "
        "resumption_secret %s",
        type,
        SafeStrToHex(ephemeral_secret_).c_str(),
        SafeStrToHex(label).c_str(),
        SafeStrToHex(handshake_hash()).c_str(),
        SafeStrToHex(resumption_secret).c_str());

    return OK;
}

int HandShakeState::ComputeFallBackUrlSignature(const mmtls::String& pri_key,
                                                const mmtls::String& url,
                                                mmtls::String& signature) {
    mmtls::String message(FALLBACK_NO_MMTLS);
    message += url;

    int ret = crypto_util_->SignMessage(pri_key, message, signature);
    MMTLS_CHECK_EQ(OK, ret, ret, "sign url fail, url %s", url.c_str());

    return OK;
}

int HandShakeState::VerifyFallBackUrlSignature(const mmtls::String& pub_key,
                                               const mmtls::String& url,
                                               const mmtls::String& signature) {
    mmtls::String message(FALLBACK_NO_MMTLS);
    message += url;

    int ret = crypto_util_->VerifyMessage(pub_key, signature, message);
    MMTLS_CHECK_EQ(OK, ret, ret, "verify url fail, url %s, sign %d", url.c_str(), (int)signature.size());

    return OK;
}

void HandShakeState::attach_crypto_util(CryptoUtil* crypto_util) {
    if (crypto_util_) {
        delete crypto_util_;
        crypto_util_ = NULL;
    }
    crypto_util_ = crypto_util;
    if (!handshake_hash_obj_.get()) {
        handshake_hash_obj_.reset(crypto_util_->CreateHash());
    }
}

void HandShakeState::SetNextStage(HandShakeStage stage, bool is_next_handshake_loop) {
    stage_ = stage;
    is_next_handshake_loop_ = is_next_handshake_loop;
    MMTLSLOG_DEBUG("set stage %u, is next loop %u", stage_, is_next_handshake_loop_);
}

void HandShakeState::ActiveSession() {
    // Clear secrets.
    static_secret_.safe_clear();
    ephemeral_secret_.safe_clear();
    master_secret_.safe_clear();

    // Clear handshake messages.
    handshake_bytes_.clear();
    handshake_hash_.clear();
    ClearHandshakeMessages();

    active_time_ = NowUTime();
}

void HandShakeState::ClearHandshakeMessages() {
    for (HandShakeMessages::iterator it = handshake_messages_.begin(); it != handshake_messages_.end(); ++it) {
        if (it->msg != NULL) {
            delete it->msg;
            it->msg = NULL;
        }
    }
    handshake_messages_.clear();
}

void HandShakeState::set_cipher_suite(uint16 ciphersuite_code) {
    cipher_suite_ = CipherSuite::GetByCode(ciphersuite_code);

    CryptoUtil* crypt_ = new OpenSslCryptoUtil(cipher_suite_);
    attach_crypto_util(crypt_);
}

void HandShakeState::Close() {
    close_time_ = NowUTime();
}

void HandShakeState::set_client_id(const mmtls::String& random) {
    size_t max_len = (random.size() > 5 ? 5 : random.size());
    if (max_len) {
        client_id_ = mmtls::StrToHex(random.substr(0, max_len));
    }
}

}  // namespace mmtls
