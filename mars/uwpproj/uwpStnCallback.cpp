#include "uwpStnCallback.h"
#include "runtime2cs.h"
#include "runtime_utils.h"


using namespace mars;



Platform::Array<uint8>^ bufferToPlatformArray(const AutoBuffer & autoBuffer)
{
	const void * buffer = autoBuffer.Ptr();
	int iLen = autoBuffer.Length();
	if (buffer == nullptr)
		return ref new Platform::Array<uint8>(0);

	Platform::Array<uint8>^  arrayBuffer = ref new Platform::Array<uint8>(iLen);
	uint8 * ptr = (uint8*)buffer;
	for (int i = 0; i< iLen; i++)
	{
		arrayBuffer[i] = ptr[i];
	}

	return arrayBuffer;

}


bool appendPlatfomrBufferToAutoBuffer(AutoBuffer& autoBuffer, Platform::Array<uint8>^ platformBuffer)
{
	if (platformBuffer == nullptr || platformBuffer->Length <= 0)
	{
		return false;
	}

	if (autoBuffer.Capacity() < (size_t)platformBuffer->Length)
	{
		autoBuffer.AddCapacity(platformBuffer->Length);
	}
	autoBuffer.Write(platformBuffer->Data, platformBuffer->Length);
	return true;
}

bool uwpStnCallback::MakesureAuthed()
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return false;
	}


	return callback->MakesureAuthed();
}

void uwpStnCallback::TrafficData(ssize_t _send, ssize_t _recv)
{
}


std::vector<std::string> uwpStnCallback::OnNewDns(const std::string& host)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();
	std::vector<std::string> iplist;
	if (nullptr == callback)
	{
		return iplist;
	}

	Platform::Array<Platform::String^>^ list = callback->OnNewDns(stdstring2String(host));
	for each (Platform::String^ cstring in list)
	{
		std::string tmp = String2stdstring(cstring);
		if (tmp.length() > 0)
		{
			iplist.push_back(tmp);
		}
	}
	return iplist;
}

void uwpStnCallback::OnPush(int32_t cmdid, const AutoBuffer & msgpayload)
{

	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return;
	}

	Platform::Array<uint8>^ buffer = bufferToPlatformArray(msgpayload);

	callback->OnPush(cmdid, buffer);
}

bool uwpStnCallback::Req2Buf(int32_t taskid, void * const user_context, AutoBuffer & outbuffer, int & error_code, const int channel_select)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return false;
	}


	Req2BufRet^ retInfo = callback->Req2Buf(taskid, (int)user_context, error_code, channel_select);
	error_code = retInfo->nErrCode;
	appendPlatfomrBufferToAutoBuffer(outbuffer, retInfo->outbuffer);
	return retInfo->bRet;
}

int uwpStnCallback::Buf2Resp(int32_t taskid, void * const user_context, const AutoBuffer & inbuffer, int & error_code, const int channel_select)
{

	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return 0;
	}

	Platform::Array<uint8>^ buffer = bufferToPlatformArray(inbuffer);

	Buf2RespRet^ retInfo = callback->Buf2Resp(taskid, (int)user_context, buffer, error_code, channel_select);
	error_code = retInfo->nErrCode;
	return retInfo->bRet;
}

int uwpStnCallback::OnTaskEnd(int32_t taskid, void * const user_context, int error_type, int error_code)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return 0;
	}

	return callback->OnTaskEnd(taskid, (int)user_context, error_type, error_code);

}

void uwpStnCallback::ReportConnectStatus(int status, int longlink_status)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return;
	}

	callback->ReportConnectStatus(status, longlink_status);
}

int uwpStnCallback::GetLonglinkIdentifyCheckBuffer(AutoBuffer & identify_buffer, AutoBuffer & buffer_hash, int32_t & cmdid)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return false;
	}


	GetLonglinkIdentifyRet^ retInfo = callback->GetLonglinkIdentifyCheckBuffer();
	cmdid = retInfo->cmdid;
	appendPlatfomrBufferToAutoBuffer(identify_buffer, retInfo->identify_buffer);
	appendPlatfomrBufferToAutoBuffer(buffer_hash, retInfo->buffer_hash);

	return (int)(retInfo->nRet);
}

bool uwpStnCallback::OnLonglinkIdentifyResponse(const AutoBuffer & response_buffer, const AutoBuffer & identify_buffer_hash)
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return false;
	}


	Platform::Array<uint8>^ platBufferResponse = bufferToPlatformArray(response_buffer);
	Platform::Array<uint8>^ platBufferHash = bufferToPlatformArray(identify_buffer_hash);


	return callback->OnLonglinkIdentifyResponse(platBufferResponse, platBufferHash);
}

void uwpStnCallback::RequestSync()
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return;
	}

	callback->RequestSync();
}

bool uwpStnCallback::IsLogoned()
{
	ICallback_Comm^ callback = Runtime2Cs_Comm::Singleton()->GetCallBack();

	if (nullptr == callback)
	{
		return false;
	}


	return callback->IsLogoned();
}
