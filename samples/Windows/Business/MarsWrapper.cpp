// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
*  MarsWrapper.cpp
*
*  Created on: 2017-7-7
*      Author: chenzihao
*/

#include "MarsWrapper.h"
#include "Wrapper/NetworkService.h"
#include "HelloCGITask.h"
#include "ChatCGITask.h"
#include "GetConvListCGITask.h"
#include "proto/generate/main.pb.h"
#include "proto/generate/messagepush.pb.h"

static const char* g_host = "marsopen.cn";

MarsWrapper& MarsWrapper::Instance()
{
	static MarsWrapper instance_;
	return instance_;
}

MarsWrapper::MarsWrapper()
	: chat_msg_observer_(nullptr)
{
	
}

void MarsWrapper::OnPush(uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend)
{
	com::tencent::mars::sample::chat::proto::MessagePush msg;
	msg.ParseFromArray(_body.Ptr(), _body.Length());
	if (chat_msg_observer_)
	{
		ChatMsg chat_msg;
		chat_msg.topic_ = msg.topic();
		chat_msg.from_ = msg.from();
		chat_msg.content_ = msg.content();
		chat_msg_observer_->OnRecvChatMsg(chat_msg);
	}
}

void MarsWrapper::start()
{
	NetworkService::Instance().setClientVersion(200);
	NetworkService::Instance().setShortLinkDebugIP(g_host, 8080);
	NetworkService::Instance().setLongLinkAddress(g_host, 8081, "");
	NetworkService::Instance().start();	

	NetworkService::Instance().setPushObserver(com::tencent::mars::sample::proto::CMD_ID_PUSH, this);
}

void MarsWrapper::pingServer(const std::string& _name, const std::string& _text, boost::weak_ptr<HelloCGICallback> _callback)
{
	HelloCGITask* task = new HelloCGITask();
	task->user_ = _name;
	task->text_ = _text;
	task->callback_ = _callback;

	task->channel_select_ = ChannelType_All;
	task->cmdid_ = com::tencent::mars::sample::proto::CMD_ID_HELLO;
	task->cgi_ = "/mars/hello";
	task->host_ = g_host;
	NetworkService::Instance().startTask(task);
}


void MarsWrapper::setChatMsgObserver(ChatMsgObserver* _observer)
{
	chat_msg_observer_ = _observer;
}

void MarsWrapper::sendChatMsg(const ChatMsg& _chat_msg)
{
	ChatCGITask* task = new ChatCGITask();
	task->channel_select_ = ChannelType_LongConn;
	task->cmdid_ = com::tencent::mars::sample::proto::CMD_ID_SEND_MESSAGE;
	task->cgi_ = "/mars/sendmessage";
	task->host_ = g_host;
	task->text_ = _chat_msg.content_;

	task->user_ = _chat_msg.from_;
	task->to_ = "all";
	task->access_token_ = "123456";
	task->topic_ = _chat_msg.topic_;

	NetworkService::Instance().startTask(task);
}
void MarsWrapper::getConversationList(boost::weak_ptr<GetConvListCGICallback> _callback)
{
	GetConvListCGITask* task = new GetConvListCGITask();
	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = com::tencent::mars::sample::proto::CMD_ID_CONVERSATION_LIST;
	task->cgi_ = "/mars/getconvlist";
	task->host_ = g_host;
	task->access_token_ = "";
	task->callback_ = _callback;
	NetworkService::Instance().startTask(task);
}