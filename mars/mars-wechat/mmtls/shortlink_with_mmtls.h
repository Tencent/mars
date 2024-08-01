/*
 * shortlink_with_mmtls.h
 *
 *  Created on: Jul 15, 2016
 *      Author: elviswu
 */

#ifndef SRC_MMTLS_SHORTLINK_WITH_MMTLS_H_
#define SRC_MMTLS_SHORTLINK_WITH_MMTLS_H_

#include "mars/boot/context.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/socket/getsocktcpinfo.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mmstn.h"
//#include "mars/mm-ext/mmstn_manager.h"
#include "mars/stn/stn_manager.h"
#include "mars/stn/src/shortlink.h"
//#include "net_stat.h"
#include "shortlink_mmtls_support.h"
namespace mars {
namespace stn {

struct ConnectProfile;
struct Task;

class ShortLinkMMTLSProfile : public ProfileExtension {
 public:
    ShortLinkMMTLSProfile() {
    }
    virtual ~ShortLinkMMTLSProfile() {
    }

    void UpdateMMTLSStateSet(int _current_state) {
        comm::ScopedLock lock(vector_mutex_);
        mmtls_state_set_.push_back(_current_state);
    }

    std::string ToString() {
        std::stringstream ss;
        ss << "mmtls_state_set:";

        comm::ScopedLock lock(vector_mutex_);
        std::vector<int>::iterator iter = mmtls_state_set_.begin();
        for (; iter != mmtls_state_set_.end(); ++iter) {
            ss << "->" << *iter;
        }
        return ss.str();
    }

    std::string ToDetailString() {
        std::stringstream ss;
        ss << "mmtls_state_set:";

        comm::ScopedLock lock(vector_mutex_);
        std::vector<int>::iterator iter = mmtls_state_set_.begin();
        for (; iter != mmtls_state_set_.end(); ++iter) {
            if ((kEctMMTLSInitSucc <= *iter && *iter <= kEctMMTLSInvalidFallbackAlert)) {
                ss << "->" << kMMTLSErrCmdCodeStr[*iter - kEctMMTLSInitSucc];
            } else {
                ss << "Error:" << ToString();
                break;
            }
        }
        return ss.str();
    }

    bool MMTLSStateValid() {
        bool ret = true;

        comm::ScopedLock lock(vector_mutex_);
        std::vector<int>::iterator iter = mmtls_state_set_.begin();
        for (; iter != mmtls_state_set_.end(); ++iter) {
            if ((*iter < kEctMMTLSInitSucc || *iter > kEctMMTLSInvalidFallbackAlert)) {
                ret = false;
                xerror2(TSF "invalid mmtls state:%_", ToDetailString());
                break;
            }
        }
        return ret;
    }

    bool ShouldReport() {
        comm::ScopedLock lock(vector_mutex_);
        int last_state = *(mmtls_state_set_.rbegin());
        return (kEctMMTLSInitSucc <= last_state && last_state <= kEctMMTLSInvalidFallbackAlert);
    }

    void Reset() {
        comm::ScopedLock lock(vector_mutex_);
        mmtls_state_set_.clear();
    }

    std::vector<int> mmtls_state_set() {
        comm::ScopedLock lock(vector_mutex_);
        return mmtls_state_set_;
    }

 private:
    std::vector<int> mmtls_state_set_;
    comm::Mutex vector_mutex_;
};

class ShortLinkWithMMTLS : public ShortLink {
 public:
    ShortLinkWithMMTLS(mars::boot::Context* _context,
                       mars::comm::MessageQueue::MessageQueue_t _messagequeueid,
                       std::shared_ptr<NetSource> _netsource,
                       const Task& _task,
                       bool _use_proxy,
//                       std::shared_ptr<NetStat> _net_stat = nullptr,
                       std::unique_ptr<SocketOperator> _operator = nullptr,
                       std::string tls_group_name = "default");
    virtual ~ShortLinkWithMMTLS();

 public:
    // boost::function<void (const std::string& _user_id, std::vector<std::string>& _host_list)> hosts_filter_;
    // boost::function<void (bool _timeout, struct tcp_info& _info)> weaknet_report_;
    // boost::function<void (bool _connect_timeout, struct tcp_info& _info)> add_weaknet_info_;

 public:
    virtual void SendRequest();
    virtual void SendRequest(AutoBuffer& _buf_req, AutoBuffer& _buffer_extend);
    void SetUseProtocol(int _protocol) {
        pack_protocol_ = _protocol;
    }
    void NeedAllResponse(bool _need) {
        need_all_response_ = _need;
    }
    void SetDebugHost(const std::string& _host) {
        debug_host_ = _host;
    }
    void OnConnectHandshakeCompleted();
    void OnNetTimeout();

 protected:
    virtual void __Run();
    virtual void __RunReadWrite(SOCKET _sock, int& _err_type, int& _err_code, ConnectProfile& _conn_profile);

    void __HandleMMTLSRecvBuf(const char* _mmtls_recv_buf,
                              size_t _mmtls_buf_length,
                              ConnectProfile& _conn_profile,
                              int& _err_code,
                              AutoBuffer& _decrypted_recv_buf);
    void __HandleMMTLSErrorCode(const int& _err_type,
                                const int& _err_code,
                                ConnectProfile& _conn_profile,
                                bool& _should_return);

    bool __PackHttp(SOCKET _sock, int& _err_type, int& _err_code, AutoBuffer& _out_buff, ConnectProfile& _conn_profile);
    bool __PackMMtls(SOCKET _sock,
                     int& _err_type,
                     int& _err_code,
                     AutoBuffer& _out_buff,
                     ConnectProfile& _conn_profile);
    bool __MakePayload(int& _err_type, int& _err_code, AutoBuffer& _out_buff, ConnectProfile& _conn_profile);
    bool __MakeTlsPayload(int& _err_type, int& _err_code, const AutoBuffer& _in_buff, ConnectProfile& _conn_profile);
    bool __MakeHttpPayload(int& _err_type,
                           int& _err_code,
                           AutoBuffer& _out_buff,
                           const AutoBuffer& _in_buff,
                           ConnectProfile& _conn_profile);
    bool __PackMMtlsOverHttp();
    void __UpdateMMtlsHandshakeProfile(ConnectProfile _conn_profile);
    void __HandleReceivedMMtlsRegion();
    void __PostSocketTcpInfo(bool _timeout, struct tcp_info _info);

 private:
    mars::boot::Context* context_;
    ShortLinkEventCallback mmtls_observer_;
    ClientChannel* mmtls_channel_;
    bool use_mmtls_;  // server controlling switch

    char mmtls_task_tag_[64];
    int pack_protocol_;
    bool need_all_response_;
    std::string debug_host_;
    bool timeout_;
//    std::shared_ptr<NetStat> net_stat_;
    std::string m_tls_group_name_="default";
};

}  // namespace stn
}  // namespace mars

#endif /* SRC_MMTLS_SHORTLINK_WITH_MMTLS_H_ */
