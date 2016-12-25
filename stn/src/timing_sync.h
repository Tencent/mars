/*
 * timing_sync.h
 *
 *  Created on: 2012-10-19
 *      Author: zhouzhijie
 *  Copyright (c) 2013-2015 Tencent. All rights reserved.
 *
 */

#ifndef STN_SRC_TIMING_SYNC_H_
#define STN_SRC_TIMING_SYNC_H_

#include "boost/function.hpp"
#include "boost/signals2.hpp"

#include "mars/baseevent/active_logic.h"
#include "mars/comm/alarm.h"
#include "mars/comm/thread/mutex.h"

#include "longlink.h"

namespace mars {
    namespace stn {

class TimingSync {
  public:
    TimingSync(ActiveLogic& _active_logic);
    ~TimingSync();

    void OnLongLinkStatuChanged(LongLink::TLongLinkStatus _status);
    void OnActiveChanged(bool _is_actived);
    void OnNetworkChange();

  private:
    void __OnAlarm();

  private:
    Alarm alarm_;

    ActiveLogic& active_logic_;
    boost::signals2::scoped_connection timing_sync_active_connection_;
};

    }
}

#endif // STN_SRC_TIMING_SYNC_H_
