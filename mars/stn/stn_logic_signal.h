//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_STN_LOGIC_SIGNAL_H
#define MMNET_STN_LOGIC_SIGNAL_H

#include "boost/signals2.hpp"

extern boost::signals2::signal<void ()>& GetStnLogicSignalOnCreate();
extern boost::signals2::signal<void (int _encoder_version)>& GetStnLogicSignalOnInitBeforeOnCreate();
extern boost::signals2::signal<void ()>& GetStnLogicSignalOnDestroy();
extern boost::signals2::signal<void (int _sig)>& GetStnLogicSignalOnSingalCrash();
extern boost::signals2::signal<void ()>& GetStnLogicSignalOnExceptionCrash();
extern boost::signals2::signal<void (bool _isForeground)>& GetStnLogicSignalOnForeground();
extern boost::signals2::signal<void ()>& GetStnLogicSignalOnNetworkChange();

extern boost::signals2::signal<void (const char* _tag, ssize_t _send, ssize_t _recv)>& GetStnLogicSignalOnNetworkDataChange();
extern boost::signals2::signal<void (int64_t _id)>& GetStnLogicSignalOnAlarm();

#endif // MMNET_STN_LOGIC_SIGNAL_H
