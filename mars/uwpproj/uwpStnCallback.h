#pragma once
#include "stn/stn_logic.h"

class uwpStnCallback :public mars::stn::Callback
{
public:
	virtual ~uwpStnCallback() {}
	virtual bool MakesureAuthed();

	//流量统计
	virtual void TrafficData(ssize_t _send, ssize_t _recv);

	//底层询问上层该host对应的ip列表
	virtual std::vector<std::string> OnNewDns(const std::string& host);
	//网络层收到push消息回调
	virtual void OnPush(int32_t cmdid, const AutoBuffer& msgpayload);
	//底层获取task要发送的数据
	virtual bool Req2Buf(int32_t taskid, void* const user_context, AutoBuffer& outbuffer, int& error_code, const int channel_select);
	//底层回包返回给上层解析
	virtual int Buf2Resp(int32_t taskid, void* const user_context, const AutoBuffer& inbuffer, int& error_code, const int channel_select);
	//任务执行结束
	virtual int  OnTaskEnd(int32_t taskid, void* const user_context, int error_type, int error_code);

	//上报网络连接状态
	virtual void ReportConnectStatus(int status, int longlink_status);
	//长连信令校验 ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2
	virtual int  GetLonglinkIdentifyCheckBuffer(AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid);
	//长连信令校验回包
	virtual bool OnLonglinkIdentifyResponse(const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash);


	virtual void RequestSync();

	//验证是否已登录
	virtual bool IsLogoned();
};


