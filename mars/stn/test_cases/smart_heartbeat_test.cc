/*
* Created on 2015-04-01
*	  Author: 闫国跃
*
*/


#include "util_func.h"

#ifdef SMARTHEARTBEAT_TEST

#include "gtest/gtest.h"
#include "thread/thread.h"

#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "../smart_heartbeat.h"
#include "../../mmcomm/active_logic.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"
#include "../../../comm/platform_comm.h"

#include "TestUtils.h"
#include "GtestEx.h"


class CDetour /* add ": public CMember" to enable access to member variables... */
{
public:
	bool Mine_IsActive();

	static bool (CDetour::* Real_IsActive)();
};

static bool sg_active = false;
bool CDetour::Mine_IsActive()
{
	return sg_active;
}

bool (CDetour::* CDetour::Real_IsActive)() = (bool (CDetour::*)())&ActiveLogic::IsActive;
static bool (CDetour::* Mine_Target1)() = &CDetour::Mine_IsActive;


static void HookIsActive()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real_IsActive, *(PBYTE*)&Mine_Target1);
	DetourTransactionCommit();
}

static void UnHookIsActive()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());  
	DetourDetach(&(PVOID&)CDetour::Real_IsActive, *(PBYTE*)&Mine_Target1);
	DetourTransactionCommit();
}

PUBC_TEST(SmartHeartBeat, test0)
{
	UtilFunc::execute_cmd();

	m_testCaseInfo->m_TestCaseName = "inactive, success three times continuously, hearbeat interval increase";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	//xlogger_SetLevel(ELevelDebug);
	//appender_open(EAppednerSync, "C:\\log", "GTEST");
	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;
	PUBC_EXPECT_EQ((unsigned int)MinHeartInterval, smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	//smartHeartbeat.OnHeartbeatStart();
	
	for (int i=0;i<3;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) three times continuously";
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	UnHookIsActive();
}


PUBC_TEST(SmartHeartBeat, test1)
{
	m_testCaseInfo->m_TestCaseName = "inactive, fail three times continuously, heartbeat interval decrease";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;

	//smartHeartbeat.OnHeartbeatStart();

	for (int i=0;i<6;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) six times continuously";
	//printf("%d\n", smartHeartbeat.getNextHeartbeatInterval(flag));
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	for (int i=0;i<3;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(false, false);
	}
	scene.eventName = "OnHeartResult(false, false) three times continuously";
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep - SuccessStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}


PUBC_TEST(SmartHeartBeat, test2)
{
	m_testCaseInfo->m_TestCaseName = "inactive, fail six times continuously, use MinHeartInterval";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");


	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;

	for (int i=0;i<6;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) six times continuously";
//	printf("%d\n", smartHeartbeat.getNextHeartbeatInterval(flag));
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	for (int i=0;i<6;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(false, false);
	}
	scene.eventName = "OnHeartResult(false, false) six times continuously";

	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}


PUBC_TEST(SmartHeartBeat, test3)
{
	m_testCaseInfo->m_TestCaseName = "inactive, heart beat the minimum is MinHeartInterval, the maximum is (MaxHeartInterval-SuccessStep)";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;
	BaseScene scene;

	for (int i=0;i<300;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) 300 times continuously";
	PUBC_EXPECT_EQ((unsigned int)(MaxHeartInterval-SuccessStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	for (int i=0;i<600;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(false, false);
	}

	scene.eventName = "OnHeartResult(false, false) 600 times continuously";


	//printf("%d\n", smartHeartbeat.getNextHeartbeatInterval(flag));

	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}


PUBC_TEST(SmartHeartBeat, test4)
{
	m_testCaseInfo->m_TestCaseName = "inactive, after MMSmartHeartbeat::OnLongLinkEstablished(), use MinHeartInterval, if success, use the previous value";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;

	for (int i=0;i<5;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) 5 times continuously";
	//printf("%d\n", smartHeartbeat.getNextHeartbeatInterval(flag));
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);


	scene.eventName = "smartHeartbeat.OnLongLinkEstablished()";
	smartHeartbeat.OnLongLinkEstablished();
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "OnHeartResult(true, false) 1 times";
	smartHeartbeat.OnHeartResult(true, false);
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "OnHeartResult(true, false) 1 times";
	
	smartHeartbeat.OnHeartResult(true, false);
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}

PUBC_TEST(SmartHeartBeat, test5)
{
	m_testCaseInfo->m_TestCaseName = "inactive, after MMSmartHeartbeat::OnLongLinkEstablished(), use MinHeartInterval, if fail three times continuously, use MinHeartInterval, or use the previous value";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;

	for (int i=0;i<6;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) 6 times continuously";
	//printf("%d\n", smartHeartbeat.getNextHeartbeatInterval(flag));
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "smartHeartbeat.OnLongLinkEstablished()";
	smartHeartbeat.OnLongLinkEstablished();
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	
	for (int i=0;i<2;++i)
	{
		smartHeartbeat.OnHeartResult(false, false);
	}

	scene.eventName = "OnHeartResult(false, false) 2 times";
	
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "OnHeartResult(true, false) 1 times";
	smartHeartbeat.OnHeartResult(true, false);
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);


	smartHeartbeat.OnLongLinkEstablished();
	for (int i=0;i<3;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(false, false);
	}
	scene.eventName = "smartHeartbeat.OnLongLinkEstablished(), OnHeartResult(false, false) 3 times";

	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}

PUBC_TEST(SmartHeartBeat, test6)
{
	m_testCaseInfo->m_TestCaseName = "inactive, after MMSmartHeartbeat::OnLongLinkDisconnect(), use MinHeartInterval, if success, use the previous value";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;

	for (int i=0;i<5;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) 5 times continuously";
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);


	scene.eventName = "smartHeartbeat.OnLongLinkDisconnect()";
	smartHeartbeat.OnLongLinkDisconnect();
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "OnHeartResult(true, false) 1 times";
	smartHeartbeat.OnHeartResult(true, false);
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "OnHeartResult(true, false) 3 times";
	for (int i=0;i<3;++i) {
		smartHeartbeat.OnHeartResult(true, false);
	}
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}

PUBC_TEST(SmartHeartBeat, test7)
{
	m_testCaseInfo->m_TestCaseName = "inactive, after MMSmartHeartbeat::OnLongLinkDisconnect(), use MinHeartInterval, if fail three times continuously, use MinHeartInterval, or use the previous value";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = false;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;

	BaseScene scene;

	for (int i=0;i<6;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) 6 times continuously";
	//printf("%d\n", smartHeartbeat.getNextHeartbeatInterval(flag));
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "smartHeartbeat.OnLongLinkDisconnect()";
	smartHeartbeat.OnLongLinkDisconnect();
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	for (int i=0;i<2;++i)
	{
		smartHeartbeat.OnHeartResult(false, false);
	}

	scene.eventName = "OnHeartResult(false, false) 2 times";

	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	scene.eventName = "OnHeartResult(true, false) 1 times";
	smartHeartbeat.OnHeartResult(true, false);
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval + HeartStep * 2), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);


	smartHeartbeat.OnLongLinkDisconnect();
	for (int i=0;i<3;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(false, false);
	}
	scene.eventName = "smartHeartbeat.OnLongLinkDisconnect(), OnHeartResult(false, false) 3 times";

	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);
	UnHookIsActive();
}

PUBC_TEST(SmartHeartBeat, test8)
{
	m_testCaseInfo->m_TestCaseName = "active, heart beat use MinHeartInterval";
	UtilFunc::del_files(getAppFilePath() + "/" + "config");

	HookIsActive();

	sg_active = true;
	SmartHeartBeat smartHeartbeat;

	bool flag = false;
	BaseScene scene;

	for (int i=0;i<300;++i)
	{
		smartHeartbeat.getNextHeartbeatInterval(flag);
		smartHeartbeat.OnHeartResult(true, false);
	}

	scene.eventName = "OnHeartResult(true, false) 300 times continuously";
	PUBC_EXPECT_EQ((unsigned int)(MinHeartInterval), smartHeartbeat.getNextHeartbeatInterval(flag), &scene);

	UnHookIsActive();
	//for coverage
	smartHeartbeat.m_currentNetHeartInfo.m_isStable = true;
	smartHeartbeat.OnHeartbeatStart();
	smartHeartbeat.judgeMIUIStyle();
	smartHeartbeat.reportSuccRate();
	smartHeartbeat.OnNetworkChanged();
}
#endif
