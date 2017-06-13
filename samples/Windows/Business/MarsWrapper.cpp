#include "MarsWrapper.h"
#include "Wrapper/NetworkService.h"
#include "HelloCGITask.h"
#include "ChatCGITask.h"
#include "GetConvListCGITask.h"
#include "proto/generate/main.pb.h"
#include "proto/generate/messagepush.pb.h""

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
	NetworkService::Instance().setShortLinkDebugIP("127.0.0.1", 8080);
	NetworkService::Instance().setLongLinkAddress("127.0.0.1", 8081, "");
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
	task->host_ = "127.0.0.1";
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
	task->host_ = "127.0.0.1";
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
	task->host_ = "127.0.0.1";
	task->access_token_ = "";
	task->callback_ = _callback;
	NetworkService::Instance().startTask(task);
}