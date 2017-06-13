#ifndef _HELLO_CGI_TASK_H_
#define _HELLO_CGI_TASK_H_

#include "Wrapper/CGITask.h"

#include "proto/generate/main.pb.h"
#include "mars/boost/weak_ptr.hpp"
class HelloCGITask;
class HelloCGICallback
{
public:
	virtual void OnResponse(HelloCGITask* task, const com::tencent::mars::sample::proto::HelloResponse& response) = 0;
};
class HelloCGITask : public CGITask 
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	std::string user_;
	std::string text_;
	boost::weak_ptr<HelloCGICallback> callback_;
};

#endif