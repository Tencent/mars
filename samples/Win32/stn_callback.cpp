#include "stdafx.h"
#include "stn_callback.h"

#include <vector>

namespace mars {
	namespace stn {
	
		bool StnCallBack::MakesureAuthed(){
			return false;
		}

		std::vector<std::string> StnCallBack::OnNewDns(const std::string& host) {
			std::vector<std::string> ips;
			return ips;
		}

		void StnCallBack::OnPush(int32_t cmdid, const AutoBuffer& msgpayload) {
			
		}

		bool StnCallBack::Req2Buf(int32_t taskid, void* const user_context, AutoBuffer& outbuffer, int& error_code, const int channel_select) {
			return true;
		}

		int StnCallBack::Buf2Resp(int32_t taskid, void* const user_context, const AutoBuffer& inbuffer, int& error_code, const int channel_select) {
			return kTaskFailHandleNormal;
		}

		int  StnCallBack::OnTaskEnd(int32_t taskid, void* const user_context, int error_type, int error_code) {
			return 0;
		}

		void StnCallBack::ReportFlow(int32_t wifi_recv, int32_t wifi_send, int32_t mobile_recv, int32_t mobile_send) {
		
		}

		void StnCallBack::ReportConnectStatus(int status, int longlink_status) {
		
		}

		int  StnCallBack::GetLonglinkIdentifyCheckBuffer(AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
			return 2;
		}

		bool StnCallBack::OnLonglinkIdentifyResponse(const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
			return true;
		}
	//
		void StnCallBack::RequestSync() {
		
		}

	}
}