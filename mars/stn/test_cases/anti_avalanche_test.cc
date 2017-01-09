#include <stdlib.h>

#include "util_func.h"

#if 1

#define ANTI_AVALANCHE_TEST
#ifdef ANTI_AVALANCHE_TEST
#include <stdio.h>
#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"

#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "../../../network/net_core.h"
#include "../../../network/shortlink.h"
#include "../../../network/host_info_logic.h"
#include "../../../network/anti_avalanche.h"
#include "../../mmcomm/new_getdns.h"
#include "../../mmcomm/active_logic.h"
#include "../../../comm/singleton.h"
#include "../../../comm/platform_comm.h"
#include "../MmNetComm.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"
#include "HookUtil.h"

//-------------------------------global variables--------------------------------------
static int sg_NetInfo = kMobile;



//-----------------------------begin hook----------------------------------------------
//---step1  Declare hook
DECLARE_HOOK_BEGIN
	DECLARE_HOOK_GLOBAL_FUNCTION0(getNetInfo, int);
DECLARE_HOOK_END

///---step2	Describe which function(the functions DECLARED above) should be Hook
ATTACH_HOOK_BEGIN
	ATTACH_HOOK_FUNCTION0(, getNetInfo, int);
ATTACH_HOOK_END

//-----step3 Describe which function(the functions DECLARED above) should be UnHook
DETACH_HOOK_BEGIN
	DETACH_HOOK_FUNCTION0(, getNetInfo, int);
DETACH_HOOK_END
///------step4 Implement the hook function for the real function
IMPLEMENT_MINE_HOOK_GLOBAL_FUNCITON0(getNetInfo, int)
{
	return sg_NetInfo;
}

//----------------------------end hook-------------------------------------


PUBC_TEST(MMAntiAvalanche_test, test0)
{
	//frequency
	ActiveLogic activeLogic;
	AntiAvalanche antiAvalanche(activeLogic.IsActive());
	bool isFrequencyLimitPass = true;
	CNetCmd acmd;
	char sendBuf[128] = {0};

	BaseScene baseScene;
	baseScene.setEventName("before FrequencyLimit check");
	PUBC_EXPECT_TRUE(isFrequencyLimitPass, &baseScene);
	int i = 0;
	for(i=0; i<200; i++)
	{
		isFrequencyLimitPass = antiAvalanche.Check(acmd, sendBuf, 128);
		if(!isFrequencyLimitPass) break;
	}
	baseScene.setEventName("send same buffer to check");
	PUBC_EXPECT_FALSE(isFrequencyLimitPass, &baseScene);
	int checkTimes = i;
	baseScene.setEventName("checkTimes which hit frequency limit");
	PUBC_EXPECT_EQ(105, checkTimes, &baseScene);
}

PUBC_TEST(MMAntiAvalanche_test, test1)
{
	//flow
	ActiveLogic activeLogic;
	activeLogic.m_isactive = true;
	BaseScene baseScene;
	baseScene.setEventName("set NetInfo = EMobile");
	PUBC_EXPECT_EQ((int)kMobile, sg_NetInfo, &baseScene);
	AntiAvalanche antiAvalanche(activeLogic.IsActive());
	bool isFlowLimitPass = true;
	CNetCmd acmd;
	char *sendBuf = (char *)malloc(1024*1024);
	int i = 0;
	for(i=0; i<200; i++)
	{
		isFlowLimitPass = antiAvalanche.Check(acmd, sendBuf, 1024*1024);
		if(!isFlowLimitPass) break;
	}
	baseScene.setEventName("send buffer(size=1M) every check");
	PUBC_EXPECT_FALSE(isFlowLimitPass, &baseScene);
	int checkTimes = i;
	baseScene.setEventName("checkTimes which hit flow limit");
	PUBC_EXPECT_EQ(8, checkTimes, &baseScene);

	free(sendBuf);
}

PUBC_TEST(MMAntiAvalanche_test, test2)
{
	//flow
	ActiveLogic activeLogic;
	activeLogic.m_isactive = true;
	sg_NetInfo = kWifi;
	BaseScene baseScene;
	baseScene.setEventName("set NetInfo = EWifi");
	PUBC_EXPECT_EQ((int)kWifi, sg_NetInfo, &baseScene);
	AntiAvalanche antiAvalanche(activeLogic.IsActive());
	bool isFlowLimitPass = true;
	CNetCmd acmd;
	char *sendBuf = (char *)malloc(1024*1024);
	int i = 0;
	for(i=0; i<100; i++)
	{
		isFlowLimitPass = antiAvalanche.Check(acmd, sendBuf, 1024*1024);
		if(!isFlowLimitPass) break;
	}

	int checkTimes = i;
	baseScene.setEventName("check times reach max(max==100)");
	PUBC_EXPECT_LT(8, checkTimes, &baseScene);

	baseScene.setEventName("send buffer(size=1M) every check");
	PUBC_EXPECT_TRUE(isFlowLimitPass, &baseScene);

	free(sendBuf);
}

#endif

#endif
