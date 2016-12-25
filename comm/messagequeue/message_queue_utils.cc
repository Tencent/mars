//
//  MessageQueueUtils.cpp
//  PublicComponent
//
//  Created by yerungui on 14-5-21.
//  Copyright (c) 2014年 Tencent. All rights reserved.
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
