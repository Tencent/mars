/*
 * baseprjevent.h
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#ifndef BASEPRJEVENT_H_
#define BASEPRJEVENT_H_

#include "boost/signals2.hpp"

extern boost::signals2::signal<void ()>& GetSignalOnCreate();
extern boost::signals2::signal<void ()>& GetSignalOnDestroy();
extern boost::signals2::signal<void (int _sig)>& GetSignalOnSingalCrash();
extern boost::signals2::signal<void ()>& GetSignalOnExceptionCrash();
extern boost::signals2::signal<void (bool _isForeground)>& GetSignalOnForeground();
extern boost::signals2::signal<void ()>& GetSignalOnNetworkChange();

extern boost::signals2::signal<void (const char* _tag, ssize_t _send, ssize_t _recv)>& GetSignalOnNetworkDataChange();

#endif /* BASEPRJEVENT_H_ */
