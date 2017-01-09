/*
* Created on 2015-03-23
*	  Author: 闫国跃
*
*/

#include "util_func.h"

#ifdef LONGLINK_TASKMANAGER_TEST

#include "gtest/gtest.h"
#include "thread/thread.h"

#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "../longlink_task_manager.h"
#include "../MMNetSource.h"
#include "../network_logic.h"
#include "../longlink_svr_push.h"
#include "../MmNetComm.h"
#include "../anti_avalanche.h"

#include "../../mmcomm/active_logic.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"

#include "TestUtils.h"
#include "GtestEx.h"


static unsigned long  __ReadWriteTimeout(unsigned long  _first_pkg_timeout)
{
	unsigned int rate = (kMobile != getNetInfo())?kWifiMinRate:kGPRSMinRate;
	return  _first_pkg_timeout + 1000*kMaxRecvLen/rate;
}

static unsigned long  __FirstPkgTimeout(unsigned long  _init_first_pkg_timeout, unsigned int _sendlen, int _send_count)
{
	xassert2(3600*1000 >= _init_first_pkg_timeout);

	unsigned int rate = (kMobile != getNetInfo())?kWifiMinRate:kGPRSMinRate;
	unsigned int base_rw_timeout = (kMobile != getNetInfo())?kBaseFirstPackageWifiTimeout:kBaseFirstPackageGPRSTimeout;
	unsigned int max_rw_timeout = (kMobile != getNetInfo())?kMaxFirstPackageWifiTimeout:kMaxFirstPackageGPRSTimeout;

	unsigned int task_delay = (kMobile != getNetInfo())?kWifiTaskDelay:kGPRSTaskDelay;

	unsigned long ret = 0;

	if (0<_init_first_pkg_timeout)
	{
		ret = _init_first_pkg_timeout;
	}
	else
	{
		ret = 	base_rw_timeout + 1000*_sendlen/rate;
		ret = ret < max_rw_timeout? ret: max_rw_timeout;
	}

	ret += _send_count*task_delay;
	return ret;
}


static void OnLongLinkNetworkError(int _line, ErrCmdType _eErrType, int _nErrCode, const std::string& _ip, unsigned int _port)
{

}

static bool Check(const CNetCmd& acmd, const void* apBuffer, int anLen)
{
	return true;
}

static Condition sg_Condition;

unsigned int sg_taskCostTime = 0;
static int CallBack(int _from, ErrCmdType _eErrType, int _nErrCode, const AutoBuffer& _cookies, int _nHashCode, const CNetCmd& _cmd, unsigned int _taskcosttime)
{
	printf("form:%d, errType:%d errCode:%d hashCode:%d, costTime:%u\n", _from, _eErrType, _nErrCode, _nHashCode, _taskcosttime);
	sg_taskCostTime = _taskcosttime;
	
	sg_Condition.notifyOne();
	return 0;
}


static void initLongLinkTaskManager (CMMLongLinkTaskManager& _longLinkTaskManager, LongLinkSvrPush& _longLinkSvrPush)
{

	_longLinkTaskManager.funNotify = boost::bind(&LongLinkSvrPush::OnLongLinkResponse, &_longLinkSvrPush, _1, _2, _3);
	_longLinkTaskManager.funCallback = boost::bind(&CallBack, 0, _1, _2, _3, _4, _5, _6);
	_longLinkTaskManager.funAntiAvalancheCheck = boost::bind(Check, _1, _2, _3);
	_longLinkTaskManager.funmakeSureAuth = ::makeSureAuth;
	_longLinkTaskManager.funreq2Buf = ::req2Buf;
	_longLinkTaskManager.funbuf2Resp = ::buf2Resp;
	_longLinkTaskManager.LongLinkChannel().FuncNetworkReport = boost::bind(&OnLongLinkNetworkError, _1, _2, _3, _4, _5);
	_longLinkTaskManager.funNotifyNetworkError = boost::bind(&OnLongLinkNetworkError, _1, _2, _3, _4, _5);
}

class CDetour /* add ": public CMember" to enable access to member variables... */
{
public:
	bool Mine_SingleRespHandle(std::list<STCmdLongLinkTask>::iterator _it, ErrCmdType _eErrType,
		int _nErrCode, const AutoBuffer& _cookies, unsigned int _respLength, const LongLinkConnectionInfo& info);



	static bool (CDetour::* Real_SingleRespHandle)(std::list<STCmdLongLinkTask>::iterator _it, ErrCmdType _eErrType,
		int _nErrCode, const AutoBuffer& _cookies, unsigned int _respLength, const LongLinkConnectionInfo& info);


};

bool CDetour::Mine_SingleRespHandle(std::list<STCmdLongLinkTask>::iterator _it, ErrCmdType _eErrType,
									int _nErrCode, const AutoBuffer& _cookies, unsigned int _respLength, const LongLinkConnectionInfo& info)
{
	//printf("%s, %d \n", __FUNCTION__, __LINE__);
	sg_Condition.notifyOne();
	return (this->*Real_SingleRespHandle)(_it, _eErrType, _nErrCode, _cookies, _respLength, info);
}

bool (CDetour::* CDetour::Real_SingleRespHandle)(std::list<STCmdLongLinkTask>::iterator _it, ErrCmdType _eErrType,
												 int _nErrCode, const AutoBuffer& _cookies, unsigned int _respLength, const LongLinkConnectionInfo& info) = (bool (CDetour::*)(std::list<STCmdLongLinkTask>::iterator _it, ErrCmdType _eErrType,
												 int _nErrCode, const AutoBuffer& _cookies, unsigned int _respLength, const LongLinkConnectionInfo& info)) &CMMLongLinkTaskManager::__SingleRespHandle;

static bool (CDetour::* Mine_Target1)(std::list<STCmdLongLinkTask>::iterator _it, ErrCmdType _eErrType, 
									  int _nErrCode, const AutoBuffer& _cookies, unsigned int _respLength, const LongLinkConnectionInfo& info) = &CDetour::Mine_SingleRespHandle;

static void HookSingleRespHandle()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real_SingleRespHandle, *(PBYTE*)&Mine_Target1);
	DetourTransactionCommit();
}

static void UnHookSingleRespHandle()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());  
	DetourDetach(&(PVOID&)CDetour::Real_SingleRespHandle, *(PBYTE*)&Mine_Target1);
	DetourTransactionCommit();
}


//task default time out:60000ms
PUBC_TEST(MMLongLinkTaskMangerTest, test0)
{
	UtilFunc::execute_cmd();
	//xlogger_SetLevel(ELevelDebug);
	//appender_open(EAppednerSync, "C:\\log", "GTEST");
	m_testCaseInfo->m_TestCaseName = "task default time out:60000ms";

	CMMNetSource netsource;
	ActiveLogic activelogic;
	LongLinkSvrPush longlinksvrpush;

	CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);

	initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);
	

	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;

	longLinkTaskManager.StartTask(1, cmd);

	uint64_t before = gettickcount();
	sg_Condition.wait(100 * 1000);
	uint64_t after = gettickcount();

	m_testCaseInfo->m_WorkerExpectTimeCost = 60 * 1000;
	m_testCaseInfo->m_WorkerRealTimeCost = after - before;

	BaseScene baseScene;
	baseScene.eventName = "use default expectfinishtime";
	PUBC_EXPECT_TRUE(sg_taskCostTime < 65000 && sg_taskCostTime > 6000, &baseScene);

}

//task user define task timeout:30000ms
PUBC_TEST(MMLongLinkTaskMangerTest, test1)
{
	m_testCaseInfo->m_TestCaseName = "task user define task timeout:30000ms";

	CMMNetSource netsource;
	ActiveLogic activelogic;
	LongLinkSvrPush longlinksvrpush;

	CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);

	initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);

	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;
	cmd.cmduser_expectfinishtime = 30 * 1000;


	longLinkTaskManager.StartTask(1, cmd);

	uint64_t before = gettickcount();
	sg_Condition.wait(40 * 1000);
	uint64_t after = gettickcount();

	m_testCaseInfo->m_WorkerExpectTimeCost = 30 * 1000;
	m_testCaseInfo->m_WorkerRealTimeCost = after - before;
	
	BaseScene baseScene;
	baseScene.eventName = "set expectfinishtime 30*1000ms";
	PUBC_EXPECT_TRUE(sg_taskCostTime < 35000 && sg_taskCostTime > 3000, &baseScene);

}


PUBC_TEST(MMLongLinkTaskMangerTest, test2)
{

	//xlogger_SetLevel(ELevelDebug);
	//appender_open(EAppednerSync, "C:\\log", "GTEST");
	m_testCaseInfo->m_TestCaseName = "default first pkg timeout";

	HookSingleRespHandle();

	CMMNetSource::SetDebugLongIP("127.0.0.100");
	CMMNetSource::SetDebugLongPort(8090);


	CMMNetSource netsource;
	ActiveLogic activelogic;
	LongLinkSvrPush longlinksvrpush;
	activelogic.OnForeground(true);

	CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);

	initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);

	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;
	cmd.cmduser_retrycount = 0;

	longLinkTaskManager.StartTask(1, cmd);

	for (int i=0;i<=cmd.cmduser_retrycount;++i)
	{
		BaseScene scene;
		uint64_t before = gettickcount();
		sg_Condition.wait(40 * 1000);
		uint64_t after = gettickcount();
		uint64_t costTime = after - before;

		scene.eventName = "respone delay 20s";
		PUBC_EXPECT_TRUE(costTime > __FirstPkgTimeout(0, 16, 0) && costTime < __FirstPkgTimeout(0, 16, 0) + 2000, &scene);
	}

	UnHookSingleRespHandle();
}

PUBC_TEST(MMLongLinkTaskMangerTest, test3)
{
	//xlogger_SetLevel(ELevelDebug);
	//appender_open(EAppednerSync, "C:\\log", "GTEST");
	m_testCaseInfo->m_TestCaseName = "user define first pkg timeout";

	HookSingleRespHandle();

	CMMNetSource::SetDebugLongIP("127.0.0.101");
	CMMNetSource::SetDebugLongPort(8091);

	CMMNetSource netsource;
	ActiveLogic activelogic;
	LongLinkSvrPush longlinksvrpush;
	activelogic.OnForeground(true);

	CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);

	initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);

	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;
	cmd.cmduser_retrycount = 0;
	cmd.cmduser_firstpkgtimeout = 10 * 1000;

	longLinkTaskManager.StartTask(1, cmd);

	for (int i=0;i<=cmd.cmduser_retrycount;++i)
	{

		uint64_t before = gettickcount();
		sg_Condition.wait(40 * 1000);
		uint64_t after = gettickcount();

		uint64_t costTime = after - before;
		unsigned long computerTimeout = __FirstPkgTimeout(10 * 1000, 16, 0);
		BaseScene scene;
		scene.eventName = "respone delay 20s";
		
		PUBC_EXPECT_TRUE(costTime > computerTimeout && costTime < computerTimeout + 2000, &scene);
		
	}

	UnHookSingleRespHandle();
}



PUBC_TEST(MMLongLinkTaskMangerTest, test4)
{
	
	m_testCaseInfo->m_TestCaseName = "read-write timeout, use default first pkg timeout";

	HookSingleRespHandle();

	CMMNetSource::SetDebugLongIP("127.0.0.102");
	CMMNetSource::SetDebugLongPort(8092);

	CMMNetSource netsource;
	ActiveLogic activelogic;
	LongLinkSvrPush longlinksvrpush;
	activelogic.OnForeground(true);

	CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);

	initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);

	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;
	cmd.cmduser_retrycount = 0;

	ThreadUtil::sleep(2);
	longLinkTaskManager.StartTask(1, cmd);

	for (int i=0;i<=cmd.cmduser_retrycount;++i)
	{

		uint64_t before = gettickcount();
		sg_Condition.wait(140 * 1000);
		uint64_t after = gettickcount();

		uint64_t costTime = after - before;
		unsigned long computerTimeout = __ReadWriteTimeout(__FirstPkgTimeout(0, 16, 0));
		BaseScene scene;
		scene.eventName = "respone 16 byte every 4 seconds";

		PUBC_EXPECT_TRUE(costTime > computerTimeout && costTime < computerTimeout + 2000, &scene);
		printf("%llu, %lu\n", after - before, computerTimeout);
	}

	UnHookSingleRespHandle();
}

PUBC_TEST(MMLongLinkTaskMangerTest, test5)
{

	m_testCaseInfo->m_TestCaseName = "read-write timeout, user define first pkg timeout";

	HookSingleRespHandle();

	CMMNetSource::SetDebugLongIP("127.0.0.103");
	CMMNetSource::SetDebugLongPort(8093);

	CMMNetSource netsource;
	ActiveLogic activelogic;
	LongLinkSvrPush longlinksvrpush;
	activelogic.OnForeground(true);

	CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);

	initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);

	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;
	cmd.cmduser_firstpkgtimeout = 10 * 1000;
	cmd.cmduser_retrycount = 0;

	ThreadUtil::sleep(2);
	longLinkTaskManager.StartTask(1, cmd);

	for (int i=0;i<=cmd.cmduser_retrycount;++i)
	{

		uint64_t before = gettickcount();
		sg_Condition.wait(140 * 1000);
		uint64_t after = gettickcount();

		uint64_t costTime = after - before;
		unsigned long computerTimeout = __ReadWriteTimeout(__FirstPkgTimeout(10 * 1000, 16, 0));
		BaseScene scene;
		scene.eventName = "respone 16 byte every 4 seconds";

		PUBC_EXPECT_TRUE(costTime > computerTimeout && costTime < computerTimeout + 2000, &scene);
		printf("%llu, %lu\n", after - before, computerTimeout);
	}
	//for coverage
	{
		CMMNetSource netsource;
		ActiveLogic activelogic;
		CMMLongLinkTaskManager longLinkTaskManager(&netsource, activelogic);
		LongLinkSvrPush longlinksvrpush;
		initLongLinkTaskManager(longLinkTaskManager, longlinksvrpush);

		CNetCmd cmd;
		cmd.reqCmdID = 100;
		cmd.respCmdID = 1001;
		cmd.networkType = 3;
		cmd.cmduser_firstpkgtimeout = 10 * 1000;
		cmd.cmduser_retrycount = 0;

		ThreadUtil::sleep(2);
		longLinkTaskManager.StartTask(6543, cmd);
		longLinkTaskManager.RedoTask();
		longLinkTaskManager.GetTaskCount();
		longLinkTaskManager.GetTasksContinuousFailCount();
		longLinkTaskManager.OnSessionTimeout();
		longLinkTaskManager.OnRedoNewDns();
	}
	CMMNetSource::SetDebugLongIP("");// avoid debug long ip impact on other testcase 
	CMMNetSource::SetDebugLongPort(0);
	UnHookSingleRespHandle();

}

#endif
