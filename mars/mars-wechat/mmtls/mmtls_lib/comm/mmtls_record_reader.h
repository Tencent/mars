#pragma once
#include "mmtls_connection_cipher_state.h"
#include "mmtls_connection_seq_number.h"
#include "mmtls_record_head.h"
#include "mmtls_string.h"
#include "mmtls_types.h"
#include "mmtls_version.h"

namespace mmtls {

class Decrypter;

// Read records and decrypt record payload if necessary.
// This is a stateful record reader. Here is a sample illustration.
//
// RecordReader reader;//a channel element data
//
class RecordReader {
 public:
    RecordReader();

    /*
     *@note set received data from proxy.
     */
    void SetReceivedData(const char* buff, size_t size, ConnectionSeqNumbers& seqnum);

    /*@note get record from received data
     *@param [OUT] record:  decode record data from network
     *@param [OUT] plaintext: decrypted record payload
     *@param [OUT] payload: record payload pointer
     *@param [IN ] conn_cipher_state : connect cipher state
     *
     * @return
     *       0  : get record succ
     *       >0 : to indicate the number of bytes still needed
     *       <0 : error occurs.send alert to peer connection
     */
    int GetNextRecord(RecordHead& record,
                      mmtls::String& plaintext,
                      const byte*& payload,
                      ConnectionCipherState* conn_cipher_state);

    /*
     * @note check if the received data has beed read complte
     * @note before calling SetReceivedData, this must be call to ensure
     *       that all previous data are read
     */
    bool IsReadToEnd() const;

 protected:
    int DecryptRecord(RecordHead& record,
                      const byte* payload,
                      ConnectionCipherState* conn_cipher_state,
                      mmtls::String& plaintext);

    int FillBufferTo(size_t desired);

    const char* GetReadPos();

    void ClearReadBytes();

 private:
    // Disallow copy and assign.
    RecordReader(const RecordReader& reader);
    RecordReader& operator=(const RecordReader& reader);

 protected:
    ConnectionSeqNumbers* con_seq_number_;

    const char* recv_buf_;
    size_t recv_buf_size_;
    size_t recv_buf_offset_;

    size_t read_size_;
    mmtls::String readbuf_;
};

}  // namespace mmtls
