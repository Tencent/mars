#include "mmtls_record_writer.h"

#include "mmtls_utils.h"

using namespace mmtls;
RecordWriter::RecordWriter() {
}

int RecordWriter::Write(DataWriter& writer,
                        RecordType type,
                        const ProtocolVersion& version,
                        const mmtls::String& raw_buf,
                        ConnectionSeqNumbers& seqnum,
                        ConnectionCipherState* conn_cipher_state) {
    return Write(writer, type, version, raw_buf.c_str(), raw_buf.size(), seqnum, conn_cipher_state);
}

int RecordWriter::Write(DataWriter& writer,
                        RecordType type,
                        const ProtocolVersion& version,
                        const char* raw_buf,
                        size_t raw_buf_size,
                        ConnectionSeqNumbers& seqnum,
                        ConnectionCipherState* conn_cipher_state) {
    MMTLS_CHECK_TRUE(raw_buf != NULL, ERR_UNEXPECT_CHECK_FAIL, "raw_buf is null");

    size_t payload_size = 0;
    int ret = GetPayloadSize(conn_cipher_state, raw_buf_size, payload_size);
    MMTLS_CHECK_EQ(OK, ret, ret, "get payload size fail");
    MMTLS_CHECK_LE(payload_size, MAX_CIPHERTEXT_SIZE, ERR_LENGTH_EXCEED_LIMIT, "payload over size");

    RecordHead record;
    ret = record.Assign(type, version, payload_size);
    MMTLS_CHECK_EQ(OK,
                   ret,
                   ret,
                   "assign record fail.type %u version %u paysize %zu",
                   type,
                   version.version(),
                   payload_size);

    ret = record.Serialize(writer);
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize record fail");

    ret = WritePayload(type, raw_buf, raw_buf_size, seqnum, conn_cipher_state, writer.MutableBuffer());
    MMTLS_CHECK_EQ(OK, ret, ret, "write record payload fail");

    return OK;
}

int RecordWriter::WritePayload(RecordType type,
                               const char* raw_buf,
                               size_t raw_buf_size,
                               ConnectionSeqNumbers& seqnum,
                               ConnectionCipherState* conn_cipher_state,
                               mmtls::String& buffer) {
    MMTLS_CHECK_TRUE(raw_buf != NULL, ERR_UNEXPECT_CHECK_FAIL, "raw_buf is null");

    uint64 seq = seqnum.NextWriteSeq();

    if (!conn_cipher_state) {
        buffer.append(raw_buf, raw_buf_size);
        return OK;
    }

    AeadCrypter* aead_crypter = conn_cipher_state->aead_crypter();
    MMTLS_CHECK_TRUE(aead_crypter, ERR_UNEXPECT_CHECK_FAIL, "now only aead crypter.so can't be null");

    mmtls::String nonce;
    conn_cipher_state->FormatNonce(seq, nonce);

    size_t payload_size = aead_crypter->GetCipherTextSize(raw_buf_size);

    mmtls::String aad;
    conn_cipher_state->FormatAad(seq, type, payload_size, aad);

    int ret = aead_crypter->Crypt(reinterpret_cast<const byte*>(nonce.data()),
                                  nonce.size(),
                                  reinterpret_cast<const byte*>(aad.data()),
                                  aad.size(),
                                  reinterpret_cast<const byte*>(raw_buf),
                                  raw_buf_size,
                                  buffer);

    MMTLS_CHECK_EQ(OK, ret, ERR_ENCRYPT_FAIL, "encrypt record payload fail");
    return OK;
}

int RecordWriter::GetPayloadSize(ConnectionCipherState* conn_cipher_state, size_t raw_buf_size, size_t& payload_size) {
    if (NULL == conn_cipher_state) {
        payload_size = raw_buf_size;
    } else {
        AeadCrypter* aead_crypter = conn_cipher_state->aead_crypter();
        MMTLS_CHECK_TRUE(aead_crypter, ERR_UNEXPECT_CHECK_FAIL, "now only aead crypter.so can't be null");
        payload_size = aead_crypter->GetCipherTextSize(raw_buf_size);
    }
    return OK;
}
