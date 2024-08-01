#pragma once

#include <map>
#include <memory>

#include "mmtls_connection_seq_number.h"
#include "mmtls_constants.h"
#include "mmtls_policy.h"
#include "mmtls_record_head.h"
#include "mmtls_stat.h"
#include "mmtls_string.h"

namespace mmtls {

class CredentialManager;
class EventCallback;
class HandShakeMessage;
class HandShakeState;
class RecordWriter;
class RecordReader;
class ConnectionCipherState;
class ConnectionCipherStateSelector;
class Alert;

// The super class for mmtls client/server endpoint.
class Channel {
 public:
    Channel();

    virtual ~Channel();

    // Send bytes to counterparty. It may call OnSend callback in event_callback_ to let mmtls user to get application
    // data. It will return error if the current handshake state is not allowed to send data.
    virtual int Send(const mmtls::String& bytes) = 0;

    // Receive bytes from counterparty. It may call OnSend to send handshake messages or OnReceive to let mmtls user to
    // get application data. If size_hint is none zero then it means size_hint more bytes are expected.
    virtual int Receive(const char* bytes, size_t size, size_t& size_hint) = 0;

    // close
    virtual void Close();
    virtual bool IsClosed();
    virtual bool IsActive();

 protected:
    int ComputeStageConnCipherState(ConnectionSide con_side,
                                    ConnectionKeyType key_type,
                                    const mmtls::String& secret,
                                    const mmtls::String& label);

    // Handle errors occurred in channel.
    // It may send alert and convert error code to external.
    virtual int DoHandleError(AlertType alert_type, int error_code);

    // Handle a record from counterparty.
    virtual int DoReceiveRecord(const RecordHead& record, mmtls::String& plain_text, const byte* payload) = 0;

    int SelectConnKeyTypeForReader(ConnectionKeyType& con_key_type);
    int SelectConnKeyTypeForWriter(ConnectionKeyType& con_key_type);

    void ActiveSession();
    void ClearCipherState();

    int SelectConnectionCipherStateForReader(ConnectionCipherState*& con_cipher_state);
    int SelectConnectionCipherStateForWriter(ConnectionCipherState*& con_cipher_state);
    int BuildWriteBuffer(RecordType record_type, const char* bytes, size_t size);
    int BuildWriteBuffer(RecordType record_type, const mmtls::String& bytes);
    int BuildAlertBuffer(AlertLevel level, AlertType type);
    virtual void FinalReport(){};
    virtual mmtls::Stat& inner_stat() = 0;
    void UpdateSendStat(RecordType type, HandShakeStage stage, int send_record_count, int send_size);
    void UpdateReceiveStat(HandShakeStage stage, const RecordHead& record);

 protected:
    virtual HandShakeState& state() = 0;
    virtual CredentialManager& credential_manager() = 0;
    virtual EventCallback& event_callback() = 0;
    virtual Policy& policy() = 0;
    void SendToPeer();

    uint32 HeartbeatStartTime();

 protected:
    RecordWriter* record_writer_;
    RecordReader* record_reader_;

    ConnectionSeqNumbers seq_numbers_;

    std::map<ConnectionKeyType, ConnectionCipherState*> write_connection_cipher_states_;
    std::map<ConnectionKeyType, ConnectionCipherState*> read_connection_cipher_states_;

    bool alerted_;

    std::shared_ptr<mmtls::String> send_bytes_ptr_;
    DataWriter send_writer_;
};

}  // namespace mmtls
