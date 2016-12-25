/*
 * baseprjevent.cpp
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#include "mars/baseevent/baseprjevent.h"

boost::signals2::signal<void ()>& GetSignalOnCreate()
{
	static boost::signals2::signal<void ()> SignalOnCreate;
	return SignalOnCreate;
}

boost::signals2::signal<void ()>& GetSignalOnDestroy()
{
	static boost::signals2::signal<void ()> SignalOnDestroy;
	return SignalOnDestroy;
}

boost::signals2::signal<void (int _sig)>& GetSignalOnSingalCrash()
{
	static boost::signals2::signal<void (int _sig)> SignalOnSingalCrash;
	return SignalOnSingalCrash;
}

boost::signals2::signal<void ()>& GetSignalOnExceptionCrash()
{
	static boost::signals2::signal<void ()> SignalOnExceptionCrash;
	return SignalOnExceptionCrash;
}

boost::signals2::signal<void (bool _isForeground)>& GetSignalOnForeground()
{
	static boost::signals2::signal<void (bool _isForeground)> SignalOnForeground;
	return SignalOnForeground;
}

boost::signals2::signal<void ()>& GetSignalOnNetworkChange()
{
	static boost::signals2::signal<void ()> SignalOnNetworkChange;
	return SignalOnNetworkChange;
}


boost::signals2::signal<void (const char* _tag, ssize_t _send, ssize_t _recv)>& GetSignalOnNetworkDataChange() {
    static boost::signals2::signal<void (const char* _tag, ssize_t _send, ssize_t _recv)> SignalOnNetworkDataChange;
    return SignalOnNetworkDataChange;
}
