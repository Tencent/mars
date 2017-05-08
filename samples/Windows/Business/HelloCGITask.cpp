#include <mars/comm/windows/projdef.h>
#include "HelloCGITask.h"
#include "mars/stn/stn_logic.h"
using namespace std;

bool HelloCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	string data;
	com::tencent::mars::sample::proto::HelloRequest request;
	request.set_user(user_);
	request.set_text(text_);
	request.SerializeToString(&data);
	_outbuffer.AllocWrite(data.size());
	_outbuffer.Write(data.c_str(), data.size());
	return true;
}
int HelloCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	com::tencent::mars::sample::proto::HelloResponse response;
	response.ParseFromArray(_inbuffer.Ptr(), _inbuffer.Length());
	if (callback_.lock())callback_.lock()->OnResponse(this, response);
	return mars::stn::kTaskFailHandleNoError;
}

