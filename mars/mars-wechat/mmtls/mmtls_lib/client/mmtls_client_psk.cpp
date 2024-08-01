#include "mmtls_client_psk.h"

#include "mmtls_psk.h"
#include "mmtls_utils.h"

namespace mmtls {

ClientPsk::ClientPsk(const ClientPsk& other) {
    if (other.psk_ != NULL) {
        psk_ = new Psk(*other.psk_);
    } else {
        psk_ = NULL;
    }
    expired_time_ = other.expired_time_;
    pre_shared_key_ = other.pre_shared_key_;
}

ClientPsk& ClientPsk::operator=(const ClientPsk& other) {
    if (other.psk_ != NULL) {
        psk_ = new Psk(*other.psk_);
    } else {
        psk_ = NULL;
    }
    expired_time_ = other.expired_time_;
    pre_shared_key_ = other.pre_shared_key_;
    return *this;
}

ClientPsk::~ClientPsk() {
    if (psk_ != NULL) {
        delete psk_;
        psk_ = NULL;
    }
}
int ClientPsk::Init(DataReader& reader) {
    return Deserialize(reader);
}

int ClientPsk::DoDeserialize(DeserializeReader& reader) {
    psk_ = new Psk();
    AutoPointHolder<Psk> holder(psk_);
    int ret = psk_->Init(reader.reader());
    MMTLS_CHECK_EQ(OK, ret, ret, "init psk fail");

    ret = reader.Get(expired_time_);
    MMTLS_CHECK_EQ(OK, ret, ret, "get psk expired time fail");

    ret = reader.GetString(pre_shared_key_, FIXED_SECRET_SIZE);
    MMTLS_CHECK_EQ(OK, ret, ret, "get pre shared key fail");

    holder.Disable();
    return OK;
}

int ClientPsk::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_EQ(FIXED_SECRET_SIZE, pre_shared_key_.size(), ERR_ENCODE_ERROR, "pre shared key size is invalid");

    int ret = psk_->Serialize(writer.writer());
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize psk fail");

    writer.Write(expired_time_);
    writer.Write(pre_shared_key_);

    return OK;
}

void ClientPsk::Clear() {
    if (psk_ != NULL) {
        delete psk_;
        psk_ = NULL;
    }
    pre_shared_key_.clear();
}

}  // namespace mmtls
