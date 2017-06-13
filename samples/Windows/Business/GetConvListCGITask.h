#ifndef _GETCONVLIST_CGI_TASK_H_
#define _GETCONVLIST_CGI_TASK_H_

#include "Wrapper/CGITask.h"

#include "proto/generate/main.pb.h"
#include "mars/boost/weak_ptr.hpp"
class GetConvListCGITask;
struct ConversationInfo
{
	std::string topic_;
	std::string notice_;
	std::string name_;
};
class GetConvListCGICallback
{
public:
	virtual void OnResponse(GetConvListCGITask* task, const com::tencent::mars::sample::proto::ConversationListResponse& response) = 0;
};
class GetConvListCGITask : public CGITask
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	std::string access_token_;
	boost::weak_ptr<GetConvListCGICallback> callback_;
};

#endif