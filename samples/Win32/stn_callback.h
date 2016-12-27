// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "mars/stn/stn.h"
#include "mars/stn/stn_logic.h"

namespace mars {
	namespace stn {
	
		class StnCallBack : public Callback {

		public:
			StnCallBack() {};
			~StnCallBack() {};

			virtual bool MakesureAuthed();
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
			//上报流量数据
			virtual void ReportFlow(int32_t wifi_recv, int32_t wifi_send, int32_t mobile_recv, int32_t mobile_send);
			//上报网络连接状态
			virtual void ReportConnectStatus(int status, int longlink_status);
			//长连信令校验 ECHECK_NOW, ECHECK_NEXT = 1, ECHECK_NEVER = 2
			virtual int  GetLonglinkIdentifyCheckBuffer(AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid);
			//长连信令校验回包
			virtual bool OnLonglinkIdentifyResponse(const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash);
			//
			virtual void RequestSync();
		};

	}
}

