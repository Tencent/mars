#include <mars/comm/windows/projdef.h>
#include "GetConvListCGITask.h"
#include "mars/stn/stn_logic.h"
using namespace std;

bool GetConvListCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	string data;
	com::tencent::mars::sample::proto::ConversationListRequest request;
	request.set_access_token(access_token_);
	request.set_type(com::tencent::mars::sample::proto::ConversationListRequest_FilterType_DEFAULT);
	request.SerializeToString(&data);
	_outbuffer.AllocWrite(data.size());
	_outbuffer.Write(data.c_str(), data.size());
	return true;
}
int GetConvListCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	com::tencent::mars::sample::proto::ConversationListResponse response;
	response.ParseFromArray(_inbuffer.Ptr(), _inbuffer.Length());
	if (callback_.lock())callback_.lock()->OnResponse(this, response);
	return mars::stn::kTaskFailHandleNoError;
}

