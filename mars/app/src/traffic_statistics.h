/*
 * traffic_statistics.h
 *
 *  Created on: 2012-9-18
 *      Author: garryyan
 *  Copyright (c) 2013-2015 Tencent. All rights reserved.
 *
 */

#ifndef STN_SRC_TRAFFIC_STATISTICS_H_
#define STN_SRC_TRAFFIC_STATISTICS_H_

#include "boost/signals2.hpp"
#include "boost/function.hpp"

#include "mars/comm/thread/mutex.h"
#include "mars/comm/singleton.h"

namespace mars {
    namespace app {

class TrafficStatistics {

  public:
    TrafficStatistics();
    TrafficStatistics(unsigned long _report_tmo, unsigned int _report_size_threshold);
    ~TrafficStatistics();
    void Data(unsigned int _send, unsigned int _recv);
    void Flush();
    void SetCallback(const boost::function<void (int32_t, int32_t, int32_t, int32_t)>& _func_report_flow);
  private:

    TrafficStatistics(const TrafficStatistics&);
    TrafficStatistics& operator=(const TrafficStatistics&);

  private:
    void __ReportData();
    bool __IsShouldReport() const;

  private:
    const unsigned long report_timeout_;
    const unsigned int report_size_threshold_;
    
    boost::function<void (int32_t wifi_recv, int32_t wifi_send, int32_t mobile_recv, int32_t mobile_send)> func_report_flow_;
    
    unsigned int wifi_recv_data_size_;
    unsigned int wifi_send_data_size_;
    unsigned int mobile_recv_data_size_;
    unsigned int mobile_send_data_size_;
    uint64_t last_report_time_;
    Mutex mutex_;
};
    }
}

#endif // STN_SRC_TRAFFIC_STATISTICS_H_
