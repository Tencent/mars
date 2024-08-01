#include "mmtls_data_pack.h"

using namespace mmtls;

int DataPackInterface::Serialize(DataWriter& writer) {
    mmtls::SerializeWriter serial_writer(writer);
    serial_writer.EnsureMsgLengthSpaceAtBegin();

    int ret = DoSerialize(serial_writer);
    MMTLS_CHECK_EQ(OK, ret, ret, "doserialize fail");

    ret = serial_writer.SetMsgLengthAtEnd();
    MMTLS_CHECK_EQ(OK, ret, ret, "SetMsgLengthAtEnd fail");
    return ret;
}

int DataPackInterface::Deserialize(DataReader& reader) {
    mmtls::DeserializeReader deserial_reader(reader);
    int ret = deserial_reader.GetMsgLengthAtBegin();
    MMTLS_CHECK_EQ(OK, ret, ret, "GetMsgLengthAtBegin fail");

    ret = DoDeserialize(deserial_reader);
    MMTLS_CHECK_EQ(OK, ret, ret, "do deserial fail");

    ret = deserial_reader.ForwardMsgTailAtEnd();
    MMTLS_CHECK_EQ(OK, ret, ret, "forward to tail fail");
    return ret;
}
