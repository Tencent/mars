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
 * baseprjevent.h
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#ifndef BASEPRJEVENT_H_
#define BASEPRJEVENT_H_

#include "mars/comm/owl/signalslot/signalslot.h"

extern owl::xsignal<void ()>& GetSignalOnCreate();
extern owl::xsignal<void (int _encoder_version)>& GetSignalOnInit();
extern owl::xsignal<void ()>& GetSignalOnDestroy();
extern owl::xsignal<void (int _sig)>& GetSignalOnSingalCrash();
extern owl::xsignal<void ()>& GetSignalOnExceptionCrash();
extern owl::xsignal<void (bool _isForeground)>& GetSignalOnForeground();
extern owl::xsignal<void ()>& GetSignalOnNetworkChange();

extern owl::xsignal<void (const char* _tag, ssize_t _send, ssize_t _recv)>& GetSignalOnNetworkDataChange();
extern owl::xsignal<void (int64_t _id)>& GetSignalOnAlarm();

#endif /* BASEPRJEVENT_H_ */
