//
//  http_detector.h
//  sdt
//
//  Created by elviswu on 17/3/23.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#ifndef http_detector_h
#define http_detector_h
#include <string>
#include <vector>
#include <map>

#include "mars/comm/thread/thread.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/socket/socketbreaker.h"
#include "mars/boost/function.hpp"
#include "mars/comm/dns/dns.h"
#include "mars/comm/xlogger/xlogger.h"
class HTTPDetectReq {
  public:
    //static const unsigned long long kDefaultHTTPDetectTimeout = 10*1000; //10s;
  public:
    HTTPDetectReq(const std::string& _url):detect_url_(_url), total_timeout_(10*1000){}
    HTTPDetectReq(const std::string& _url, const unsigned long long& _timeout):detect_url_(_url), total_timeout_(_timeout){}
    ~HTTPDetectReq(){}
    
    std::string ToString(){
        XMessage xmsg;
        xmsg(TSF"detect_url_:%_, total_timeout_:%_.", detect_url_, total_timeout_);
        xmsg(TSF"prior_ip_(%_):", prior_ip_.size());
        for (const auto& ip : prior_ip_) {
            xmsg(TSF"%_;", ip);
        }
        xmsg(TSF"extra_header_:\n");
        for (const auto& kv : extra_header_ ) {
            xmsg(TSF"%_:%_\n", kv.first, kv.second);
        }
        
        return xmsg.String();
    }
    void SetTimeOut(unsigned long long _total_timeout) {total_timeout_ = _total_timeout;}
    void SetPost(const std::string& _post_data) {http_method_="POST"; post_data_ = _post_data;}
    void SetPriorIPs(const std::vector<std::string>& _ips) {prior_ip_ = _ips;}
    
    std::string detect_url_;
    
    std::vector<std::string> prior_ip_;
    
    std::map<std::string, std::string> extra_header_;
    unsigned long long total_timeout_; //ms
    std::string http_method_="GET";
    std::string post_data_;
    
};

class HTTPDectectResult {
  public:
    HTTPDectectResult(){}
    ~HTTPDectectResult(){}
    
    std::string ToString() const{
        XMessage xmsg;
        xmsg(TSF"detect_url_:%_, connect_ip_port_:%_, logic_error_msg_:%_, dns_start_time_:%_, dns_errno_:%_, dns_errmsg_:%_, dns_cost_:%_,",
             detect_url_, connect_ip_port_, logic_error_msg_, dns_start_time_, dns_errno_, dns_errmsg_, dns_cost_);
        xmsg(TSF"tcp_start_connect_time_:%_, tcp_connect_cost_:%_, first_packet_cost_:%_, tcp_connect_errno_:%_, tcp_connect_errmsg_:%_, tcp_rw_errno_:%_, tcp_rw_errmsg_:%_,",
             tcp_start_connect_time_, tcp_connect_cost_, first_packet_cost_, tcp_connect_errno_, tcp_connect_errmsg_, tcp_rw_errno_, tcp_rw_errmsg_);
        xmsg(TSF"http_send_req_time_:%_, http_recv_resp_time_:%_, http_status_code_:%_, http_req_packet_size_:%_, http_resp_packet_size_:%_, http_send_req_cost_:%_, http_recv_resp_cost_:%_, http_errmsg_:%_, http_resp_header_:%_",
             http_send_req_time_, http_recv_resp_time_, http_status_code_, http_req_packet_size_, http_resp_packet_size_, http_send_req_cost_, http_recv_resp_cost_, http_errmsg_, http_resp_header_);
        xmsg(TSF"resolved ips:");
        for (auto ip:dns_resolved_ip_) {
            xmsg(TSF"%_,", ip);
        }
        return xmsg.String();
}
    void Reset(){
        detect_url_ = "";
        connect_ip_port_ = "";
        is_timeout_ = false;
        logic_error_msg_ = "";
        dns_start_time_ = "";
        dns_errno_ = -1;
        dns_errmsg_ = "";
        dns_cost_ = 0;
        dns_resolved_ip_.clear();
        tcp_start_connect_time_ = "";
        tcp_connect_cost_ = 0;
        first_packet_cost_ = 0;
        tcp_connect_errno_ = -1;
        tcp_connect_errmsg_ = "";
        tcp_rw_errno_= -1;
        tcp_rw_errmsg_ = "";
        
        http_send_req_time_ = "";
        http_recv_resp_time_="";
        http_status_code_= -1;
        http_req_packet_size_=0;
        http_resp_packet_size_=0;
        http_send_req_cost_=0;
        http_recv_resp_cost_=0;
        http_errmsg_ = "";
        http_resp_header_ = "";
    }
  public:
    //base info
    std::string         detect_url_;
    std::string         connect_ip_port_;
    bool                is_timeout_ = false;
    //logic error
    std::string         logic_error_msg_;
    //dns
    std::string         dns_start_time_; //localtime
    int                 dns_errno_ = -1;
    std::string         dns_errmsg_;
    unsigned long long  dns_cost_ =0;
    std::vector<std::string> dns_resolved_ip_;
    //tcp
    std::string         tcp_start_connect_time_; //localtime
    unsigned long long  tcp_connect_cost_ = 0;//ms
    unsigned long long  first_packet_cost_= 0 ;//ms
    int                 tcp_connect_errno_= -1;
    std::string         tcp_connect_errmsg_;
    int                 tcp_rw_errno_= -1;
    std::string         tcp_rw_errmsg_;
    
    //http
    std::string         http_send_req_time_; //localtime
    std::string         http_recv_resp_time_; //localtime
    int                 http_status_code_= -1;
    size_t              http_req_packet_size_=0;
    size_t              http_resp_packet_size_=0;
    unsigned long long  http_send_req_cost_=0;
    unsigned long long  http_recv_resp_cost_=0;
    std::string         http_errmsg_;
    std::string         http_resp_header_;
};

class HTTPDetector {
  public:
    typedef void (*DetectEndCallBack)(const HTTPDectectResult&);
    static const unsigned long long kDefaultDNSTimeout = 3*1000;
  public:
    HTTPDetector(const HTTPDetectReq& _req);
    ~HTTPDetector();
    
    /*
     return: 0 for start succ
            <0 for start fail
     */
    int StartAsync(DetectEndCallBack _callback);
    int StartSync(HTTPDectectResult& result);
    int StartSync(const std::vector<std::string>& _prior_ips, HTTPDectectResult& _result);
    
    void CancelAndWait();
    
  private:
    void __Run();
    void __Detect();
    
    HTTPDetectReq      req_;
    HTTPDectectResult   result_;
    
    Mutex               mutex_;
    Thread              worker_thread_;
    SocketBreaker       breaker_;
    
    DetectEndCallBack   callback_;
    
    DNS                 dns_;
    DNSBreaker          dns_breaker_;
    
};



#endif /* http_detector_h */
