#ifndef _MARS_SERVICE_PROXY_H_
#define _MARS_SERVICE_PROXY_H_
#include <queue>
#include <map>
#include <string>

#include "mars/comm/thread/thread.h"
#include "mars/comm/autobuffer.h"
#include "Wrapper/CGITask.h"
#include "NetworkObserver.h"

class NetworkService
{
public:
	static NetworkService& Instance();
	bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);
	int OnTaskEnd(uint32_t _taskid, void* const _user_context, int _error_type, int _error_code);
	void OnPush(uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend);

	void setClientVersion(uint32_t _client_version);
	void setShortLinkDebugIP(const std::string& _ip, unsigned short _port);
	void setShortLinkPort(unsigned short _port);
	void setLongLinkAddress(const std::string& _ip, unsigned short _port, const std::string& _debug_ip = "");
	void start();

	int startTask(CGITask* task);

	void setPushObserver(uint32_t _cmdid, PushObserver* _observer);
protected:
	NetworkService();
	~NetworkService();

	void __Init();
private:
	std::map<uint32_t, CGITask*> map_task_;
	std::map<uint32_t, PushObserver*> map_push_observer_;
};

#endif