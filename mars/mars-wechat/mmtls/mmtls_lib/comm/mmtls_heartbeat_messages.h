#pragma once

#include "mmtls_constants.h"
#include "mmtls_data_pack.h"
#include "mmtls_string.h"

namespace mmtls {

class Heartbeat : public DataPackInterface {
 public:
    Heartbeat();
    Heartbeat(const HeartbeatType heartbeat_type, const mmtls::String& payload, const mmtls::String& padding);
    virtual ~Heartbeat();

    const HeartbeatType heartbeat_type() const {
        return heartbeat_type_;
    }

    const mmtls::String& payload() const {
        return payload_;
    }

    const mmtls::String& padding() const {
        return padding_;
    }

 protected:
    virtual int DoSerialize(SerializeWriter& writer);
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    HeartbeatType heartbeat_type_;
    mmtls::String payload_;
    mmtls::String padding_;
};

}  // namespace mmtls
