/*
 * traffic_statistics.cc
 *
 *  Created on: 2012-9-18
 *      Author: garryyan
 *  Copyright (c) 2013-2015 Tencent. All rights reserved.
 *
 */

#include "traffic_statistics.h"

#include <time.h>

#include "mars/app/app.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/platform_comm.h"

using namespace mars::app;

TrafficStatistics::TrafficStatistics()
: report_timeout_(10 * 1000/*10s*/)
, report_size_threshold_(10 * 1024/*10KB*/)
, wifi_recv_data_size_(0)
, wifi_send_data_size_(0)
, mobile_recv_data_size_(0)
, mobile_send_data_size_(0)
, last_report_time_(gettickcount())
{}

TrafficStatistics::TrafficStatistics(unsigned long _report_tmo, unsigned int _report_size_threshold)
    : report_timeout_(_report_tmo)
    , report_size_threshold_(_report_size_threshold)
    , wifi_recv_data_size_(0)
    , wifi_send_data_size_(0)
    , mobile_recv_data_size_(0)
    , mobile_send_data_size_(0)
    , last_report_time_(gettickcount())
{}

TrafficStatistics::~TrafficStatistics() {
    xinfo_function();
    // Flush();
}
void TrafficStatistics::SetCallback(const boost::function<void (int32_t, int32_t, int32_t, int32_t)>& _func_report_flow) {
    ScopedLock lock(mutex_);
    xassert2(!func_report_flow_);
    func_report_flow_ = _func_report_flow;
}
void TrafficStatistics::Flush() {
    ScopedLock lock(mutex_);
    __ReportData();
}

void TrafficStatistics::Data(unsigned int _send, unsigned int _recv) {

    ScopedLock lock(mutex_);

    if (0 < _send || 0 < _recv) {
        if (kMobile != getNetInfo())
            wifi_recv_data_size_ += _recv;
        else
            mobile_recv_data_size_ += _recv;

        if (kMobile != getNetInfo())
            wifi_send_data_size_ += _send;
        else
            mobile_send_data_size_ += _send;
    }

    if (__IsShouldReport()) __ReportData();
}

void TrafficStatistics::__ReportData() {
    if (func_report_flow_) {
        if (wifi_recv_data_size_>0 || wifi_send_data_size_ || mobile_recv_data_size_ || mobile_send_data_size_)
            func_report_flow_(wifi_recv_data_size_, wifi_send_data_size_, mobile_recv_data_size_, mobile_send_data_size_);
        xdebug2(TSF"wifi:%_, r:%_, mobile:s:%_, r:%_", wifi_send_data_size_, wifi_recv_data_size_, mobile_send_data_size_, mobile_recv_data_size_);
    } else {
        xassert2(false, TSF"wifi:s:%_, r:%_, mobile:s:%_, r:%_", wifi_send_data_size_, wifi_recv_data_size_, mobile_send_data_size_, mobile_recv_data_size_);
    }
    wifi_recv_data_size_ = 0;
    wifi_send_data_size_ = 0;
    mobile_recv_data_size_ = 0;
    mobile_send_data_size_ = 0;
    last_report_time_ = gettickcount();
}

bool TrafficStatistics::__IsShouldReport() const {
    if ((gettickcount() - last_report_time_ > report_timeout_)
            || (wifi_recv_data_size_ + wifi_send_data_size_ + mobile_recv_data_size_ + mobile_send_data_size_ > report_size_threshold_)) {
        return true;
    }

    return false;
}
