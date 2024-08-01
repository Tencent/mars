#include "mmtls_client_channel.h"

#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#include "mmtls_alert.h"
#include "mmtls_client_channel_processor.h"
#include "mmtls_client_credential_manager.h"
#include "mmtls_client_handshake_state.h"
#include "mmtls_constants.h"
#include "mmtls_event_callback.h"
#include "mmtls_key_pair.h"
#include "mmtls_log.h"
#include "mmtls_openssl_crypto_util.h"
#include "mmtls_psk.h"
#include "mmtls_record_head.h"
#include "mmtls_record_reader.h"
#include "mmtls_record_writer.h"
#include "mmtls_version.h"

namespace mmtls {

ClientChannel::~ClientChannel() {
    FinalReport();
}

int ClientChannel::Init(const mmtls::String* app_data, bool is_long_connection) {
    MMTLS_CHECK_TRUE(!(is_long_connection && early_app_data_ != NULL),
                     ERR_UNEXPECT_CHECK_FAIL,
                     "0-RTT mode can't be used in long connection");

    // Init no encryption connection state.
    write_connection_cipher_states_[CON_KEY_NONE] = NULL;
    read_connection_cipher_states_[CON_KEY_NONE] = NULL;

    // The early_app_data_ pointer will be invalid after Init returned.
    early_app_data_ = app_data;
    is_long_connection_ = is_long_connection;

    if (app_data != NULL && app_data->size() > policy_.MaxAppDataSize()) {
        MMTLSLOG_ERR("early application data size exceed limit, size %zu, limit %zu",
                     app_data->size(),
                     policy_.MaxAppDataSize());
        return ERR_APP_DATA_EXCEED_LIMIT;
    }

    // Next to initialize the client channel.
    state_.SetNextStage(STAGE_INIT, true);

    int ret = DoHandShakeLoop(NULL, NULL);
    if (OK != ret) {
        MMTLSLOG_ERR("initialize client channel fail, ret [%d]", ret);
        // All initialization failure are internal error.
        return DoHandleError(ALERT_NONE, ret);
    }

    ret = AuditFor1stSendToPeer();
    MMTLS_CHECK_EQ(OK, ret, DoHandleError(ALERT_NONE, ret), "audit for 1st send to peer fail");

    // Send all data in send_buffer.
    SendToPeer();

    MMTLS_CHECK_FALSE((state_.mode() != HS_MODE_ZERO_RTT_PSK && app_data != NULL),
                      ERR_APP_DATA_NOT_SENT,
                      "early app data is not sent");
    return OK;
}

int ClientChannel::Send(const mmtls::String& bytes) {
    if (IsClosed()) {
        MMTLSLOG_IMPT("client tls channel is closed. can't send data any more!");
        return ERR_TLS_CLOSED;
    }
    if (!is_long_connection_) {
        MMTLSLOG_IMPT("data should be sent via Init in short connection");
        return ERR_APP_DATA_NOT_SENT;
    }
    if (!IsActive()) {
        MMTLSLOG_ERR("handshake is not completed, application data can't be sent in stage [%d]", state_.stage());
        return ERR_APP_DATA_NOT_SENT;
    }
    if (bytes.size() > policy_.MaxAppDataSize()) {
        MMTLSLOG_ERR("application data size exceed limit, size %zu, limit %zu", bytes.size(), policy_.MaxAppDataSize());
        return ERR_APP_DATA_EXCEED_LIMIT;
    }

    int ret = BuildWriteBuffer(RECORD_APPLICATION_DATA, bytes);
    if (OK != ret) {
        Close();
        MMTLSLOG_ERR("write application data to send buffer fail. stage [%d] ret [%d]", state_.stage(), ret);
        return DoHandleError(ALERT_NONE, ERR_PROCESS_APP_DATA_FAIL);
    }

    // Send all data in send_buffer.
    SendToPeer();

    return OK;
}

int ClientChannel::SendHeartbeat(const mmtls::String& payload) {
    if (IsClosed()) {
        MMTLSLOG_IMPT("client tls channel is closed. can't send heartbeat any more!");
        return ERR_TLS_CLOSED;
    }
    if (!is_long_connection_) {
        MMTLSLOG_IMPT("heartbeat should be sent in long connection");
        return ERR_HEARTBEAT_NOT_SENT;
    }
    if (!IsActive()) {
        MMTLSLOG_ERR("handshake is not completed, heartbeat can't be sent in stage [%d]", state_.stage());
        return ERR_HEARTBEAT_NOT_SENT;
    }

    mmtls::String bytes;
    int ret = processor_.CreateClientHeartbeat(state_, payload, &bytes);
    MMTLS_CHECK_EQ(OK, ret, ret, "create client heartbeat fail");

    ret = BuildWriteBuffer(RECORD_HEARTBEAT, bytes);
    if (OK != ret) {
        Close();
        MMTLSLOG_ERR("write heartbeat data to send buffer fail. stage [%d] ret [%d]", state_.stage(), ret);
        return DoHandleError(ALERT_NONE, ERR_PROCESS_HEARTBEAT_FAIL);
    }

    SendToPeer();

    return OK;
}

int ClientChannel::Receive(const char* buf, size_t size, size_t& size_hint) {
    MMTLS_CHECK_TRUE(buf != NULL, ERR_ILLEGAL_PARAM, "buf is null");

    if (IsClosed()) {
        MMTLSLOG_IMPT("client tls channel is closed. can't receive data any more!");
        return ERR_TLS_CLOSED;
    }

    size_hint = 0;

    record_reader_->SetReceivedData(buf, size, seq_numbers_);
    ConnectionCipherState* conn_cipher_state = NULL;
    const byte* payload = NULL;

    while (!record_reader_->IsReadToEnd()) {
        int ret = SelectConnectionCipherStateForReader(conn_cipher_state);
        MMTLS_CHECK_EQ(OK, ret, ERR_INTERNAL_ERROR, "select cipher state for record reader fail");

        RecordHead record;
        mmtls::String plain_text;
        ret = record_reader_->GetNextRecord(record, plain_text, payload, conn_cipher_state);

        if (OK == ret) {
            // Read a record. Handle it.
            ret = DoReceiveRecord(record, plain_text, payload);
            if (OK != ret) {
                MMTLSLOG_ERR("received a record but process it fail. stage [%d] ret [%d]", state_.stage(), ret);
                Close();
                return DoHandleError(ALERT_NONE, ret);
            }
        } else if (ret > 0) {
            // Need more bytes.
            MMTLSLOG_DEBUG("debug: need %d bytes to read a record. stage [%d]", ret, state_.stage());
            size_hint = ret;
        } else {
            // Error occur.
            MMTLSLOG_ERR("read record fail. stage [%d], ret [%d]", state_.stage(), ret);
            Close();
            return ERR_INVALID_RECORD;
        }
    }

    // Send all data in send_buffer.
    SendToPeer();

    return OK;
}

int ClientChannel::DoReceiveRecord(const RecordHead& record, mmtls::String& plain_text, const byte* /*payload*/) {
    UpdateReceiveStat(state_.stage(), record);

    MMTLS_CHECK_TRUE(policy_.AcceptProtocolVersion(record.GetProtocolVersion()),
                     ERR_INVALID_RECORD,
                     "record version is not accepted, version 0x%x",
                     record.GetProtocolVersion().version());

    int ret = OK;
    DataReader reader(plain_text);

    Alert alert;
    HandShakeMessage* msg = NULL;
    AutoPointHolder<HandShakeMessage> holder(msg);

    switch (record.GetType()) {
        case RECORD_ALERT:
            // Deserialize the alert.
            ret = alert.Deserialize(reader);
            MMTLS_CHECK_EQ(OK, ret, ERR_INVALID_ALERT, "deserialize received alert fail");
            ret = DoReceiveAlert(alert);
            MMTLSLOG_DEBUG("debug: receive alert ret %d", ret);
            return ret;

        case RECORD_EARLY_HANDSHAKE:
            MMTLSLOG_ERR("early handshake record is not accepted by client");
            return ERR_INVALID_RECORD;

        case RECORD_HANDSHAKE:
            // Deserialize the message.
            ret = state_.DeserializeMsg(reader, &msg);
            MMTLS_CHECK_EQ(OK, ret, ERR_INVALID_HANDSHAKE_MESSAGE, "deserialize handshake message fail");
            MMTLS_CHECK_TRUE(msg != NULL, ERR_INVALID_HANDSHAKE_MESSAGE, "handshake message is null");
            MMTLS_CHECK_TRUE(IsRecvMsgExpected(msg),
                             ERR_INVALID_HANDSHAKE_MESSAGE,
                             "received unexpected handshake message");
            holder.Disable();

            // Process it.
            ret = DoHandShakeLoop(&plain_text, msg);
            MMTLS_CHECK_EQ(OK, ret, ret, "process handshake loop with received handshake message fail");

            return OK;

        case RECORD_APPLICATION_DATA:
            if (state_.stage() != STAGE_COMPLETED && state_.stage() != STAGE_RECV_EARLY_APP_DATA) {
                MMTLSLOG_ERR("can't received application in stage [%d]", state_.stage());
                return ERR_INVALID_RECORD;
            }

            if (STAGE_RECV_EARLY_APP_DATA == state_.stage()) {
                ret = AuditForReceiveEarlyData();
                MMTLS_CHECK_EQ(OK, ret, ret, "audit for early data fail");
            }

            event_callback_.OnReceive(plain_text, NULL);
            return OK;

        case RECORD_HEARTBEAT:
            MMTLSLOG_ERR("recieve heartbeat response");
            return OK;

        default:
            MMTLSLOG_ERR("err: unknown record type, type [%d]", record.GetType());
            return ERR_INVALID_RECORD;
    }
}

int ClientChannel::DoReceiveAlert(const Alert& alert) {
    MMTLSLOG_DEBUG("receive an alert, level %d, type %d", alert.Level(), alert.Type());
    event_callback_.OnAlert(alert.Type());

    if (alert.Level() == ALERT_LEVEL_WARNING) {
        if (alert.Type() == ALERT_END_OF_EARLY_DATA) {
            MMTLS_CHECK_EQ(STAGE_RECV_EARLY_APP_DATA,
                           state_.stage(),
                           ERR_INVALID_ALERT,
                           "receive end of early data alert in wrong stage, stage %d",
                           state_.stage());

            int ret = DoHandShakeLoop(NULL, NULL);
            MMTLS_CHECK_EQ(OK, ret, ret, "process handshake loop with end of early data alert fail");
        }
        // For unknown warning alerts, ignore them.
        return OK;
    } else if (alert.Level() == ALERT_LEVEL_FATAL) {
        if (alert.Type() == ALERT_UNKNOWN_PSK_IDENTITY) {
            // Delete access psk if receive unknown psk identify. Refresh psk will never be deleted.
            if (state_.mode() == HS_MODE_ONE_RTT_PSK || state_.mode() == HS_MODE_ZERO_RTT_PSK) {
                int ret = credential_manager_.DeletePsk(PSK_ACCESS);
                stat_.add_delete_access_psk_cnt(OK == ret);
                MMTLSLOG_DEBUG("debug: delete access psk, ret %d", ret);
            }
        } else if (alert.Type() == ALERT_FALLBACK_NO_MMTLS) {
            // check if fallback to no mmtls;
            if (alert.HasFallBackUrl(server_url_)) {
                const uint32 cert_version = alert.CertVersion();
                int ret = state_.set_ecdsa_key(cert_version);
                MMTLS_CHECK_EQ(OK, ret, ret, "set ecdsa key failed, version %u", cert_version);

                const mmtls::String& urls = alert.GetFallBackUrls();
                ret = state_.VerifyFallBackUrlSignature(state_.ecdsa_key().public_key(),
                                                        urls,
                                                        alert.GetFallBackUrlsSignature());
                MMTLS_CHECK_EQ(OK, ret, ret, "verify failed, version %u urls %s", cert_version, urls.c_str());

                MMTLSLOG_ERR("recevie fallback no mmtls alert and verify succ, set no mmtls, %u", alert.CertRegion());
                stat_.set_no_mmtls(true);
                state_.set_cert_region(alert.CertRegion());
                state_.set_fallback_urls_vec(alert.GetFallbackUrlsVec());
            }
        }
        return ERR_RECEIVE_FATAL_ALERT;
    } else {
        MMTLSLOG_ERR("alert level is invalid");
        return ERR_INVALID_ALERT;
    }
}

int ClientChannel::WriteMsgToSendBuffer(RecordType type, HandShakeMessage* msg) {
    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "msg is null");
    AutoPointHolder<HandShakeMessage> holder(msg);

    DataWriter writer;
    int ret = msg->Serialize(writer);
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize handshake message fail");

    ret = state_.Add(msg, writer.Buffer(), CONN_SIDE_CLIENT);
    MMTLS_CHECK_EQ(OK, ret, ret, "add handshake message to handshake state fail");
    holder.Disable();

    ret = BuildWriteBuffer(type, writer.Buffer());
    MMTLS_CHECK_EQ(OK, ret, ret, "write serialized handshake message to send buffer fail");

    return OK;
}

bool ClientChannel::IsRecvMsgExpected(HandShakeMessage* msg) {
    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "msg is null");
    switch (state_.stage()) {
        case STAGE_RECV_SERVER_HELLO:
            return msg->GetType() == SERVER_HELLO;
        case STAGE_RECV_CERTIFICATE_VERIFY:
            return msg->GetType() == CERTIFICATE_VERIFY;
        case STAGE_RECV_NEW_SESSION_TICKET:
            return msg->GetType() == NEW_SESSION_TICKET;
        case STAGE_RECV_SERVER_FINISHED:
            return msg->GetType() == FINISHED;
        default:
            return false;
    }
}

int ClientChannel::DoHandShakeLoop(mmtls::String* plain_text, HandShakeMessage* msg) {
    if (state_.stage() != STAGE_INIT) {
        MMTLS_CHECK_TRUE(state_.mode() == HS_MODE_ONE_RTT_PSK || state_.mode() == HS_MODE_ONE_RTT_ECDH
                             || state_.mode() == HS_MODE_ZERO_RTT_PSK,
                         ERR_UNEXPECT_CHECK_FAIL,
                         "handshake mode is not set, can't do handshake loop");
    }

    int ret = OK;

    do {
        state_.stage_audit().set_flag(state_.stage());
        // MMTLSLOG_DEBUG("intset 0x%x stage %u\n",state_.stage_audit().flags(),state_.stage());
        switch (state_.stage()) {
            case STAGE_INIT:
                ret = DoInit();
                MMTLS_CHECK_EQ(OK, ret, ERR_INTERNAL_ERROR, "init fail");
                break;

            case STAGE_SEND_CLIENT_HELLO:
                ret = DoSendClientHello();
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "send client hello fail");
                break;

            case STAGE_SEND_ENCRYPTED_EXTENSIONS:
                ret = DoSendEncryptedExtension();
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "send encrypted extensions fail");
                break;

            case STAGE_SEND_EARLY_APP_DATA:
                ret = DoSendEarlyAppData();
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_EARLY_DATA_FAIL, "send early app data fail");
                break;

            case STAGE_RECV_SERVER_HELLO:
                MMTLS_CHECK_TRUE(plain_text, ERR_UNEXPECT_CHECK_FAIL, "plain text null");
                ret = DoReceiveServerHello(*plain_text, msg);
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "process received server hello fail");
                break;

            case STAGE_RECV_CERTIFICATE_VERIFY:
                MMTLS_CHECK_TRUE(plain_text, ERR_UNEXPECT_CHECK_FAIL, "plain text null");
                ret = DoReceiveCertificateVerify(*plain_text, msg);
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "process received certificate verify fail");
                break;

            case STAGE_RECV_NEW_SESSION_TICKET:
                MMTLS_CHECK_TRUE(plain_text, ERR_UNEXPECT_CHECK_FAIL, "plain text null");
                ret = DoReceiveNewSessionTicket(*plain_text, msg);
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "process received new session ticket fail");
                break;

            case STAGE_RECV_SERVER_FINISHED:
                MMTLS_CHECK_TRUE(plain_text, ERR_UNEXPECT_CHECK_FAIL, "plain text null");
                ret = DoReceiveServerFinished(*plain_text, msg);
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "process received server finished fail");
                break;

            case STAGE_RECV_EARLY_APP_DATA:
                ret = DoReceiveEarlyAppData();
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_EARLY_DATA_FAIL, "process received early app data fail");
                break;

            case STAGE_SEND_CLIENT_FINISHED:
                ret = DoSendClientFinished();
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "send client finished fail");
                break;

            case STAGE_COMPLETED:
                ret = DoComplete();
                MMTLS_CHECK_EQ(OK, ret, ERR_PROCESS_HANDSHAKE_FAIL, "compelete handshake fail");
                break;

            default:
                MMTLSLOG_ERR("handshake stage invalid, stage [%d]", state_.stage());
                return ERR_UNEXPECT_CHECK_FAIL;
        }
    } while (state_.IsNextHandShakeLoop());

    return OK;
}

int ClientChannel::DoHandleError(AlertType alert_type, int error_code) {
    if (-100 < error_code)
        return error_code;
    else
        return ERR_INTERNAL_ERROR;
}

int ClientChannel::DoInit() {
    // Set protocol version.
    state_.set_protocol_version(ProtocolVersion::LatestVersion());

    // Try to get access psk from credential manager.
    ClientPsk* psk = NULL;
    int ret = credential_manager_.GetValidPsk(PSK_ACCESS, &psk);
    if (OK == ret) {
        stat_.add_access_psk_cnt(1);
        state_.set_access_psk(psk);
    } else {
        stat_.inc_get_access_psk_fail();
        MMTLSLOG_ERR("get access psk fail, ret [%d]", ret);
    }

    // Check if there is 0-RTT data and can be sent.
    send_early_app_data_ = early_app_data_ != NULL && state_.access_psk() != NULL;

    // Choose handshake mode.
    if (state_.access_psk() != NULL) {
        state_.set_cipher_suite(CipherSuite::TLS_PSK_WITH_AES_128_GCM_SHA256);
        if (send_early_app_data_) {
            state_.SetMode(HS_MODE_ZERO_RTT_PSK);
        } else {
            state_.SetMode(HS_MODE_ONE_RTT_PSK);
        }
    } else {
        state_.set_cipher_suite(CipherSuite::TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
        state_.SetMode(HS_MODE_ONE_RTT_ECDH);

        // Get static keys.
        std::vector<KeyPair> static_server_ecdh_keys;
        std::vector<KeyPair> ecdsa_keys;

        int ret = credential_manager_.GetStaticEcdhKeyPairs(static_server_ecdh_keys);
        MMTLS_CHECK_EQ(OK, ret, ret, "get static ecdh key pairs");
        MMTLS_CHECK_GE(static_server_ecdh_keys.size(),
                       1U,
                       ERR_UNEXPECT_CHECK_FAIL,
                       "static server ecdh key count is 0");

        ret = credential_manager_.GetEcdsaKeyPairs(ecdsa_keys);
        MMTLS_CHECK_EQ(OK, ret, ret, "get ecdsa key paris fail");
        MMTLS_CHECK_GE(ecdsa_keys.size(), 1U, ERR_UNEXPECT_CHECK_FAIL, "ecdsa key count is 0");

        state_.set_static_server_ecdh_keys(static_server_ecdh_keys);
        // state_.set_ecdsa_key(ecdsa_keys[0]);
        state_.set_ecdsa_keys(ecdsa_keys);

        // Try to get refresh psk from credential manager.
        ClientPsk* psk = NULL;
        ret = credential_manager_.GetValidPsk(PSK_REFRESH, &psk);
        if (OK == ret) {
            stat_.add_refresh_psk_cnt(1);
            state_.set_refresh_psk(psk);
        } else {
            stat_.inc_get_refresh_psk_fail();
            MMTLSLOG_ERR("get refresh psk fail, ret [%d]", ret);
        }
    }

    MMTLSLOG_DEBUG("debug: client channel init succ, handshake mode %d, ciphersuite %s",
                   state_.mode(),
                   state_.cipher_suite().ToString().c_str());

    // Next to send ClientHello.
    state_.SetNextStage(STAGE_SEND_CLIENT_HELLO, true);

    return OK;
}

int ClientChannel::DoSendClientHello() {
    // Create client hello.
    ClientHello* client_hello = NULL;
    int ret = processor_.CreateClientHello(state_, client_hello);
    MMTLS_CHECK_EQ(OK, ret, ret, "create client hello fail");
    MMTLS_CHECK_TRUE(client_hello, ERR_UNEXPECT_CHECK_FAIL, "client hello is null");

    ret = WriteMsgToSendBuffer(HS_MODE_ZERO_RTT_PSK == state_.mode() ? RECORD_EARLY_HANDSHAKE : RECORD_HANDSHAKE,
                               client_hello);
    MMTLS_CHECK_EQ(OK, ret, ret, "write client hello to send buffer fail");

    // Move to next state.
    switch (state_.mode()) {
        // Next to wait for ServerHello.
        case HS_MODE_ONE_RTT_ECDH:
        case HS_MODE_ONE_RTT_PSK:
            // Next to wait for ServerHello.
            state_.SetNextStage(STAGE_RECV_SERVER_HELLO, false);
            break;

        case HS_MODE_ZERO_RTT_PSK:
            // Compute early handshake cipher state.
            ret = ComputeStageConnCipherState(CONN_SIDE_CLIENT,
                                              CON_KEY_EARLY_DATA,
                                              state_.static_secret(),
                                              CON_KEY_LABLE_EARLY_DATA);
            MMTLS_CHECK_EQ(OK, ret, ret, "compute cipher state for early handshake fail");

            // Next to send EncryptedExtensions.
            state_.SetNextStage(STAGE_SEND_ENCRYPTED_EXTENSIONS, true);
            break;
    }

    return OK;
}

int ClientChannel::DoSendEncryptedExtension() {
    MMTLS_CHECK_EQ(HS_MODE_ZERO_RTT_PSK,
                   state_.mode(),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "send encrypted extensions in wrong handshake mode");

    // Create encrypted extensions.
    EncryptedExtensions* encrypted_extensions = NULL;
    int ret = processor_.CreateEncryptedExtensions(state_, encrypted_extensions);
    MMTLS_CHECK_EQ(OK, ret, ret, "create encrypted extensions fail");
    MMTLS_CHECK_TRUE(encrypted_extensions, ERR_UNEXPECT_CHECK_FAIL, "encrypted extensions is null");

    ret = WriteMsgToSendBuffer(RECORD_EARLY_HANDSHAKE, encrypted_extensions);
    MMTLS_CHECK_EQ(OK, ret, ret, "write encrypted extensions to send buffer fail");

    // Next to send 0-RTT application data.
    state_.SetNextStage(STAGE_SEND_EARLY_APP_DATA, true);

    return OK;
}

int ClientChannel::DoSendEarlyAppData() {
    MMTLS_CHECK_TRUE(send_early_app_data_, ERR_UNEXPECT_CHECK_FAIL, "no early application data to send");
    MMTLS_CHECK_TRUE(early_app_data_ != NULL, ERR_UNEXPECT_CHECK_FAIL, "early app data is null");
    MMTLS_CHECK_EQ(HS_MODE_ZERO_RTT_PSK,
                   state_.mode(),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "send early application data in wrong handshake mode");

    int ret = BuildWriteBuffer(RECORD_APPLICATION_DATA, *early_app_data_);
    MMTLS_CHECK_EQ(OK, ret, ret, "write early application data to send buffer fail");

    ret = BuildAlertBuffer(ALERT_LEVEL_WARNING, ALERT_END_OF_EARLY_DATA);
    MMTLS_CHECK_EQ(OK, ret, ret, "write end of early data alert to send buffer fail");

    // Next to receive server finished.
    state_.SetNextStage(STAGE_RECV_SERVER_HELLO, false);

    return OK;
}

int ClientChannel::DoReceiveServerHello(const mmtls::String& plain_text, HandShakeMessage* msg) {
    AutoPointHolder<HandShakeMessage> holder(msg);

    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "server hello is null");
    int ret = processor_.ProcessServerHello(state_, dynamic_cast<ServerHello*>(msg));
    MMTLS_CHECK_EQ(OK, ret, ret, "process received server hello fail");

    // Add the message.
    ret = state_.Add(msg, plain_text, CONN_SIDE_SERVER);
    MMTLS_CHECK_EQ(OK, ret, ret, "add handshake message to handshake state fail");
    holder.Disable();

    // Compute handshake cipher state.
    ret = ComputeStageConnCipherState(CONN_SIDE_CLIENT,
                                      CON_KEY_HANDSHAKE,
                                      state_.ephemeral_secret(),
                                      CON_KEY_LABLE_HANDSHAKE);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute cipher state for handshake fail");

    // Move to next state.
    if (HS_MODE_ONE_RTT_ECDH == state_.mode()) {
        // Next to receive certificate verify.
        state_.SetNextStage(STAGE_RECV_CERTIFICATE_VERIFY, false);
    } else if (HS_MODE_ZERO_RTT_PSK == state_.mode() || HS_MODE_ONE_RTT_PSK == state_.mode()) {
        // Next to receive server finished.
        state_.SetNextStage(STAGE_RECV_SERVER_FINISHED, false);
    }

    return OK;
}

int ClientChannel::DoReceiveCertificateVerify(const mmtls::String& plain_text, HandShakeMessage* msg) {
    AutoPointHolder<HandShakeMessage> holder(msg);

    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "certificate verify is null");
    int ret = processor_.ProcessCertificateVerify(state_, dynamic_cast<CertificateVerify*>(msg));
    MMTLS_CHECK_EQ(OK, ret, ret, "process received certificate verify fail");

    // Add the message.
    ret = state_.Add(msg, plain_text, CONN_SIDE_SERVER);
    MMTLS_CHECK_EQ(OK, ret, ret, "add handshake message to handshake state fail");
    holder.Disable();

    // Next to receive new session ticket.
    state_.SetNextStage(STAGE_RECV_NEW_SESSION_TICKET, false);

    return OK;
}

int ClientChannel::DoReceiveNewSessionTicket(const mmtls::String& plain_text, HandShakeMessage* msg) {
    AutoPointHolder<HandShakeMessage> holder(msg);

    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "new session ticket is null");
    int ret = processor_.ProcessNewSessionTicket(state_, dynamic_cast<NewSessionTicket*>(msg));
    MMTLS_CHECK_EQ(OK, ret, ret, "process received new session ticket fail");

    // Add the message.
    ret = state_.Add(msg, plain_text, CONN_SIDE_SERVER);
    MMTLS_CHECK_EQ(OK, ret, ret, "add handshake message to handshake state fail");
    holder.Disable();

    // Move to next stage.
    state_.SetNextStage(STAGE_RECV_SERVER_FINISHED, false);
    return OK;
}

int ClientChannel::DoReceiveServerFinished(const mmtls::String& plain_text, HandShakeMessage* msg) {
    AutoPointHolder<HandShakeMessage> holder(msg);

    MMTLS_CHECK_TRUE(msg != NULL, ERR_UNEXPECT_CHECK_FAIL, "server finished is null");
    int ret = processor_.ProcessServerFinished(state_, dynamic_cast<Finished*>(msg));
    MMTLS_CHECK_EQ(OK, ret, ret, "process received server finished fail");

    // Add the message.
    ret = state_.Add(msg, plain_text, CONN_SIDE_SERVER);
    MMTLS_CHECK_EQ(OK, ret, ret, "add handshake message to handshake state fail");
    holder.Disable();

    if (HS_MODE_ONE_RTT_ECDH == state_.mode()) {
        // Psks should be saved after server finished was verified.
        ret = DoSavePsks();
        MMTLS_CHECK_EQ(OK, ret, ret, "save psks from new session ticket fail");
    }

    // Move to next state.
    if (HS_MODE_ZERO_RTT_PSK == state_.mode()) {
        // Next to receive early app data.
        // TODO: Should server always return ealy app data when in 0-RTT mode?
        state_.SetNextStage(STAGE_RECV_EARLY_APP_DATA, false);
    } else if (HS_MODE_ONE_RTT_ECDH == state_.mode() || HS_MODE_ONE_RTT_PSK == state_.mode()) {
        if (is_long_connection_) {
            // Next to send client finished.
            state_.SetNextStage(STAGE_SEND_CLIENT_FINISHED, true);
        } else {
            // No more app data to send. Just complete it.
            state_.SetNextStage(STAGE_COMPLETED, true);
        }
    }

    return OK;
}

int ClientChannel::DoSavePsks() {
    if (state_.new_access_psk() != NULL) {
        int ret = credential_manager_.SavePsk(*state_.new_access_psk());
        stat_.add_new_access_psk_cnt(ret == OK);
        if (OK != ret) {
            MMTLSLOG_ERR("save new access psk fail, ret %d", ret);
        }
    }
    if (state_.new_refresh_psk() != NULL) {
        int ret = credential_manager_.SavePsk(*state_.new_refresh_psk());
        stat_.add_new_refresh_psk_cnt(ret == OK);
        if (OK != ret) {
            MMTLSLOG_ERR("save new refresh psk fail, ret %d", ret);
        }
    }

    return OK;
}

int ClientChannel::DoReceiveEarlyAppData() {
    // Move to next state.
    // No more app data to send. Just complete it.
    state_.SetNextStage(STAGE_COMPLETED, true);

    return OK;
}

int ClientChannel::DoSendClientFinished() {
    // Create client finished.
    Finished* client_finished = NULL;
    int ret = processor_.CreateClientFinished(state_, client_finished);
    MMTLS_CHECK_EQ(OK, ret, ret, "create client finished fail");
    MMTLS_CHECK_TRUE(client_finished, ERR_UNEXPECT_CHECK_FAIL, "client finished is null");

    ret = WriteMsgToSendBuffer(RECORD_HANDSHAKE, client_finished);
    MMTLS_CHECK_EQ(OK, ret, ret, "write client finished to send buffer fail");

    // Compute master secret.
    ret = state_.ComputeMasterSecret();
    MMTLS_CHECK_EQ(OK, ret, ret, "compute master secret fail");

    // Compute application data cipher state.
    ret = ComputeStageConnCipherState(CONN_SIDE_CLIENT,
                                      CON_KEY_APPLICATION,
                                      state_.master_secret(),
                                      CON_KEY_LABLE_APPLICATION);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute application data cipher state fail");

    // Next to complete the handshake.
    state_.SetNextStage(STAGE_COMPLETED, true);

    return OK;
}

int ClientChannel::DoComplete() {
    int ret = AduitForActiveSession();
    MMTLS_CHECK_EQ(OK, ret, ret, "audit for active session fail");

    ActiveSession();

    // Mark the handshake as completed.
    state_.SetNextStage(STAGE_COMPLETED, false);

    // Report handshake completed.
    EventCallback::ExportState export_state(state_.stage());
    MMTLSLOG_IMPT("DoComplete found cert region %u", state_.cert_region());

    if (state_.has_cert_region()) {
        export_state.set_cert_region(state_.cert_region());
    }

    event_callback_.OnHandShake(export_state);

    return OK;
}

void ClientChannel::FinalReport() {
    MMTLSLOG_DEBUG("sumary id %s time %" PRIu64 " %" PRIu64 " %" PRIu64
                   " snd %zu %zu %zu %zu %zu %zu recv %zu %zu %zu %zu %zu status %zu %u %u %u",
                   state_.client_id(),
                   stat_.init_time(),
                   stat_.active_time(),
                   stat_.close_time(),

                   stat_.send_data_cnt(),
                   stat_.send_data_size(),
                   stat_.send_handshake_data_cnt(),
                   stat_.send_handshake_data_size(),
                   stat_.send_early_data_cnt(),
                   stat_.send_early_data_size(),

                   stat_.receive_data_cnt(),
                   stat_.receive_data_size(),
                   stat_.receive_handshake_data_cnt(),
                   stat_.receive_handshake_data_size(),
                   stat_.receive_early_data_cnt(),
                   stat_.receive_early_data_size(),

                   stat_.mode(),
                   stat_.stage(),
                   stat_.alert());
}

int ClientChannel::AduitForActiveSession() {
    StageAudit expect_stage_audit;
    switch (state_.mode()) {
        case HS_MODE_ONE_RTT_ECDH:
            expect_stage_audit.set_flags(7,
                                         STAGE_INIT,
                                         STAGE_SEND_CLIENT_HELLO,
                                         STAGE_RECV_SERVER_HELLO,
                                         STAGE_RECV_CERTIFICATE_VERIFY,
                                         STAGE_RECV_NEW_SESSION_TICKET,
                                         STAGE_RECV_SERVER_FINISHED,
                                         STAGE_COMPLETED);
            if (is_long_connection_) {
                expect_stage_audit.set_flag(STAGE_SEND_CLIENT_FINISHED);
            }
            break;
        case HS_MODE_ONE_RTT_PSK:
            // only use for long connect's so must have STAGE_SEND_CLIENT_FINISHED
            expect_stage_audit.set_flags(6,
                                         STAGE_INIT,
                                         STAGE_SEND_CLIENT_HELLO,
                                         STAGE_RECV_SERVER_HELLO,
                                         STAGE_RECV_SERVER_FINISHED,
                                         STAGE_SEND_CLIENT_FINISHED,
                                         STAGE_COMPLETED);
            break;
        case HS_MODE_ZERO_RTT_PSK:
            // can't go to active session?
            expect_stage_audit.set_flags(8,
                                         STAGE_INIT,
                                         STAGE_SEND_CLIENT_HELLO,
                                         STAGE_SEND_ENCRYPTED_EXTENSIONS,
                                         STAGE_SEND_EARLY_APP_DATA,
                                         STAGE_RECV_SERVER_HELLO,
                                         STAGE_RECV_SERVER_FINISHED,
                                         STAGE_RECV_EARLY_APP_DATA,
                                         STAGE_COMPLETED);
            break;
        default:
            return ERR_UNEXPECT_CHECK_FAIL;
    }
    int ret = state_.stage_audit().CheckEqual(expect_stage_audit);
    MMTLS_CHECK_EQ(OK, ret, ret, "check stage audit fail.mode %u con %u", state_.mode(), is_long_connection_);
    return OK;
}

int ClientChannel::AuditFor1stSendToPeer() {
    StageAudit expect_stage_audit;
    switch (state_.mode()) {
        case HS_MODE_ONE_RTT_ECDH:
        case HS_MODE_ONE_RTT_PSK:
            expect_stage_audit.set_flags(2, STAGE_INIT, STAGE_SEND_CLIENT_HELLO);
            break;
        case HS_MODE_ZERO_RTT_PSK:
            expect_stage_audit.set_flags(4,
                                         STAGE_INIT,
                                         STAGE_SEND_CLIENT_HELLO,
                                         STAGE_SEND_ENCRYPTED_EXTENSIONS,
                                         STAGE_SEND_EARLY_APP_DATA);
            break;
        default:
            return ERR_UNEXPECT_CHECK_FAIL;
    }
    int ret = state_.stage_audit().CheckEqual(expect_stage_audit);
    MMTLS_CHECK_EQ(OK, ret, ret, "check stage audit fail");
    return OK;
}

int ClientChannel::AuditForReceiveEarlyData() {
    MMTLS_CHECK_EQ(HS_MODE_ZERO_RTT_PSK, state_.mode(), ERR_UNEXPECT_CHECK_FAIL, state_.mode());

    StageAudit expect_stage_audit;
    expect_stage_audit.set_flags(6,
                                 STAGE_INIT,
                                 STAGE_SEND_CLIENT_HELLO,
                                 STAGE_SEND_ENCRYPTED_EXTENSIONS,
                                 STAGE_SEND_EARLY_APP_DATA,
                                 STAGE_RECV_SERVER_HELLO,
                                 STAGE_RECV_SERVER_FINISHED);
    int ret = state_.stage_audit().CheckEqual(expect_stage_audit);
    MMTLS_CHECK_EQ(OK, ret, ret, "check stage audit fail");
    return OK;
}

}  // namespace mmtls
