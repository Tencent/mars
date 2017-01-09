#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"

//#include "../../test/include/hook.h"
#include "include/mock_spy.h"

#include "utils.h"
#include "platform_comm.h"
#include "../src/longlink_connect_monitor.h"
#include "app.h"
#include "active_logic.h"
#include "active_logic.h"
#include "HookUtil.h"

using namespace mars:app;

static const unsigned long NONET_SALT_RATE = 3;
static const unsigned long NONET_SALT_RISE = 600;
static const unsigned long NOACCOUNTINFO_SALT_RATE = 2;
static const unsigned long NOACCOUNTINFO_SALT_RISE = 300;

static const unsigned long NOACCOUNTINFO_INACTIVE_INTERVAL = (7 * 24 * 60 * 60);  // s

enum {
    ETaskConnect,
    ELongLinkConnect,
    ENetworkChangeConnect,
};

enum {
    EForgroundOneMinute,
    EForgroundTenMinute,
    EForgroundActive,
    EBackgroundActive,
    EInactive,
};

typedef struct
{
	int type;
	int interval;

}TestInputItem;

extern int __CurActiveState(const ActiveLogic& _activeLogic) ;
extern unsigned long __Interval(int _type, const ActiveLogic& _activeLogic);





//**************************hook*******************************************

//-----------------------------begin hook----------------------------------------------
static int sg_NetInfo=kNoNet;
static struct AccountInfo sg_accountInfo;
//---step1  Declare hook
DECLARE_HOOK_BEGIN
	DECLARE_HOOK_GLOBAL_FUNCTION0(getNetInfo, int);
//	DECLARE_HOOK_GLOBAL_FUNCTION0(getAccountInfo, struct AccountInfo);
DECLARE_HOOK_END

///---step2	Describe which function(the functions DECLARED above) should be Hook
ATTACH_HOOK_BEGIN
	ATTACH_HOOK_FUNCTION0(, getNetInfo, int);
//	ATTACH_HOOK_FUNCTION0(, getAccountInfo, struct AccountInfo);
ATTACH_HOOK_END

//-----step3 Describe which function(the functions DECLARED above) should be UnHook
DETACH_HOOK_BEGIN
	DETACH_HOOK_FUNCTION0(, getNetInfo, int);
//	DETACH_HOOK_FUNCTION0(, getAccountInfo, struct AccountInfo);
DETACH_HOOK_END
///------step4 Implement the hook function for the real function
IMPLEMENT_MINE_HOOK_GLOBAL_FUNCITON0(getNetInfo, int)
{
	return sg_NetInfo;
}
/*
IMPLEMENT_MINE_HOOK_GLOBAL_FUNCITON0(getAccountInfo, struct AccountInfo)
{
	return sg_accountInfo;
}

*/
struct AccountInfo getAccountInfo()
{
	return sg_accountInfo;
}

//----------------------------end hook-------------------------------------


void changeActiveLogic(int activeState, ActiveLogic& activeLogic)
{
	switch(activeState)
	{
		case EForgroundActive:
			activeLogic.m_isactive = true;
			activeLogic.m_isforeground = true;
			activeLogic.m_lastforegroundchangetime = ::gettickcount() - 10*60*1000;
			break;
		case EInactive:
			activeLogic.m_isactive = false;
			activeLogic.m_isforeground = false;
			break;
		case EForgroundOneMinute:
			activeLogic.m_isactive = true;
			activeLogic.m_isforeground = true;
			activeLogic.m_lastforegroundchangetime = ::gettickcount();
			break;
		case EForgroundTenMinute:
			activeLogic.m_isactive = true;
			activeLogic.m_isforeground = true;
			activeLogic.m_lastforegroundchangetime = ::gettickcount()-5*60*1000;
			break;
		case EBackgroundActive:
			activeLogic.m_isactive = true;
			activeLogic.m_isforeground = false;
			break;
		default:
			printf("**error activeState=%d****************************************\n", activeState);
	}
}
void testFunc(TestCaseInfo* m_testCaseInfo, NetType netType, bool isUserNameEmpty, int activeState, ActiveLogic& activeLogic, BaseScene& baseScene, unsigned long expectInterval)
{
	Hook();
	sg_NetInfo = netType;
	PUBC_EXPECT_EQ((int)netType, (int)getNetInfo(), &baseScene);

	if(!isUserNameEmpty)
		sg_accountInfo.username = "test";
	//PUBC_EXPECT_EQ((int)isUserNameEmpty, (int)getAccountInfo().username.empty(), &baseScene);

	changeActiveLogic(activeState, activeLogic);
	bool isActiveStateMatchExpect = (activeState ==__CurActiveState(activeLogic));
	PUBC_EXPECT_TRUE(isActiveStateMatchExpect , &baseScene);

	unsigned long interval = __Interval(ELongLinkConnect, activeLogic);
	PUBC_EXPECT_EQ(interval, expectInterval, &baseScene);
	UnHook();
}


///*******************nonet   usename is empty

PUBC_TEST(MMLongLinkConnectMonitor, test0)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, No AccountInfo, EForgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 240 * NONET_SALT_RATE + NONET_SALT_RISE;
	testFunc(m_testCaseInfo, kNoNet, true, EForgroundActive, activeLogic, baseScene, expectInterval);
}


PUBC_TEST(MMLongLinkConnectMonitor, test1)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, No AccountInfo, EInactive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = NOACCOUNTINFO_INACTIVE_INTERVAL;
	testFunc(m_testCaseInfo, kNoNet, true, EInactive, activeLogic, baseScene, expectInterval);
}

PUBC_TEST(MMLongLinkConnectMonitor, test2)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, No AccountInfo, EForgroundOneMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 15;
	testFunc(m_testCaseInfo, kNoNet, true, EForgroundOneMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test3)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, No AccountInfo, EForgroundTenMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 30;
	testFunc(m_testCaseInfo, kNoNet, true, EForgroundTenMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test4)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, No AccountInfo, EBackgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 300;
	testFunc(m_testCaseInfo, kNoNet, true, EBackgroundActive, activeLogic, baseScene, expectInterval);

}

//*********EWifi username is empty
PUBC_TEST(MMLongLinkConnectMonitor, test5)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, No AccountInfo, EForgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 240 * NOACCOUNTINFO_SALT_RATE + NOACCOUNTINFO_SALT_RISE;
	testFunc(m_testCaseInfo, kWifi, true, EForgroundActive, activeLogic, baseScene, expectInterval);
}


PUBC_TEST(MMLongLinkConnectMonitor, test6)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, No AccountInfo, EInactive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = NOACCOUNTINFO_INACTIVE_INTERVAL;
	testFunc(m_testCaseInfo, kWifi, true, EInactive, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test7)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, No AccountInfo, EForgroundOneMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 15;
	testFunc(m_testCaseInfo, kWifi, true, EForgroundOneMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test8)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, No AccountInfo, EForgroundTenMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 30;
	testFunc(m_testCaseInfo, kWifi, true, EForgroundTenMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test9)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, No AccountInfo, EBackgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 300;
	testFunc(m_testCaseInfo, kWifi, true, EBackgroundActive, activeLogic, baseScene, expectInterval);

}

//**********EWIFI   username is not empty

PUBC_TEST(MMLongLinkConnectMonitor, test10)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, Have AccountInfo, EForgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 240;
	testFunc(m_testCaseInfo, kWifi, false, EForgroundActive, activeLogic, baseScene, expectInterval);
}


PUBC_TEST(MMLongLinkConnectMonitor, test11)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, Have AccountInfo, EInactive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 600;
	testFunc(m_testCaseInfo, kWifi, false, EInactive, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test12)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, Have AccountInfo, EForgroundOneMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 15;
	testFunc(m_testCaseInfo, kWifi, false, EForgroundOneMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test13)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, Have AccountInfo, EForgroundTenMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 30;
	testFunc(m_testCaseInfo, kWifi, false, EForgroundTenMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test14)
{
	m_testCaseInfo->m_TestCaseName = "(EWifi, Have AccountInfo, EBackgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 300;
	testFunc(m_testCaseInfo, kWifi, false, EBackgroundActive, activeLogic, baseScene, expectInterval);

}

//ENONET  username is not empty

PUBC_TEST(MMLongLinkConnectMonitor, test15)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, Have AccountInfo, EForgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 240 * NONET_SALT_RATE + NONET_SALT_RISE;
	testFunc(m_testCaseInfo, kNoNet, false, EForgroundActive, activeLogic, baseScene, expectInterval);
}


PUBC_TEST(MMLongLinkConnectMonitor, test16)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, Have AccountInfo, EInactive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 600 * NONET_SALT_RATE + NONET_SALT_RISE;
	testFunc(m_testCaseInfo, kNoNet, false, EInactive, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test17)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, Have AccountInfo, EForgroundOneMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 15;
	testFunc(m_testCaseInfo, kNoNet, false, EForgroundOneMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test18)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, Have AccountInfo, EForgroundTenMinute)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 30;
	testFunc(m_testCaseInfo, kNoNet, false, EForgroundTenMinute, activeLogic, baseScene, expectInterval);

}

PUBC_TEST(MMLongLinkConnectMonitor, test19)
{
	m_testCaseInfo->m_TestCaseName = "(ENoNet, Have AccountInfo, EBackgroundActive)";
	//net, accountInfo , activeState
	BaseScene baseScene;
	ActiveLogic activeLogic;


	unsigned long  expectInterval = 300;
	testFunc(m_testCaseInfo, kNoNet, false, EBackgroundActive, activeLogic, baseScene, expectInterval);

}
