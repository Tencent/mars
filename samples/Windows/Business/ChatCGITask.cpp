#include <mars/comm/windows/projdef.h>
#include "ChatCGITask.h"
#include "proto/generate/chat.pb.h"
#include "mars/stn/stn_logic.h"
using namespace std;

bool ChatCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	string data;
	com::tencent::mars::sample::chat::proto::SendMessageRequest request;
	request.set_from(user_);
	request.set_to(to_);
	request.set_access_token(access_token_);
	request.set_topic(topic_);
	request.set_text(text_);
	request.SerializeToString(&data);
	_outbuffer.AllocWrite(data.size());
	_outbuffer.Write(data.c_str(), data.size());
	return true;
}
int ChatCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	com::tencent::mars::sample::chat::proto::SendMessageResponse response;
	response.ParseFromArray(_inbuffer.Ptr(), _inbuffer.Length());
	return mars::stn::kTaskFailHandleNoError;
}

