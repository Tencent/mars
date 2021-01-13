// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * baseprjevent.cpp
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#include "mars/baseevent/baseprjevent.h"

owl::xsignal<void ()>& GetSignalOnCreate()
{
	static owl::xsignal<void ()> SignalOnCreate;
	return SignalOnCreate;
}

owl::xsignal<void (int _encoder_version)>& GetSignalOnInitBeforeOnCreate(){
	static owl::xsignal<void (int _encoder_version)> SignalOnInit;
	return SignalOnInit;
}

owl::xsignal<void ()>& GetSignalOnDestroy()
{
	static owl::xsignal<void ()> SignalOnDestroy;
	return SignalOnDestroy;
}

owl::xsignal<void (int _sig)>& GetSignalOnSingalCrash()
{
	static owl::xsignal<void (int _sig)> SignalOnSingalCrash;
	return SignalOnSingalCrash;
}

owl::xsignal<void ()>& GetSignalOnExceptionCrash()
{
	static owl::xsignal<void ()> SignalOnExceptionCrash;
	return SignalOnExceptionCrash;
}

owl::xsignal<void (bool _isForeground)>& GetSignalOnForeground()
{
	static owl::xsignal<void (bool _isForeground)> SignalOnForeground;
	return SignalOnForeground;
}

owl::xsignal<void ()>& GetSignalOnNetworkChange()
{
	static owl::xsignal<void ()> SignalOnNetworkChange;
	return SignalOnNetworkChange;
}


owl::xsignal<void (const char* _tag, ssize_t _send, ssize_t _recv)>& GetSignalOnNetworkDataChange() {
    static owl::xsignal<void (const char* _tag, ssize_t _send, ssize_t _recv)> SignalOnNetworkDataChange;
    return SignalOnNetworkDataChange;
}

owl::xsignal<void (int64_t _id)>& GetSignalOnAlarm() {
    static owl::xsignal<void (int64_t _id)> SignalOnAlarm;
    return SignalOnAlarm;
}
