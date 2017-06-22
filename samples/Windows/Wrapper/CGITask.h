#ifndef _CGI_TASK_H_
#define _CGI_TASK_H_
#include <map>
#include <string>

#include "mars/comm/autobuffer.h"

enum ChannelType 
{
	ChannelType_ShortConn = 1,
	ChannelType_LongConn = 2,
	ChannelType_All = 3
} ;
class CGITask
{
public:
	virtual ~CGITask() {};
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;


	uint32_t taskid_;
	ChannelType channel_select_;
	uint32_t cmdid_;
	std::string cgi_;
	std::string host_;
};

#endif