#pragma once

#include "mmtls_connection_cipher_state.h"
#include "mmtls_connection_seq_number.h"
#include "mmtls_constants.h"
#include "mmtls_record_head.h"
#include "mmtls_string.h"
#include "mmtls_version.h"

namespace mmtls {

class Encrypter;

// Write records and encrypt record payload if necessary.
// Please note that this writer won't split the record even its very big.
// Caller should make sure a record is fitted in their logical size.
class RecordWriter {
 public:
    RecordWriter();

    /*
     * @note Write a msg(handshake msg/alert/appdata) to a serialize record to a DataWriter. Payload will be encrypted
     * if necessary.
     * @param  writer : writer to write serialize record
     * @param  type   : write record type
     * @param  version: recrod version
     * @param  raw_buf: record raw payload
     * @param  seqnum : connection sequence number
     *
     */
    int Write(DataWriter& writer,
              RecordType type,
              const ProtocolVersion& version,
              const mmtls::String& raw_buf,
              ConnectionSeqNumbers& seqnum,
              ConnectionCipherState* conn_cipher_state);

    int Write(DataWriter& writer,
              RecordType type,
              const ProtocolVersion& version,
              const char* raw_buf,
              size_t raw_buf_size,
              ConnectionSeqNumbers& seqnum,
              ConnectionCipherState* conn_cipher_state);

 protected:
    int WritePayload(RecordType type,
                     const char* raw_buf,
                     size_t raw_buf_size,
                     ConnectionSeqNumbers& seqnum,
                     ConnectionCipherState* conn_cipher_state,
                     mmtls::String& buffer);

    int GetPayloadSize(ConnectionCipherState* conn_cipher_state, size_t raw_buf_size, size_t& payload_size);

 private:
    // Disallow copy and assign.
    RecordWriter(const RecordWriter& writer);
    RecordWriter& operator=(const RecordWriter& writer);
};

}  // namespace mmtls
