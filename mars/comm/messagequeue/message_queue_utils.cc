// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  MessageQueueUtils.cpp
//  PublicComponent
//
//  Created by yerungui on 14-5-21.
//

#include "comm/messagequeue/message_queue.h"
#include "comm/messagequeue/message_queue_utils.h"


namespace MessageQueue {

ScopeRegister::ScopeRegister(const MessageHandler_t& _reg)
    : m_reg(new MessageHandler_t(_reg)) {}

ScopeRegister::~ScopeRegister() {
    Cancel();
    delete m_reg;
}

const MessageHandler_t& ScopeRegister::Get() const
{return *m_reg;}

void ScopeRegister::Cancel() const {
    UnInstallMessageHandler(*m_reg);
    CancelMessage(*m_reg);
}
void ScopeRegister::CancelAndWait() const {
    Cancel();
    WaitForRuningLockEnd(*m_reg);
}

}  // namespace MessageQueue
