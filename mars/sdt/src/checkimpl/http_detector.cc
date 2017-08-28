//
//  http_detector.cpp
//  sdt
//
//  Created by elviswu on 17/3/23.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#include <string.h>

#include "http_detector.h"
#include "http_url_parser.h"

#include "boost/bind.hpp"

#include "mars/comm/strutil.h"
#include "mars/comm/http.h"
#include "mars/comm/dns/dns.h"
#include "mars/comm/tickcount.h"
#include "mars/comm/strutil.h"
#include "mars/comm/string_cast.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/block_socket.h"
#include "mars/comm/socket/complexconnect.h"
#include "mars/comm/socket/socket_address.h"

#include "mars/sdt/constants.h"


using namespace mars::sdt;

static const int kComplexConnectTimeout = 5 * 1000; //ms
static const int kComplexConnectInterval = 3 * 1000; //ms


static std::string GetCurTimeStr() {
    struct timeval timeval;
    gettimeofday(&timeval, NULL);
    char temp_time[96] = {0};
    time_t sec = timeval.tv_sec;
    tm tm = *localtime((const time_t*)&sec);
#ifdef ANDROID
    snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3ld", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
             tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, timeval.tv_usec / 1000);
#elif _WIN32
    snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
             (-_timezone) / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, timeval.tv_usec / 1000);
#else
    snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
             tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, timeval.tv_usec / 1000);
#endif
    return std::string(temp_time);
}

HTTPDetector::HTTPDetector(const HTTPDetectReq& _req)
: req_(_req)
, worker_thread_(boost::bind(&HTTPDetector::__Run, this), XLOGGER_TAG "::HTTPDetector")
, callback_(NULL){
    
}
HTTPDetector::~HTTPDetector(){
    CancelAndWait();
}

int HTTPDetector::StartAsync(DetectEndCallBack _callback) {
    ScopedLock lock(mutex_);
    if (worker_thread_.isruning()) {
        xwarn2(TSF"@%_ HTTPDetect is running.", this);
        return -1;
    }
    callback_ = _callback;
    return worker_thread_.start();
}
int HTTPDetector::StartSync(HTTPDectectResult& _result) {
    if (worker_thread_.isruning()) {
        xwarn2(TSF"@%_ HTTPDetect is running.", this);
        return -1;
    }
    dns_breaker_.Clear();
    breaker_.Clear();
    __Detect();
    _result = result_;
    result_.Reset();
    return 0;
}

int HTTPDetector::StartSync(const std::vector<std::string>& _prior_ips, HTTPDectectResult& _result) {
    if (worker_thread_.isruning()) {
        xwarn2(TSF"@%_ HTTPDetect is running.", this);
        return -1;
    }
    dns_breaker_.Clear();
    breaker_.Clear();
    HTTPDetectReq old_req = req_;
    req_.SetPriorIPs(_prior_ips);
    __Detect();
    _result = result_;
    result_.Reset();
    req_ = old_req;
    return 0;
}


void HTTPDetector::CancelAndWait() {
    ScopedLock lock(mutex_);
    dns_.Cancel(dns_breaker_);
    breaker_.Break();
    if (worker_thread_.isruning())
        worker_thread_.join();
}

void HTTPDetector::__Run() {
    __Detect();
    if (NULL!=callback_) {
        callback_(result_);
        callback_ = NULL;
    } else {
        xassert2(false, TSF"@%_ HTTPDetect async, but not set callback", this);
    }
    result_.Reset();
}

void HTTPDetector::__Detect() {
    using namespace http;
    xinfo2(TSF"@%_ HTTPDetect req:%_", this, req_.ToString());
    
    result_.detect_url_ = req_.detect_url_;
    if (!strutil::StartsWith(req_.detect_url_, "http://")) {
        xerror2(TSF"@%_ url is not start with http://, detect_url_:%_", this, req_.detect_url_);
        result_.logic_error_msg_.append("url is not start with http://;\n");
        return;
    }
    
    if (req_.total_timeout_<=0) {
        req_.total_timeout_ = 10*1000;//HTTPDetectReq::kDefaultHTTPDetectTimeout;
    }
    
    tickcount_t detect_tick(true);
    unsigned long long remain_timeout = req_.total_timeout_;
    
    HttpUrlParser url_parser(req_.detect_url_);
    std::string host = url_parser.Host();
    unsigned short port = url_parser.Port()==0? 80 : url_parser.Port();
    
    Builder req_builder(kRequest);
    RequestLine::THttpMethod method = (0==strncasecmp(req_.http_method_.c_str(), "POST", 4)?RequestLine::kPost:RequestLine::kGet);
    req_builder.Request().Method(method);
    req_builder.Request().Version(kVersion_1_1);
    
    req_builder.Request().Url(req_.detect_url_); //TODO remove detect_url_'s http:://
    
    req_builder.Fields().HeaderFiled(HeaderFields::MakeAcceptAll());
    req_builder.Fields().HeaderFiled(HeaderFields::MakeAcceptEncodingGzip());
    req_builder.Fields().HeaderFiled(HeaderFields::KStringHost, host.c_str());
    req_builder.Fields().HeaderFiled(HeaderFields::KStringUserAgent, USER_AGENT);
    
    if (0==strncasecmp(req_.http_method_.c_str(), "POST", 4)) {
        req_builder.Fields().HeaderFiled(HeaderFields::MakeContentLength((int)req_.post_data_.size()));
    }
    
    for (std::map<std::string, std::string>::const_iterator iter=req_.extra_header_.begin(); iter!=req_.extra_header_.end(); ++iter) {
        req_builder.Fields().HeaderFiled(iter->first.c_str(), iter->second.c_str());
    }

    AutoBuffer req_buffer;
    req_builder.HttpToBuffer(req_buffer);
    
    if (0==strncasecmp(req_.http_method_.c_str(), "POST", 4)) {
        req_buffer.Write(req_.post_data_.c_str(), req_.post_data_.size());
    }
    xdebug2(TSF"req_buffer:%_", (char*)req_buffer.Ptr());
    
    std::string svr_ip;
    if (req_.prior_ip_.empty()) {
        
        bool is_host_ipaddr = socket_address(host.c_str(), 0).valid();
        
        svr_ip = host;
        if (!is_host_ipaddr) {
            std::vector<std::string> vec_ip;
            result_.dns_start_time_ = GetCurTimeStr();
            detect_tick.gettickcount();
            bool ret = dns_.GetHostByName(host, vec_ip, kDefaultDNSTimeout<remain_timeout?kDefaultDNSTimeout:remain_timeout, &dns_breaker_);
            result_.dns_cost_ = detect_tick.gettickspan();
            if (ret && !vec_ip.empty()) {
                std::random_shuffle(vec_ip.begin(), vec_ip.end());
                svr_ip = vec_ip[0];
                for (auto ip : vec_ip)
                    result_.dns_resolved_ip_.push_back(ip);
            } else if (dns_breaker_.isbreak) {
                xwarn2(TSF"@%_ GetHostByName break by user", this);
                result_.dns_errmsg_.append("GetHostByName break by user;");
                return;
            } else {
                xerror2(TSF"@%_ GetHostByName error， remain_timeout：%_", this, remain_timeout);
                if (result_.dns_cost_>=remain_timeout) {
                    result_.dns_errmsg_.append(std::string("GetHostByName timeout, set timeout:")+string_cast(remain_timeout).str()+"ms");
                } else {
                    result_.dns_errmsg_.append("GetHostByName error;");
                }
                return;
            }
            remain_timeout -= result_.dns_cost_;
            if (remain_timeout<=0) {
                xwarn2(TSF"@%_ HTTPDetect timeout, dns_cost:%_, total_timeout:%_", this, result_.dns_cost_, req_.total_timeout_);
                result_.is_timeout_ = true;
                result_.logic_error_msg_.append("HTTPDetect total timeout after dns resolve.");
                return;
            }
        }
    } else {
        svr_ip = req_.prior_ip_.at(0);
        xinfo2(TSF"@%_ use prior ip %_", this, svr_ip);
    }
    
    detect_tick.gettickcount();
    
    socket_address svr_addr(svr_ip.c_str(), port);
    result_.connect_ip_port_ = svr_addr.url();
    
    int conn_timeout = std::min(kComplexConnectTimeout, (int)remain_timeout);
    xdebug2(TSF"@%_, con_timeout:%_", this, conn_timeout);
    ComplexConnect conn(conn_timeout, kComplexConnectInterval);
    std::vector<socket_address> vecaddr;
    vecaddr.push_back(svr_addr);
    result_.tcp_start_connect_time_ = GetCurTimeStr();
    SOCKET sock = conn.ConnectImpatient(vecaddr, breaker_, NULL);
    result_.tcp_connect_cost_ = detect_tick.gettickspan();
    remain_timeout -= result_.tcp_connect_cost_;
    
    if (INVALID_SOCKET==sock) {
        if (breaker_.IsBreak()) {
            result_.logic_error_msg_.append("user interrupt detect when tcp connect");
        } else if (detect_tick.gettickspan() >= kComplexConnectTimeout) {
            result_.tcp_connect_errno_ = SOCKET_ERRNO(ETIMEDOUT);
            result_.tcp_connect_errmsg_ = strerror(SOCKET_ERRNO(ETIMEDOUT));
        } else {
            result_.tcp_connect_errno_ = socket_errno;
            result_.tcp_connect_errmsg_ = strerror(socket_errno);
        }
        xerror2(TSF"tcp connect error, isbreak:%_, errno:%_(%_)", breaker_.IsBreak(), result_.tcp_connect_errno_, result_.tcp_connect_errmsg_);
        return;
    }
    
    if (remain_timeout<=0) {
        xwarn2(TSF"@%_ HTTPDetect timeout, tcp_connect_cost_:%_, total_timeout:%_", this, result_.tcp_connect_cost_, req_.total_timeout_);
        result_.logic_error_msg_.append("HTTPDetect total timeout after tcp connect.");
        return;
    }
    
    int err_code = -1;
    
    // send request
    detect_tick.gettickcount();
    result_.http_send_req_time_ = GetCurTimeStr();
    int send_ret = block_socket_send(sock, (const unsigned char*)req_buffer.Ptr(), (unsigned int)req_buffer.Length(), breaker_, err_code);
    
    if (send_ret < 0) {
        xerror2(TSF"Send Request Error, ret:%_, errno:%_, nread:%_, nwrite:%_", send_ret, strerror(err_code), socket_nread(sock), socket_nwrite(sock));
        result_.tcp_rw_errno_ = err_code;
        result_.tcp_rw_errmsg_ = strerror(err_code);
        socket_close(sock);
        return;
    }
    
    
    if (breaker_.IsBreak()) {
        xwarn2(TSF"Send Request break, sent:%_ nread:%_, nwrite:%_", send_ret, socket_nread(sock), socket_nwrite(sock));
        result_.logic_error_msg_.append("user interrupt detect when tcp send;\n");
        socket_close(sock);
        return;
    }
    result_.http_send_req_cost_ = detect_tick.gettickspan();
    result_.http_req_packet_size_ = send_ret;
    xassert2(send_ret == (int)req_buffer.Length());
    remain_timeout -= result_.http_send_req_cost_;
    if (remain_timeout<=0) {
        xwarn2(TSF"@%_ HTTPDetect timeout, http_send_req_cost_:%_, total_timeout:%_", this, result_.http_send_req_cost_, req_.total_timeout_);
        result_.logic_error_msg_.append("HTTPDetect total timeout after send req.");
        return;
    }
    detect_tick.gettickcount();
    //recv response
    err_code = -1;
    AutoBuffer body;
    AutoBuffer recv_buf;
    AutoBuffer extension;
    int        status_code = -1;
   // off_t recv_pos = 0;
    MemoryBodyReceiver* receiver = new MemoryBodyReceiver(body);
    http::Parser parser(receiver, true);
    detect_tick.gettickcount();
    bool is_first_recv_packet = true;
    result_.http_recv_resp_time_ = GetCurTimeStr();
    while (true) {
        remain_timeout -= detect_tick.gettickspan();
        if (remain_timeout<=0) {
            xwarn2(TSF"@%_ HTTPDetect timeout, span:%_, total_timeout:%_", this, (int64_t)detect_tick.gettickspan(), req_.total_timeout_);
            result_.logic_error_msg_.append("HTTPDetect total timeout when recv resp.");
            return;
        }
        detect_tick.gettickcount();
        
        int recv_ret = block_socket_recv(sock, recv_buf, 8 * 1024, breaker_, err_code, 5000);
        if (recv_ret < 0) {
            xerror2(TSF"read block socket return false, error:%0, nread:%_, nwrite:%_", strerror(err_code), socket_nread(sock), socket_nwrite(sock));
            result_.tcp_rw_errno_ = err_code;
            result_.tcp_rw_errmsg_ = strerror(err_code);
            break;
        }
        
        if (breaker_.IsBreak()) {
            xinfo2(TSF"user cancel, nread:%_, nwrite:%_", socket_nread(sock), socket_nwrite(sock));
            result_.logic_error_msg_.append("user interrupt detect when tcp recv;\n");
            break;
        }
        
        if (recv_ret == 0 && SOCKET_ERRNO(ETIMEDOUT) == err_code) {
            xerror2(TSF"read timeout error:(%_,%_), nread:%_, nwrite:%_ ", err_code, strerror(err_code), socket_nread(sock), socket_nwrite(sock));
            continue;
        }
        if (recv_ret == 0) {
            xerror2(TSF"remote disconnect, nread:%_, nwrite:%_", socket_nread(sock), socket_nwrite(sock));
            result_.tcp_rw_errmsg_ = "remote disconnect";
            break;
        }
        
        if (recv_ret > 0) {
            xinfo2(TSF"recv len:%_ ", recv_ret);
            //recv_pos = recv_buf.Pos();
            if (is_first_recv_packet) {
                is_first_recv_packet = false;
                result_.first_packet_cost_ = detect_tick.gettickspan();
            }
        }
        
        Parser::TRecvStatus parse_status = parser.Recv(recv_buf.Ptr(recv_buf.Length() - recv_ret), recv_ret);
        if (parser.FirstLineReady()) {
            status_code = parser.Status().StatusCode();
            result_.http_status_code_ = status_code;
        }
        
        if (parse_status == http::Parser::kFirstLineError) {
            xerror2(TSF"http head not receive yet, but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(sock), socket_nwrite(sock));
            result_.http_errmsg_ = "http head not receive yet, but socket closed";
            break;
        }
        else if (parse_status == http::Parser::kHeaderFieldsError) {
            xerror2(TSF"parse http head failed, but socket closed, length:%_, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(sock), socket_nwrite(sock));
             result_.http_errmsg_ = "parse http head failed, but socket closed";
            break;
        }
        else if (parse_status == http::Parser::kBodyError) {
            xerror2(TSF"content_length_ != body.Length(), Head:%0, http dump:%1 \n headers size:%2" , parser.Fields().ContentLength(), xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size());
            XMessage xmsg;
            xmsg(TSF"content_length(%_) != body size(%_)", parser.Fields().ContentLength(),recv_buf.Length());
            result_.http_errmsg_ = xmsg.String();
            break;
        }
        else if (parse_status == http::Parser::kEnd) {
            if (status_code != 200) {
                xerror2(TSF"@%_, status_code != 200, code:%_, http dump:%_ \n headers size:%_", this, status_code, xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size());
            } else {
                xinfo2(TSF"@%_, headers size:%_, ", this, parser.Fields().GetHeaders().size());
            }
            result_.http_resp_header_ = parser.Fields().ToString();
            break;
        }
        else {
            xdebug2(TSF"http parser status:%_ ", parse_status);
        }
    }
    result_.http_recv_resp_cost_ = detect_tick.gettickspan();
    result_.http_resp_packet_size_ = recv_buf.Length();
    xdebug2(TSF"read with nonblock socket http response, length:%_, ", recv_buf.Length());

    
}
