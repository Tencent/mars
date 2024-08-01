#pragma once
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"
#include "mmtls_string.h"

namespace mmtls {

class SerializeWriter {
 public:
    SerializeWriter(DataWriter& writer) : writer_(writer), pos_(0) {
    }

    void EnsureMsgLengthSpaceAtBegin() {
        pos_ = writer_.Tell();

        writer_.Write((uint32)0x00);

        // MMTLSLOG_DEBUG("write pos %zu",pos_);
    }

    int SetMsgLengthAtEnd() {
        size_t end_pos = writer_.Tell();
        MMTLS_CHECK_GT(end_pos,
                       pos_ + sizeof(uint32),
                       ERR_UNEXPECT_CHECK_FAIL,
                       "msg end pos %zu less than expect %zu + 2",
                       end_pos,
                       pos_);
        uint32 msg_len = (uint32)(end_pos - pos_ - sizeof(uint32));
        // MMTLSLOG_DEBUG("replace pos %zu msglen %u end pos %zu",pos_,msg_len,end_pos);
        return writer_.Replace(pos_, msg_len);
    }

    /*
     *@note write binary data using network order,to buff
     *
     */
    void Write(const void* buf, size_t size) {
        writer_.Write(buf, size);
    }

    /*
     *@note write binary data using network order,to buff
     *
     */
    void Write(const mmtls::String& buf) {
        writer_.Write(buf);
    }

    /**
     *@note write host data to network buffer
     *@pram val : type T must define in mmtls_types.h
     */
    template <typename T>
    void Write(const T& val) {
        writer_.Write(val);
    }

    /**
     * @note write host data to network buffer
     * @pram val : type T must be uint32 or uint16
     */
    template <typename T>
    void WriteVarintT(T val) {
        writer_.WriteVarintT(val);
    }

    DataWriter& writer() {
        return writer_;
    }

 protected:
    DataWriter& writer_;
    size_t pos_;
};

class DeserializeReader {
 public:
    DeserializeReader(DataReader& reader) : reader_(reader), start_pos_(0), end_pos_(0), now_pos_(0), len_(0) {
    }

    int GetMsgLengthAtBegin() {
        int ret = reader_.Get(len_);
        MMTLS_CHECK_EQ(OK, ret, ret, "get msg lenth fail");
        MMTLS_CHECK_GT(len_, 0U, ERR_DECODE_ERROR, "len should >0");
        MMTLS_CHECK_LT(len_, MAX_DATA_PACK_SIZE, ERR_DECODE_ERROR, "len to larger. %u", len_);
        MMTLS_CHECK_LE(len_,
                       reader_.Remaining(),
                       ERR_DECODE_ERROR,
                       "len %u should >= reader remain %zu",
                       len_,
                       reader_.Remaining());

        start_pos_ = reader_.Tell();
        end_pos_ = start_pos_ + len_;
        // MMTLSLOG_DEBUG("get len %u endpos %zu",len_,end_pos_);
        return OK;
    }

    int ForwardMsgTailAtEnd() {
        now_pos_ = reader_.Tell();
        MMTLS_CHECK_GE(end_pos_,
                       now_pos_,
                       ERR_UNEXPECT_CHECK_FAIL,
                       "msg end pos %zu + len %u should >= now pos %zu",
                       end_pos_,
                       len_,
                       now_pos_);

        size_t discard_len = end_pos_ - now_pos_;
        if (discard_len > 0) {
            int ret = reader_.DiscardNext(discard_len);
            MMTLS_CHECK_EQ(OK, ret, ret, "forward msg tail fail.at discard. now %zu endpos %zu", now_pos_, end_pos_);
        }
        // MMTLSLOG_DEBUG("len %u endpos %zu dicard len %zu nowpos %zu",len_,end_pos_,discard_len,now_pos);
        return OK;
    }

    inline int CheckSpace(size_t size) {
        size_t now_pos = reader_.Tell();
        if (now_pos + size <= end_pos_) {
            return 0;
        } else if (now_pos == end_pos_) {
            return 1;
        } else {
            MMTLSLOG_ERR("now pos %zu greater than endpos %zu len %u size %zu", now_pos, end_pos_, len_, size);
            return ERR_UNEXPECT_CHECK_FAIL;
        }
    }

    /**
     * @note read network buffer to host type data
     * @param val : type T must define in mmtls_types.h
     * @param move_offset : true then move the offset forward
     * @return 0 :succ,get val
     * 		 else : fail
     */
    template <typename T>
    inline int Get(T& val, bool move_offset = true, const T& default_val = 0, bool required = true) {
        int ret = CheckSpace(sizeof(T));
        if (0 == ret) {
            //��Ȼ�����ݵ����
            return reader_.Get(val, move_offset);
        } else if (1 == ret) {
            //�Ѿ�û�������ˣ����Ϊ�ɰ汾��ϣ������°汾�Ľ�����°汾���ӵ��ֶ�
            MMTLS_CHECK_FALSE(required, ERR_NO_ENOUGH_DATA, "item not exist");
            val = default_val;
            return OK;
        }
        return ret;
    }

    /**
     * @note read network buffer to host type data
     * @param val : type T must be uint32 or uint16
     * @param move_offset : true then move the offset forward
     * @return 0 :succ,get val
     *            else : fail
     */
    template <typename T>
    inline int GetVarintT(T& val, bool move_offset = true, const T& default_val = 0, bool required = true) {
        int ret = CheckSpace(1);
        if (0 == ret) {
            return reader_.GetVarintT(val, move_offset);
        } else if (1 == ret) {
            MMTLS_CHECK_FALSE(required, ERR_NO_ENOUGH_DATA, "item not exist");
            val = default_val;
            return OK;
        }
        return ret;
    }

    /*@note get string from data
     *@param val : output string
     *@param size : need size
     *
     * @return 0 :succ,get data with need size
     * 		 else : fail
     */
    int GetString(mmtls::String& val, size_t size, bool required = true, const mmtls::String& default_val = "") {
        int ret = CheckSpace(size);
        if (0 == ret) {
            return reader_.GetString(val, size);
        } else if (1 == ret) {
            val.assign(default_val);
            return OK;
        }
        return ret;
    }

    /*@note get string from data
     *@param size : need size
     *
     * @return buf addr :succ,get buff addr
     * 		 NULL : fail
     */
    const byte* GetString(size_t size) {
        int ret = CheckSpace(size);
        if (0 == ret) {
            return reader_.GetString(size);
        } else {
            return NULL;
        }
    }

    int DiscardNext(size_t bytes) {
        return reader_.DiscardNext(bytes);
    }

    DataReader& reader() {
        return reader_;
    }

 protected:
    DataReader& reader_;
    size_t start_pos_;
    size_t end_pos_;
    size_t now_pos_;
    uint32 len_;
};

// All inherited classes should only mutate their states via constructor, Serialize, Deserialize, DeserializeFromString
// No setter method to mutate a single field..
class DataPackInterface {
 public:
    DataPackInterface() {
    }
    virtual ~DataPackInterface() {
    }

    virtual int Serialize(DataWriter& writer);
    virtual int Deserialize(DataReader& reader);

 public:
    template <typename T>
    static int DetectPackNumber(T& val, const DataReader& reader, size_t offset) {
        std::unique_ptr<DataReader> rd(reader.SubReader());
        mmtls::DeserializeReader deserial_reader(*rd.get());
        int ret = deserial_reader.GetMsgLengthAtBegin();
        MMTLS_CHECK_EQ(OK, ret, ret, "GetMsgLengthAtBegin fail");

        ret = deserial_reader.DiscardNext(offset);
        MMTLS_CHECK_EQ(OK, ret, ret, "discard offset %zu fail", offset);

        ret = deserial_reader.Get(val);
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type");
        return OK;
    }

 protected:
    virtual int DoSerialize(SerializeWriter& writer) = 0;
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader) = 0;
};

}  // namespace mmtls
