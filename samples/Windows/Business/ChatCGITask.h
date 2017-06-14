#ifndef _CHAT_CGI_TASK_H_
#define _CHAT_CGI_TASK_H_

#include "Wrapper/CGITask.h"

class ChatCGITask : public CGITask 
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	std::string user_;
	std::string to_;
	std::string text_;
	std::string access_token_;
	std::string topic_;
};

#endif