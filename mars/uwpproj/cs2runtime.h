#pragma once

#include "xlogger/xlogger.h"
#include "alarm.h"
#include "singleton.h"
#include "platform_comm.h"
#include "runtime2cs.h"
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


namespace mars
{

	public ref class MarsRuntimeComponent sealed
	{

	public:
		static bool Init(ICallback_Comm^ callBackForRuntime);
		static void OnCreate();
		static void OnDestroy();
		static void OnSingalCrash(int _sig);
		static void OnExceptionCrash();
		static void OnForeground(bool _isforeground);
		static void OnNetworkChange();
	};

#pragma region stn 

	public ref struct  TaskRuntime sealed {

		//require
		property int32       taskid;
		property int32       cmdid;
		property int32        channel_select;
		property Platform::String^    cgi;    // user

											  //optional
		property bool    send_only;  // user
		property bool    need_authed;  // user
		property bool    limit_flow;  // user
		property bool    limit_frequency;  // user

		property bool        network_status_sensitive;  // user
		property int32     channel_strategy;
		property int32     priority;  // user

		property int32     retry_count;  // user
		property int32     server_process_cost;  // user
		property int32     total_timetout;  // user ms

		property int       user_context;  // user
		property Platform::String^ report_arg;  // user for cgi report

		property Platform::Array<Platform::String^>^ shortlink_host_list;
	};

	public ref class StnComponent sealed
	{
	public:

		static void SetClientVersion(uint32 ver);

		static void SetShortlinkSvrAddr(uint16 port);

		static void SetLonglinkSvrAddr(Platform::String^ host, const Platform::Array<uint16>^ ports);

		// 'host' will be ignored when 'debugip' is not empty.
		static void SetLonglinkSvrAddr(Platform::String^ host, const Platform::Array<uint16>^ ports, Platform::String^ debugip);

		// 'task.host' will be ignored when 'debugip' is not empty.
		static void SetShortlinkSvrAddr(uint16 port, Platform::String^ debugip);

		// setting debug ip address for the corresponding host
		static void SetDebugIP(Platform::String^ host, Platform::String^ ip);

		// setting backup iplist for the corresponding host
		// if debugip is not empty, iplist will be ignored.
		// iplist will be used when newdns/dns ip is not available.
		static void SetBackupIPs(Platform::String^ host, const Platform::Array<Platform::String^>^ iplist);

		static void StartTask(TaskRuntime^ task);

		// sync function
		static void StopTask(int32 taskid);

		// check whether task's list has the task or not.
		static bool HasTask(int32 taskid);

		// reconnect longlink and redo all task
		// when you change svr ip, you must call this function.
		static void RedoTasks();

		// stop and clear all task
		static void ClearTasks();

		// the same as ClearTasks(), but also reinitialize network.
		static void Reset();

		//setting signalling's parameters.
		//if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
		static void SetSignallingStrategy(long long period, long long keeptime);

		// used to keep longlink active
		// keep signnaling once 'period' and last 'keeptime'
		static void KeepSignalling();

		static void StopSignalling();

		// connect quickly if longlink is not connected.
		static void MakesureLonglinkConnected();

		static bool LongLinkIsConnected();

		// noop is used to keep longlink conected
		// get noop taskid
		static uint32 GetNoopTaskID();
	};
#pragma endregion

#pragma region log

	public ref struct  LogGetPathRet sealed {
		property Platform::String^ logPath;    		
		property bool bRet; 
	};

	public enum class TAppenderModeRuntime { kAppednerAsync = 0, kAppednerSync = 1 };
	
	public enum class TLogLevelRuntime {
		kLevelAll = 0,
		kLevelVerbose = 0,
		kLevelDebug,    // Detailed information on the flow through the system.
		kLevelInfo,     // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
		kLevelWarn,     // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
		kLevelError,    // Other runtime errors or unexpected conditions.
		kLevelFatal,    // Severe errors that cause premature termination.
		kLevelNone,     // Special level used to disable all log messages.
	};

	public ref class LogComponent sealed
	{
	public:
		static void AppenderOpen(TAppenderModeRuntime _mode, Platform::String^ _dir, Platform::String^ _nameprefix);
		static void AppenderOpenWithCache(TAppenderModeRuntime _mode, Platform::String^ _cachedir, Platform::String^ _logdir, Platform::String^ _nameprefix);
		static void AppenderFlush();
		static void AppenderFlushSync();
		static void AppenderClose();
		static void AppenderSetMode(TAppenderModeRuntime _mode);
		static bool AppenderGetFilePathFromTimeSpan(int _timespan, Platform::String^ _prefix, const Platform::Array<Platform::String^>^ _filepath_vec);
		static LogGetPathRet^ AppenderGetCurrentLogPath();
		static LogGetPathRet^ AppenderGetCurrentLogCachePath();
		static void AppenderSetConsoleLog(bool _is_open);
		static void SetLogLevel(TLogLevelRuntime _level);
		static TLogLevelRuntime GetLogLevel();
		static void LogWrite(TLogLevelRuntime _level, Platform::String^ _tag, Platform::String^ _filename, Platform::String^ _funcname, int _line, intmax_t _pid, intmax_t _tid, intmax_t _maintid, Platform::String^ _log);
	};
#pragma endregion

#pragma region sdt
	public ref class SdtComponent sealed
	{
	public:
		static void SetHttpNetcheckCGI(Platform::String^ cgi);
	};
#pragma endregion

}

