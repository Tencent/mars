#include "mmtls_channel.h"

#include "mmtls_alert.h"
#include "mmtls_connection_cipher_state.h"
#include "mmtls_constants.h"
#include "mmtls_event_callback.h"
#include "mmtls_handshake_state.h"
#include "mmtls_record_reader.h"
#include "mmtls_record_writer.h"
#include "mmtls_utils.h"

namespace mmtls {

Channel::Channel()
: alerted_(false), send_bytes_ptr_(std::make_shared<mmtls::String>()), send_writer_(*send_bytes_ptr_.get()) {
    record_writer_ = new RecordWriter();
    record_reader_ = new RecordReader();
}

Channel::~Channel() {
    if (record_writer_) {
        delete record_writer_;
        record_writer_ = NULL;
    }
    if (record_reader_) {
        delete record_reader_;
        record_reader_ = NULL;
    }

    ClearCipherState();
}

int Channel::BuildWriteBuffer(RecordType record_type, const mmtls::String& bytes) {
    return BuildWriteBuffer(record_type, bytes.c_str(), bytes.size());
}

int Channel::BuildWriteBuffer(RecordType record_type, const char* bytes, size_t size) {
    size_t start_len = send_writer_.Buffer().size();
    size_t send_record_count = 0;
    ConnectionCipherState* conn_cipher_state = NULL;
    int ret = SelectConnectionCipherStateForWriter(conn_cipher_state);
    MMTLS_CHECK_EQ(OK, ret, ret, "select cipher state for record writer fail");

    if (record_type == RECORD_APPLICATION_DATA) {
        size_t max_plaintext_size_per_record = std::min(MAX_PLAINTEXT_SIZE, policy().MaxRecordPlaintextSize());
        MMTLSLOG_DEBUG("debug: write app data to send buffer, max_plaintext_size_per_record %zu, bytes size %zu",
                       max_plaintext_size_per_record,
                       size);

        for (size_t offset = 0; offset < size;) {
            size_t length = std::min(size - offset, max_plaintext_size_per_record);
            ret = record_writer_->Write(send_writer_,
                                        record_type,
                                        state().protocol_version(),
                                        bytes + offset,
                                        length,
                                        seq_numbers_,
                                        conn_cipher_state);
            MMTLS_CHECK_EQ(OK,
                           ret,
                           ret,
                           "write serialized record to send buffer fail, offset %zu, size %zu",
                           offset,
                           size);

            offset += length;
            send_record_count++;
        }

    } else {
        ret = record_writer_->Write(send_writer_,
                                    record_type,
                                    state().protocol_version(),
                                    bytes,
                                    size,
                                    seq_numbers_,
                                    conn_cipher_state);
        MMTLS_CHECK_EQ(OK, ret, ret, "write serialized record to send buffer fail");

        send_record_count++;
    }

    UpdateSendStat(record_type,
                   state().stage(),
                   (int)send_record_count,
                   (int)(send_writer_.Buffer().size() - start_len));

    return OK;
}

int Channel::BuildAlertBuffer(AlertLevel level, AlertType type) {
    DataWriter writer;
    Alert alert(level, type);
    int ret = alert.Serialize(writer);
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize alert fail, level %d, type %d", level, type);

    BuildWriteBuffer(RECORD_ALERT, writer.Buffer());
    return OK;
}

bool Channel::IsActive() {
    return STAGE_COMPLETED == state().stage();
}

bool Channel::IsClosed() {
    return STAGE_CLOSED == state().stage();
}

int Channel::DoHandleError(AlertType type, int error_code) {
    if (alerted_) {
        MMTLSLOG_ERR("has alerted. skip this alert %u %d", type, error_code);
        return error_code;
    }

    alerted_ = true;
    if (!IsClosed()) {
        int ret = BuildAlertBuffer(Alert::GetAlertLevel(type), type);
        if (OK == ret) {
            SendToPeer();
            MMTLSLOG_ERR("send alert type %u errorcode %d to peer", type, error_code);
        }
    }

    Close();
    return error_code;
}

int Channel::ComputeStageConnCipherState(ConnectionSide con_side,
                                         ConnectionKeyType key_type,
                                         const mmtls::String& secret,
                                         const mmtls::String& label) {
    ConnectionKeys* conn_keys = NULL;
    AutoPointHolder<ConnectionKeys> key_holder(conn_keys);

    int ret = state().ComputeStageConnKey(secret, label, conn_keys);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute connection keys fail");
    MMTLS_CHECK_TRUE(NULL != conn_keys, ERR_UNEXPECT_CHECK_FAIL, "conn_keys is null");

    ConnectionCipherState* client_state = NULL;
    AutoPointHolder<ConnectionCipherState> client_state_holder(client_state);

    bool is_our_side = (con_side == CONN_SIDE_CLIENT);
    ret = state().ComputeStageConnCipherState(con_side, is_our_side, *conn_keys, client_state);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute cipher state for client fail");

    if (is_our_side) {
        write_connection_cipher_states_[key_type] = client_state;
    } else {
        read_connection_cipher_states_[key_type] = client_state;
    }
    client_state_holder.Disable();

    ConnectionCipherState* server_state = NULL;
    AutoPointHolder<ConnectionCipherState> server_state_holder(server_state);

    is_our_side = (con_side == CONN_SIDE_SERVER);
    ret = state().ComputeStageConnCipherState(con_side, is_our_side, *conn_keys, server_state);
    MMTLS_CHECK_EQ(OK, ret, ret, "compute cipher state for server fail");

    if (is_our_side) {
        write_connection_cipher_states_[key_type] = server_state;
    } else {
        read_connection_cipher_states_[key_type] = server_state;
    }
    server_state_holder.Disable();

    return OK;
}

int Channel::SelectConnKeyTypeForReader(ConnectionKeyType& con_key_type) {
    switch (state().stage()) {
        // client stages
        case STAGE_SEND_CLIENT_HELLO:
        case STAGE_RECV_SERVER_HELLO:
        // server stages
        case STAGE_RECV_CLIENT_HELLO:
        case STAGE_SEND_SERVER_HELLO:
            con_key_type = CON_KEY_NONE;
            break;

            // client stages
            // If receive alert in these stages, alert should not be encrypted.
        case STAGE_SEND_ENCRYPTED_EXTENSIONS:
        case STAGE_SEND_EARLY_APP_DATA:
            con_key_type = CON_KEY_NONE;
            break;

        // server stages
        case STAGE_RECV_ENCRYPTED_EXTENSIONS:
        case STAGE_RECV_CLIENT_EARLY_DATA:
            con_key_type = CON_KEY_EARLY_DATA;
            break;

        // client stages
        case STAGE_RECV_CERTIFICATE_VERIFY:
        case STAGE_RECV_NEW_SESSION_TICKET:
        case STAGE_RECV_EARLY_APP_DATA:
        case STAGE_RECV_SERVER_FINISHED:
        case STAGE_SEND_CLIENT_FINISHED:
        // server stages
        case STAGE_SEND_CERTIFICATE_VERIFY:
        case STAGE_SEND_NEW_SESSION_TICKET:
        case STAGE_SEND_SERVER_EARLY_DATA:
        case STAGE_SEND_SERVER_FINISHED:
        case STAGE_RECV_CLIENT_FINISHED:
            con_key_type = CON_KEY_HANDSHAKE;
            break;

        case STAGE_COMPLETED:
            con_key_type = CON_KEY_APPLICATION;
            break;
        default:
            return ERR_NO_CON_KEY_TYPE;
    }
    return OK;
}

int Channel::SelectConnKeyTypeForWriter(ConnectionKeyType& con_key_type) {
    switch (state().stage()) {
        // client stages
        case STAGE_SEND_CLIENT_HELLO:
        case STAGE_RECV_SERVER_HELLO:
        // server stages
        case STAGE_RECV_CLIENT_HELLO:
        case STAGE_SEND_SERVER_HELLO:
            con_key_type = CON_KEY_NONE;
            break;

        // client stages
        case STAGE_SEND_ENCRYPTED_EXTENSIONS:
        case STAGE_SEND_EARLY_APP_DATA:
            con_key_type = CON_KEY_EARLY_DATA;
            break;

            // server stages
            // If send alert in these stages, alert should not be encrypted.
        case STAGE_RECV_ENCRYPTED_EXTENSIONS:
        case STAGE_RECV_CLIENT_EARLY_DATA:
            con_key_type = CON_KEY_NONE;
            break;

        // client stages
        case STAGE_RECV_CERTIFICATE_VERIFY:
        case STAGE_RECV_NEW_SESSION_TICKET:
        case STAGE_RECV_EARLY_APP_DATA:
        case STAGE_RECV_SERVER_FINISHED:
        case STAGE_SEND_CLIENT_FINISHED:
        // server stages
        case STAGE_SEND_CERTIFICATE_VERIFY:
        case STAGE_SEND_NEW_SESSION_TICKET:
        case STAGE_SEND_SERVER_EARLY_DATA:
        case STAGE_SEND_SERVER_FINISHED:
        case STAGE_RECV_CLIENT_FINISHED:
            con_key_type = CON_KEY_HANDSHAKE;
            break;

        case STAGE_COMPLETED:
            con_key_type = CON_KEY_APPLICATION;
            break;
        default:
            return ERR_NO_CON_KEY_TYPE;
    }
    return OK;
}

int Channel::SelectConnectionCipherStateForReader(ConnectionCipherState*& con_cipher_state) {
    ConnectionKeyType con_key_type;
    int ret = SelectConnKeyTypeForReader(con_key_type);
    MMTLS_CHECK_EQ(OK, ret, ret, "select connection key type for reader fail.con key type %u", con_key_type);

    std::map<ConnectionKeyType, ConnectionCipherState*>::const_iterator it =
        read_connection_cipher_states_.find(con_key_type);

    MMTLS_CHECK_TRUE(it != read_connection_cipher_states_.end(),
                     ERR_NO_FIND_CON_KEY,
                     "conn key type %u not find conn cipher state",
                     con_key_type);

    con_cipher_state = it->second;
    return OK;
}

int Channel::SelectConnectionCipherStateForWriter(ConnectionCipherState*& con_cipher_state) {
    ConnectionKeyType con_key_type;
    int ret = SelectConnKeyTypeForWriter(con_key_type);
    MMTLS_CHECK_EQ(OK, ret, ret, "select connection key type for writer fail.conkeytype %u", con_key_type);

    std::map<ConnectionKeyType, ConnectionCipherState*>::const_iterator it =
        write_connection_cipher_states_.find(con_key_type);

    MMTLS_CHECK_TRUE(it != write_connection_cipher_states_.end(),
                     ERR_NO_FIND_CON_KEY,
                     "conn key type %u not find conn cipher state",
                     con_key_type);

    con_cipher_state = it->second;
    return OK;
}

void Channel::ActiveSession() {
    for (std::map<ConnectionKeyType, ConnectionCipherState*>::iterator it = write_connection_cipher_states_.begin();
         it != write_connection_cipher_states_.end();
         it++) {
        if (it->first != CON_KEY_APPLICATION) {
            if (it->second != NULL) {
                delete it->second;
                it->second = NULL;
            }
        }
    }

    for (std::map<ConnectionKeyType, ConnectionCipherState*>::iterator it = read_connection_cipher_states_.begin();
         it != read_connection_cipher_states_.end();
         it++) {
        if (it->first != CON_KEY_APPLICATION) {
            if (it->second != NULL) {
                delete it->second;
                it->second = NULL;
            }
        }
    }

    state().ActiveSession();
}

void Channel::ClearCipherState() {
    for (std::map<ConnectionKeyType, ConnectionCipherState*>::iterator it = write_connection_cipher_states_.begin();
         it != write_connection_cipher_states_.end();
         it++) {
        if (it->second != NULL) {
            delete it->second;
            it->second = NULL;
        }
    }

    for (std::map<ConnectionKeyType, ConnectionCipherState*>::iterator it = read_connection_cipher_states_.begin();
         it != read_connection_cipher_states_.end();
         it++) {
        if (it->second != NULL) {
            delete it->second;
            it->second = NULL;
        }
    }

    write_connection_cipher_states_.clear();
    read_connection_cipher_states_.clear();
}

void Channel::Close() {
    if (IsClosed()) {
        MMTLSLOG_IMPT("id %s side %u has close done", state().client_id(), state().con_side());
        return;
    }

    ClearCipherState();

    // set closed stage
    state().SetNextStage(STAGE_CLOSED, false);
    state().Close();

    MMTLSLOG_IMPT("id %s side %u close succ", state().client_id(), state().con_side());
}

void Channel::SendToPeer() {
    if (send_bytes_ptr_->size() > 0) {
        event_callback().OnSendTo(send_bytes_ptr_.get());
        send_bytes_ptr_ = std::make_shared<mmtls::String>();
        send_writer_.Reset(send_bytes_ptr_.get());
        state().inc_send_to_peer_cnt();
    }
}

void Channel::UpdateSendStat(RecordType type, HandShakeStage stage, int send_record_count, int send_size) {
    inner_stat().add_send_data_size(send_size);
    inner_stat().add_send_data_cnt(send_record_count);

    switch (type) {
        case RECORD_EARLY_HANDSHAKE:
        case RECORD_HANDSHAKE:
            inner_stat().add_send_handshake_data_size(send_size);
            inner_stat().add_send_handshake_data_cnt(send_record_count);
            break;

        case RECORD_APPLICATION_DATA:
            if (stage == STAGE_SEND_EARLY_APP_DATA || stage == STAGE_SEND_SERVER_EARLY_DATA) {
                inner_stat().add_send_early_data_size(send_size);
                inner_stat().add_send_early_data_cnt(send_record_count);
            }
            break;
    }
}

void Channel::UpdateReceiveStat(HandShakeStage stage, const RecordHead& record) {
    inner_stat().add_receive_data_cnt(1);
    inner_stat().add_receive_data_size(record.GetPayloadSize() + FIXED_RECORD_HEAD_SIZE);
    switch (record.GetType()) {
        case RECORD_EARLY_HANDSHAKE:
        case RECORD_HANDSHAKE:
            inner_stat().add_receive_handshake_data_cnt(1);
            inner_stat().add_receive_handshake_data_size(record.GetPayloadSize() + FIXED_RECORD_HEAD_SIZE);
            break;

        case RECORD_APPLICATION_DATA:
            if (stage == STAGE_RECV_EARLY_APP_DATA || stage == STAGE_RECV_CLIENT_EARLY_DATA) {
                inner_stat().add_receive_early_data_cnt(1);
                inner_stat().add_receive_early_data_size(record.GetPayloadSize() + FIXED_RECORD_HEAD_SIZE);
            }
            break;
    }
}

uint32 Channel::HeartbeatStartTime() {
    return seq_numbers_.heartbeat_start_time();
}

}  // namespace mmtls
