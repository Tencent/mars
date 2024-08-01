#pragma once

#include <string>

#include "comm/mmtls_data_pack.h"
#include "comm/mmtls_data_reader.h"
#include "comm/mmtls_psk.h"
#include "comm/mmtls_string.h"
#include "comm/mmtls_types.h"

namespace mmtls {

class Psk;

// The client PSK structure store
class ClientPsk : public DataPackInterface {
 public:
    ClientPsk() : psk_(NULL), expired_time_(0) {
    }

    ClientPsk(Psk* psk, uint64 expired_time, const mmtls::String& pre_shared_key)
    : psk_(psk), expired_time_(expired_time), pre_shared_key_(pre_shared_key) {
    }

    ClientPsk(const ClientPsk& other);
    ClientPsk& operator=(const ClientPsk& other);

    virtual ~ClientPsk();

    int Init(DataReader& reader);

    Psk* psk() const {
        return psk_;
    }
    uint64 expired_time() const {
        return expired_time_;
    }
    const mmtls::String& pre_shared_key() const {
        return pre_shared_key_;
    }

    void Clear();

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    Psk* psk_;
    // The psk expired timestamp in second.
    uint64 expired_time_;
    // The plaintext pre_shared_key.
    mmtls::String pre_shared_key_;
};

}  // namespace mmtls
