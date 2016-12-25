/*
 * active_logic.h
 *
 *  Created on: 2012-8-22
 *      Author: yerungui
 */

#ifndef MMCOMM_SRC_ACTIVE_LOGIC_H_
#define MMCOMM_SRC_ACTIVE_LOGIC_H_

#include "boost/signals2.hpp"

#include "mars/comm/alarm.h"
#include "mars/comm/thread/mutex.h"

class ActiveLogic
{
public:
    boost::signals2::signal<void (bool _isForeground)> SignalForeground;
    boost::signals2::signal<void (bool _isactive)> SignalActive;

public:
    ActiveLogic();
    virtual ~ActiveLogic();

    void OnForeground(bool _isforeground);
    bool IsActive() const;

    bool IsForeground() const;
    uint64_t LastForegroundChangeTime() const;

private:
    void __OnInActive();

private:
    bool   isforeground_;
    bool   isactive_;
    Alarm  alarm_;
    uint64_t lastforegroundchangetime_;
};

#endif // MMCOMM_SRC_ACTIVE_LOGIC_H_
