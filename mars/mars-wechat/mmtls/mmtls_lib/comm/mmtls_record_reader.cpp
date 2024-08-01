#include "mmtls_record_reader.h"
using namespace mmtls;

RecordReader::RecordReader()
: con_seq_number_(NULL), recv_buf_(NULL), recv_buf_size_(0), recv_buf_offset_(0), read_size_(0) {
}

void RecordReader::SetReceivedData(const char* recv_buf, size_t size, ConnectionSeqNumbers& seqnum) {
    if (recv_buf == NULL) {
        MMTLSLOG_ERR("recv_buf is null");
        return;
    }

    recv_buf_ = recv_buf;
    recv_buf_size_ = size;
    recv_buf_offset_ = 0;
    con_seq_number_ = &seqnum;
}

int RecordReader::FillBufferTo(size_t desired) {
    if (desired <= read_size_)
        return 0;

    MMTLS_CHECK_TRUE(recv_buf_, ERR_UNEXPECT_CHECK_FAIL, "recv buf null");
    MMTLS_CHECK_GT(recv_buf_size_, 0U, ERR_UNEXPECT_CHECK_FAIL, "recv buf null");

    // If readbuf_ is empty, read bytes are recv_buf_[recv_buf_offset, recv_buf_offset + read_size_)
    // If readbuf_ is not empty, read bytes are readbuf_[0, read_size_)
    size_t remain_size = recv_buf_size_ - recv_buf_offset_ - (readbuf_.empty() ? read_size_ : 0);
    size_t taken = std::min(remain_size, desired - read_size_);

    // Copy to readbuf_ if
    // 1. readbuf_ already have some bytes
    // 2. remain bytes in recv_buf_ is not enough for desired
    if (!readbuf_.empty()) {
        readbuf_.append((const char*)recv_buf_ + recv_buf_offset_, taken);
        recv_buf_offset_ += taken;
    } else if (desired - read_size_ > remain_size) {
        readbuf_.append((const char*)recv_buf_ + recv_buf_offset_, read_size_ + taken);
        recv_buf_offset_ += read_size_ + taken;
    }

    read_size_ += taken;
    return (int)(desired - read_size_);
}

const char* RecordReader::GetReadPos() {
    if (readbuf_.empty()) {
        return (const char*)recv_buf_ + recv_buf_offset_;
    } else {
        return readbuf_.data();
    }
}

int RecordReader::DecryptRecord(RecordHead& record,
                                const byte* payload,
                                ConnectionCipherState* conn_cipher_state,
                                mmtls::String& plaintext) {
    MMTLS_CHECK_TRUE(payload != NULL, ERR_UNEXPECT_CHECK_FAIL, "payload is null");
    MMTLS_CHECK_TRUE(con_seq_number_, ERR_UNEXPECT_CHECK_FAIL, "connection sequence number is null");

    uint64 seq = con_seq_number_->NextReadSeq();

    if (!conn_cipher_state) {
        plaintext.assign(reinterpret_cast<const char*>(payload), record.GetPayloadSize());
        return OK;
    }

    AeadCrypter* aead_crypter = conn_cipher_state->aead_crypter();
    MMTLS_CHECK_TRUE(aead_crypter, ERR_UNEXPECT_CHECK_FAIL, "now only aead crypter.so can't be null");

    mmtls::String nonce;
    mmtls::String aad;
    conn_cipher_state->FormatNonce(seq, nonce);
    conn_cipher_state->FormatAad(seq, record.GetType(), record.GetPayloadSize(), aad);

    int ret = aead_crypter->Crypt(reinterpret_cast<const byte*>(nonce.data()),
                                  nonce.size(),
                                  reinterpret_cast<const byte*>(aad.data()),
                                  aad.size(),
                                  payload,
                                  record.GetPayloadSize(),
                                  plaintext);

    MMTLS_CHECK_EQ(OK,
                   ret,
                   ret,
                   "decrypt record payload fail. receive buffer offset %zu receive buffer size %zu",
                   recv_buf_offset_,
                   recv_buf_size_);

    return ret;
}

bool RecordReader::IsReadToEnd() const {
    return (recv_buf_size_ <= recv_buf_offset_ + (readbuf_.empty() ? read_size_ : 0));
}

void RecordReader::ClearReadBytes() {
    if (readbuf_.empty()) {
        recv_buf_offset_ += read_size_;
    }
    readbuf_.clear();
    read_size_ = 0;
}

int RecordReader::GetNextRecord(RecordHead& record,
                                mmtls::String& plaintext,
                                const byte*& payload,
                                ConnectionCipherState* conn_cipher_state) {
    MMTLS_CHECK_TRUE(recv_buf_, ERR_UNEXPECT_CHECK_FAIL, "receive buffer is null");
    MMTLS_CHECK_GT(recv_buf_size_, 0U, ERR_UNEXPECT_CHECK_FAIL, "recv buf null");

    if (read_size_ < FIXED_RECORD_HEAD_SIZE) {
        if (int need = FillBufferTo(FIXED_RECORD_HEAD_SIZE)) {
            return need;
        }
        MMTLS_CHECK_EQ(FIXED_RECORD_HEAD_SIZE, read_size_, ERR_UNEXPECT_CHECK_FAIL, "read entire record header fail");
    }

    DataReader headreader(GetReadPos(), read_size_);
    int ret = record.Deserialize(headreader);
    if (OK != ret) {
        ClearReadBytes();
        MMTLSLOG_ERR("deserialize record header fail, ret %d", ret);
        return ret;
    }

    if (int need = FillBufferTo(FIXED_RECORD_HEAD_SIZE + record.GetPayloadSize())) {
        return need;
    }

    MMTLS_CHECK_EQ(FIXED_RECORD_HEAD_SIZE + record.GetPayloadSize(),
                   read_size_,
                   ERR_UNEXPECT_CHECK_FAIL,
                   "read entire record fail");

    DataReader recordreader(GetReadPos(), read_size_);
    MMTLS_CHECK_EQ(OK,
                   recordreader.DiscardNext(FIXED_RECORD_HEAD_SIZE),
                   ERR_UNEXPECT_CHECK_FAIL,
                   "forward sizse %zu fail",
                   FIXED_RECORD_HEAD_SIZE);

    payload = recordreader.GetString(record.GetPayloadSize());

    MMTLS_CHECK_TRUE(payload, ERR_UNEXPECT_CHECK_FAIL, "get payload fail");

    ret = DecryptRecord(record, payload, conn_cipher_state, plaintext);
    if (OK != ret) {
        record.SetInvalid();
        ClearReadBytes();
        return ERR_DECRYPT_FAIL;
    }
    ClearReadBytes();
    return OK;
}
