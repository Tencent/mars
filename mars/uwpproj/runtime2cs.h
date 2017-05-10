#pragma once

#include "xlogger/xlogger.h"
#include "alarm.h"
#include "singleton.h"
#include "platform_comm.h"
//#include "ActiveLogic.h"

#define NETWORK

#ifdef NETWORK
//#include "MMNetCore.h"
//#include "MMStatReport.h"
#endif

//#include "MMLogReport.h"
//#include "MMLogLogic.h"
#include "log/appender.h"


#define KVCOMM

#ifdef KVCOMM
//#include "MMKVCommCore.h"
#endif


namespace mars {
	
		public ref struct AccountInfoRet sealed
		{
			property uint64 uin;
			property Platform::String^ username;
		};

		public ref struct DeviceInfoRet sealed
		{
			property Platform::String^ devicename;
			property Platform::String^ devicetype;
		};

		public ref struct Req2BufRet sealed
		{
			property Platform::Array<uint8>^ outbuffer;
			property bool bRet;
			property int nErrCode;
		};

		public ref struct Buf2RespRet sealed
		{
			property int bRet;
			property int nErrCode;

		};

		public ref struct Buf2RespArg sealed
		{
			property int taskid;
			property int user_context;
			property Platform::Array<uint8>^ inbuffer;
			property int error_code;
			property int channel_select;

		};

		public enum class LonglinkCheckType { ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2 };
		public ref struct GetLonglinkIdentifyRet sealed
		{
			property Platform::Array<uint8>^ identify_buffer;
			property Platform::Array<uint8>^ buffer_hash;
			property int32 cmdid;
			property LonglinkCheckType nRet;
		};

		public ref struct ProxyInfo sealed
		{
			property Platform::String^ strProxy;
			property Platform::String^ _host;
			property int port;
		};

		public ref struct CurWifiInfo sealed
		{
			property Platform::String^ ssid;
			property Platform::String^ bssid;
		};

		public ref struct CurRadioAccessNetworkInfo sealed
		{
			property Platform::String^ radio_access_network;
		};

		public ref struct CurSIMInfo sealed
		{
			property Platform::String^ isp_code;
			property Platform::String^ isp_name;
		};

		public ref struct CurAPNInfo sealed
		{
			property int nettype;
			property int sub_nettype;
			property Platform::String^ extra_info;
		};

		public interface class ICallback_Comm
		{
			////////////////////////stn callback
			bool MakesureAuthed();
			//流量统计 
			void TrafficData(int _send, int _recv);
			//底层询问上层该host对应的ip列表 
			Platform::Array<Platform::String^>^ OnNewDns(Platform::String^ host);
			//网络层收到push消息回调 
			void OnPush(int cmdid, const Platform::Array<uint8>^ msgpayload);
			//底层获取task要发送的数据 
			Req2BufRet^ Req2Buf(int taskid, int user_context, int error_code, int channel_select);
			//底层回包返回给上层解析 
			Buf2RespRet^ Buf2Resp(int taskid, int user_context, const Platform::Array<uint8>^ inbuffer, int error_code, int channel_select);
			//任务执行结束 
			int  OnTaskEnd(int taskid, int user_context, int error_type, int error_code);
			//上报网络连接状态 
			void ReportConnectStatus(int status, int longlink_status);
			//长连信令校验 ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2 
			GetLonglinkIdentifyRet^  GetLonglinkIdentifyCheckBuffer();
			//长连信令校验回包 
			bool OnLonglinkIdentifyResponse(const Platform::Array<uint8>^ response_buffer, const Platform::Array<uint8>^ identify_buffer_hash);

			void RequestSync();

			//验证是否已登录
			bool IsLogoned();
			/////////////////////////////////////////////////////


			////////////////////////platfrom comm callback
			ProxyInfo^ GetProxyInfo();

			int GetNetInfo();

			CurRadioAccessNetworkInfo^ GetCurRadioAccessNetworkInfo();

			CurWifiInfo^ GetCurWifiInfo();

			CurSIMInfo^ GetCurSIMInfo();

			CurAPNInfo^ GetAPNInfo();

			unsigned int GetSignal(bool bIsWifi);

			bool IsNetworkConnected();	

			void ConsoleLog(int logLevel, Platform::String^ tag, Platform::String^ filename, Platform::String^ funcname, int line, Platform::String^ log);
			//////////////////////

			//////////////////////////app callback
			Platform::String^ GetAppFilePath();

			AccountInfoRet^ GetAccountInfo();

			unsigned int GetClientVersion();

			DeviceInfoRet^ GetDeviceInfo();
			//////////////////////////

		};

		public ref class Runtime2Cs_Comm sealed
		{
		public:
			static Runtime2Cs_Comm^ Singleton();

			void SetCallback(ICallback_Comm^ _callback);
			ICallback_Comm^ GetCallBack();

		private:
			Runtime2Cs_Comm(void);

		private:
			static Runtime2Cs_Comm^ instance;
			ICallback_Comm^ m_callback;
		};
}

